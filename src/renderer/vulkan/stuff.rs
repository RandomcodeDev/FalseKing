use super::{Gpu, VkRenderer};
use crate::platform::PlatformBackend;
use log::{error, info};
use pci_ids::FromId;
use std::sync::{Arc, Mutex};
use vulkano::{
    device::physical::PhysicalDeviceError,
    device::physical::PhysicalDeviceType,
    device::Device,
    device::DeviceCreateInfo,
    device::DeviceCreationError,
    device::DeviceExtensions,
    device::Queue,
    device::QueueCreateInfo,
    device::QueueFlags,
    image::{ImageAccess, SwapchainImage, view::ImageView},
    instance::Instance,
    instance::InstanceCreateInfo,
    instance::InstanceCreationError,
    instance::InstanceExtensions,
    pipeline::graphics::viewport::Viewport,
    render_pass::{Framebuffer, FramebufferCreationError, RenderPass, FramebufferCreateInfo},
    swapchain::{Surface, Swapchain, SwapchainCreateInfo, SwapchainCreationError},
    Version, VulkanLibrary, OomError,
};

impl VkRenderer {
    pub(super) fn create_instance(
        extensions: InstanceExtensions,
    ) -> Result<Arc<Instance>, InstanceCreationError> {
        info!("Creating instance with extensions {extensions:?}");

        let library = match VulkanLibrary::new() {
            Ok(library) => library,
            Err(err) => {
                error!("Failed to load Vulkan library: {err}");
                return Err(InstanceCreationError::InitializationFailed);
            }
        };

        let mut create_info = InstanceCreateInfo::application_from_cargo_toml();
        create_info.enabled_extensions = extensions;
        create_info.max_api_version = Some(Version::V1_3);
        Instance::new(library, create_info)
    }

    pub(super) fn choose_gpu(
        instance: Arc<Instance>,
        device_extensions: &DeviceExtensions,
        surface: Arc<Surface>,
    ) -> Result<(Vec<Gpu>, usize), PhysicalDeviceError> {
        info!("Finding usable GPUs");

        let mut devices: Vec<Gpu> = instance
            .enumerate_physical_devices()?
            .filter(|device| device.supported_extensions().contains(device_extensions))
            .filter_map(|device| {
                device
                    .queue_family_properties()
                    .iter()
                    .enumerate()
                    .position(|(index, queue_family)| {
                        queue_family.queue_flags.contains(QueueFlags::GRAPHICS)
                            && device
                                .surface_support(index as u32, surface.as_ref())
                                .unwrap_or(false)
                    })
                    .map(|index| Gpu {
                        device,
                        queue_family_index: index as u32,
                    })
            })
            .collect();

        // Sort by type
        devices.sort_by_key(|gpu| match gpu.device.properties().device_type {
            PhysicalDeviceType::DiscreteGpu => 0,
            PhysicalDeviceType::IntegratedGpu => 1,
            PhysicalDeviceType::VirtualGpu => 2,
            PhysicalDeviceType::Cpu => 3,
            PhysicalDeviceType::Other => 4,
            _ => 5,
        });

        devices.iter().enumerate().for_each(|(index, gpu)| {
            let properties = gpu.device.properties();
            info!("GPU {index} {}:", properties.device_name);
            info!(
                "\tDriver information: {} {}",
                properties.driver_name.clone().unwrap_or(String::new()),
                properties.driver_info.clone().unwrap_or(String::new())
            );
            info!(
                "\tPCI ID: [{:04x}:{:04x}] ({:?} {:?})",
                properties.vendor_id,
                properties.device_id,
                match pci_ids::Vendor::from_id(properties.vendor_id as u16) {
                    Some(vendor) => vendor.name(),
                    None => "unknown",
                },
                match pci_ids::Device::from_vid_pid(
                    properties.vendor_id as u16,
                    properties.device_id as u16
                ) {
                    Some(device) => device.name(),
                    None => "unknown",
                }
            );
        });

        Ok((devices, 0))
    }

    pub(super) fn create_device(
        gpu: &Gpu,
        device_extensions: DeviceExtensions,
    ) -> Result<
        (
            Arc<Device>,
            impl ExactSizeIterator + Iterator<Item = Arc<Queue>>,
        ),
        DeviceCreationError,
    > {
        info!("Creating device from GPU {}", gpu);

        Device::new(
            gpu.device.clone(),
            DeviceCreateInfo {
                enabled_extensions: device_extensions,
                queue_create_infos: vec![QueueCreateInfo {
                    queue_family_index: gpu.queue_family_index,
                    ..Default::default()
                }],
                ..Default::default()
            },
        )
    }

    pub(super) fn create_swapchain(
        backend: Arc<Mutex<dyn PlatformBackend>>,
        device: Arc<Device>,
        surface: Arc<Surface>,
    ) -> Result<(Arc<Swapchain>, Vec<Arc<SwapchainImage>>), SwapchainCreationError> {
        info!("Creating swapchain");

        let capabilities = match device
            .physical_device()
            .surface_capabilities(surface.as_ref(), Default::default())
        {
            Ok(capabilities) => capabilities,
            Err(err) => {
                error!("Failed to get capabilities for device: {err}");
                return Err(SwapchainCreationError::DeviceLost);
            }
        };

        let image_usage = capabilities.supported_usage_flags;
        let composite_alpha = match capabilities
            .supported_composite_alpha
            .into_iter()
            .next()
        {
            Some(alpha) => alpha,
            None => {
                error!("Failed to get composite alpha for swapchain");
                return Err(SwapchainCreationError::DeviceLost); // not close enough
            }
        };

        let image_format = Some(
            device
                .physical_device()
                .surface_formats(&surface, Default::default())
                .unwrap()[0]
                .0,
        );

        let backend = backend.try_lock().unwrap();
        let image_extent: [u32; 2] = [backend.get_width(), backend.get_height()];

        Swapchain::new(
            device.clone(),
            surface.clone(),
            SwapchainCreateInfo {
                min_image_count: capabilities.min_image_count,
                image_format,
                image_extent,
                image_usage,
                composite_alpha,
                ..Default::default()
            },
        )
    }

    // this one is a little ugly
    pub(super) fn create_framebuffers(
        images: &[Arc<SwapchainImage>],
        render_pass: Arc<RenderPass>,
        viewport: &mut Viewport,
    ) -> Result<Vec<Arc<Framebuffer>>, FramebufferCreationError> {
        let dimensions = images[0].dimensions().width_height();
        viewport.dimensions = [dimensions[0] as f32, dimensions[1] as f32];

        info!("Creating {} {}x{} framebuffers", images.len(), viewport.dimensions[0], viewport.dimensions[1]);

        let framebuffers: Vec<Result<Arc<Framebuffer>, FramebufferCreationError>> = images.iter().enumerate().map(|(index, image)| {
            let view = match ImageView::new_default(image.clone()) {
                Ok(view) => view,
                Err(err) => {
                    error!("Failed to create image view {index}: {err}");
                    return Err(FramebufferCreationError::OomError(OomError::OutOfHostMemory));
                }
            };

            Framebuffer::new(render_pass.clone(), FramebufferCreateInfo {attachments: vec![view], ..Default::default()})
        }).collect::<Vec<_>>();

        // in order to return a Result, so other renderers get a chance if Vulkan can't be initialized
        let mut unwrapped_framebuffers = Vec::new();
        for (index, result) in framebuffers.iter().enumerate() {
            let framebuffer = match result {
                Ok(framebuffer) => framebuffer,
                Err(err) => {
                    error!("Failed to create framebuffer {index}: {err}");
                    return Err(*err);
                }
            };
            unwrapped_framebuffers.push(framebuffer.clone());
        };

        Ok(unwrapped_framebuffers)
    }
}
