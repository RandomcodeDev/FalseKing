use super::Renderer;
use crate::platform::PlatformBackend;
use log::{error, info};
use vulkano::{device::{physical::PhysicalDevice, DeviceExtensions}, instance::InstanceExtensions};
use std::{fmt::Display, sync::Arc};

// boilerplate stuff
mod stuff;

struct Gpu {
    device: Arc<PhysicalDevice>,
    queue_family_index: u32,
}

impl Display for Gpu {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(&self.device.properties().device_name)
    }
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

        let queue = queues.next();

        let (mut swapchain, images) = match Self::create_swapchain(backend, device.clone()) {
            Ok(pair) => pair,
            Err(err) => {
                error!("Failed to create swapchain: {err}");
                return None;
            }
        };

        Some(Box::new(Self {}))
    }
}

impl Renderer for VkRenderer {
    fn begin_frame(&mut self) {}

    fn end_frame(&mut self) {}

    fn shutdown(&mut self) {}
}
