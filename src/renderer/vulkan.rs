use super::Renderer;
use crate::platform::PlatformBackend;
use log::{error, info};
use pci_ids::FromId;
use std::sync::Arc;
use vulkano::{
    device::physical::PhysicalDevice, device::physical::PhysicalDeviceError,
    device::physical::PhysicalDeviceType, device::Device, device::DeviceCreateInfo,
    device::DeviceCreationError, device::DeviceExtensions, device::Queue, device::QueueFlags,
    instance::Instance, instance::InstanceCreateInfo, instance::InstanceCreationError,
    instance::InstanceExtensions, swapchain::Surface, Version, VulkanLibrary,
};

struct Gpu {
    device: Arc<PhysicalDevice>,
    queue_family_index: u32,
}

pub struct VkRenderer {}

impl VkRenderer {
    pub fn new(backend: &dyn PlatformBackend) -> Option<Box<Self>> {
        info!("Initializing Vulkan renderer");

        let mut instance_extensions = InstanceExtensions {
            khr_surface: true,
            ..Default::default()
        };
        backend.enable_vulkan_extensions(&mut instance_extensions);

        let instance = match Self::create_instance(instance_extensions) {
            Ok(instance) => instance,
            Err(err) => {
                error!("Failed to create instance: {err}");
                return None;
            }
        };

        let surface = match backend.create_vulkan_surface(instance.clone()) {
            Ok(surface) => surface,
            Err(err) => {
                error!("Failed to create surface: {err}");
                return None;
            }
        };

        let device_extensions = DeviceExtensions {
            khr_swapchain: true,
            ..Default::default()
        };

        let (gpus, gpu_index) =
            match Self::choose_gpu(instance.clone(), &device_extensions, surface.clone()) {
                Ok(pair) => pair,
                Err(err) => {
                    error!("Failed to find usable GPU: {err}");
                    return None;
                }
            };

        let (device, mut queues) = match Self::create_device(&gpus[0], device_extensions) {
            Ok(pair) => pair,
            Err(err) => {
                error!("Failed to create device: {err}");
                return None;
            }
        };

        Some(Box::new(Self {}))
    }

    fn create_instance(
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

    fn choose_gpu(
        instance: Arc<Instance>,
        device_extensions: &DeviceExtensions,
        surface: Arc<Surface>,
    ) -> Result<(Vec<Gpu>, u64), PhysicalDeviceError> {
        info!("Finding usable GPUs");

        let mut gpus = Vec::new();

        let mut devices: Vec<Gpu> = instance
            .enumerate_physical_devices()?
            .filter(|device| device.supported_extensions().contains(&device_extensions))
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
                    None => "unknown"
                },
                match pci_ids::Device::from_vid_pid(
                    properties.vendor_id as u16,
                    properties.device_id as u16
                ) {
                    Some(device) => device.name(),
                    None => "unknown"
                }
            );
        });

        Ok((gpus, 0))
    }

    fn create_device(
        gpu: &Gpu,
        device_extensions: DeviceExtensions,
    ) -> Result<
        (
            Arc<Device>,
            impl ExactSizeIterator + Iterator<Item = Arc<Queue>>,
        ),
        DeviceCreationError,
    > {
        Device::new(gpu.device.clone(), DeviceCreateInfo::default())
    }
}

impl Renderer for VkRenderer {
    fn begin_frame(&mut self) {}

    fn end_frame(&mut self) {}

    fn shutdown(&mut self) {}
}
