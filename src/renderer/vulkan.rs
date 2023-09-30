use crate::platform::PlatformBackend;
use log::{debug, error, info};

use super::Renderer;
use std::{collections::HashSet, sync::Arc};
use vulkano as vk;

type VkResult<T> = Result<T, vk::VulkanError>;

struct GpuInfo {
    device: Arc<vk::device::physical::PhysicalDevice>,
    graphics_queue_family: u32,
    compute_queue_family: u32,
}

pub struct VkRenderer {
    library: Arc<vk::VulkanLibrary>,
    instance: Arc<vk::instance::Instance>,
    surface: Arc<vk::swapchain::Surface>,
    devices: Vec<GpuInfo>,
    device_index: usize,
    device: Arc<vk::device::Device>,
    graphics_queue: Arc<vk::device::Queue>,
    compute_queue: Arc<vk::device::Queue>,
}

impl VkRenderer {
    pub fn new(backend: &dyn PlatformBackend) -> Option<Box<Self>> {
        info!("Initializing Vulkan backend");

        let library = match vk::VulkanLibrary::new() {
            Ok(library) => library,
            Err(err) => {
                error!("Failed to load library: {}", err);
                return None;
            }
        };

        let mut instance_extensions = vk::instance::InstanceExtensions {
            khr_surface: true,
            ..Default::default()
        };

        backend.enable_vulkan_extensions(&mut instance_extensions);

        debug!(
            "Enabling these instance extensions: {:#?}",
            instance_extensions
        );

        let instance = match Self::create_instance(library.clone(), instance_extensions) {
            Ok(instance) => instance,
            Err(err) => {
                error!("Failed to create instance: {}", err);
                return None;
            }
        };

        let surface = match backend.create_vulkan_surface(instance.clone()) {
            Ok(surface) => surface,
            Err(err) => {
                error!("Failed to create surface: {}", err);
                return None;
            }
        };

        let device_extensions = vk::device::DeviceExtensions {
            khr_swapchain: true,
            ..Default::default()
        };
        let (devices, device_index) =
            match Self::choose_device(instance.clone(), &device_extensions) {
                Ok(pair) => pair,
                Err(err) => {
                    error!("Failed to locate suitable device: {}", err);
                    return None;
                }
            };

        let (device, graphics_queue, compute_queue) =
            match Self::create_device(&devices[device_index], &device_extensions)
            {
                Ok(pair) => pair,
                Err(err) => {
                    error!("Failed to create device: {}", err);
                    return None;
                }
            };

        Some(Box::new(Self {
            library,
            instance,
            surface,
            devices,
            device_index,
            device,
            graphics_queue,
            compute_queue,
        }))
    }

    fn create_instance(
        library: Arc<vk::VulkanLibrary>,
        extensions: vk::instance::InstanceExtensions,
    ) -> Result<Arc<vk::instance::Instance>, vk::instance::InstanceCreationError> {
        let mut create_info = vk::instance::InstanceCreateInfo::application_from_cargo_toml();
        create_info.max_api_version = Some(vk::Version::V1_3);
        create_info.enabled_extensions = extensions;

        vk::instance::Instance::new(library, create_info)
    }

    fn choose_device(
        instance: Arc<vk::instance::Instance>,
        extensions: &vk::device::DeviceExtensions,
    ) -> VkResult<(Vec<GpuInfo>, usize)> {
        info!("Enumerating GPUs");

        let mut gpus = Vec::new();
        instance.enumerate_physical_devices()?.for_each(|device| {
            fn find_family(
                queue_families: std::slice::Iter<'_, vk::device::QueueFamilyProperties>,
                queue_flags: vk::device::QueueFlags,
            ) -> Option<u32> {
                queue_families
                    .enumerate()
                    .find_map(|(index, family)| -> Option<u32> {
                        if family.queue_flags.contains(queue_flags) {
                            Some(index as u32)
                        } else {
                            None
                        }
                    })
            }

            let queue_families = device.queue_family_properties().into_iter();

            let graphics = find_family(queue_families.clone(), vk::device::QueueFlags::GRAPHICS);
            let compute = find_family(queue_families.clone(), vk::device::QueueFlags::COMPUTE);

            let have_extensions = device.supported_extensions().contains(&extensions);

            if graphics.is_some() && compute.is_some() && have_extensions {
                let gpu = GpuInfo {
                    device,
                    graphics_queue_family: graphics.unwrap(),
                    compute_queue_family: compute.unwrap(),
                };

                let properties = gpu.device.properties();

                info!("GPU {}:", gpus.len() + 1);
                info!("\tName: {}", properties.device_name);
                info!(
                    "\tDriver: {} {}",
                    properties
                        .driver_name
                        .clone()
                        .unwrap_or(String::from("unknown")),
                    properties
                        .driver_info
                        .clone()
                        .unwrap_or(String::from("unknown"))
                );
                info!(
                    "\tPCI ID: [{:04x}:{:04x}]",
                    properties.vendor_id, properties.device_id
                );
                info!("\tType: {:?}", properties.device_type);

                gpus.push(gpu);
            }
        });

        info!("Got {} usable GPU(s)", gpus.len());

        Ok((gpus, 0))
    }

    fn create_device(
        gpu: &GpuInfo,
        enabled_extensions: &vk::device::DeviceExtensions,
    ) -> Result<
        (
            Arc<vk::device::Device>,
            Arc<vk::device::Queue>,
            Arc<vk::device::Queue>,
        ),
        vk::device::DeviceCreationError,
    > {
        let mut queue_create_infos = Vec::new();
        queue_create_infos.push(vk::device::QueueCreateInfo {
            queue_family_index: gpu.graphics_queue_family,
            ..Default::default()
        });

        if gpu.graphics_queue_family != gpu.compute_queue_family {
            queue_create_infos.push(vk::device::QueueCreateInfo {
                queue_family_index: gpu.compute_queue_family,
                ..Default::default()
            });
        }

        let create_info = vk::device::DeviceCreateInfo {
            enabled_extensions: enabled_extensions.clone(),
            queue_create_infos,
            ..Default::default()
        };

        let (device, mut queues) = vk::device::Device::new(gpu.device.clone(), create_info)?;
        let graphics_queue = queues.next().unwrap();
        let compute_queue = if gpu.graphics_queue_family != gpu.compute_queue_family {
            queues.next().unwrap()
        } else {
            graphics_queue.clone()
        };

        Ok((device, graphics_queue, compute_queue))
    }
}

impl Renderer for VkRenderer {
    fn begin_frame(&mut self) {}

    fn end_frame(&mut self) {}

    fn shutdown(&mut self) {
        info!("Shutting down Vulkan backend");
    }
}
