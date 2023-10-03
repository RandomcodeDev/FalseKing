use super::Renderer;
use crate::platform::PlatformBackend;
use log::{error, info};
use std::sync::Arc;
use vulkano::{
    instance::Instance, instance::InstanceCreateInfo, instance::InstanceCreationError,
    instance::InstanceExtensions, Version, VulkanLibrary,
};

pub struct VulkanRenderer {}

impl VulkanRenderer {
    pub fn new(_backend: &dyn PlatformBackend) -> Option<Box<Self>> {
        info!("Initializing Vulkan renderer");

        let mut instance_extensions = InstanceExtensions {
            khr_surface: true,
            ..Default::default()
        };
        _backend.enable_vulkan_extensions(&mut instance_extensions);

        let instance = match Self::create_instance(instance_extensions) {
            Ok(instance) => instance,
            Err(err) => {
                error!("Failed to create instance: {err}");
                return None;
            }
        };

        Some(Box::new(Self {}))
    }

    fn create_instance(
        extensions: InstanceExtensions,
    ) -> Result<Arc<Instance>, InstanceCreationError> {
        info!("Creating instance with extensions {extensions:?}");;

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
}

impl Renderer for VulkanRenderer {
    fn begin_frame(&mut self) {}

    fn end_frame(&mut self) {}

    fn shutdown(&mut self) {}
}
