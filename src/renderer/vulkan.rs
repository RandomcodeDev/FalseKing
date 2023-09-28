use log::{debug, error, info};
use super::Renderer;
use std::sync::Arc;
use vulkano as vk;

type VkResult<T> = Result<T, vk::VulkanError>;

pub struct VkRenderer {
    library: Arc<vk::VulkanLibrary>,
    instance: Arc<vk::instance::Instance>,
}

impl VkRenderer {
    pub fn new() -> VkResult<Self> {
        let library = match vk::VulkanLibrary::new() {
            Ok(library) => library,
            Err(err) => {
                error!("Failed to load library: {}", err);
                return Err(vk::VulkanError::InitializationFailed);
            }
        };

        let instance = match Self::create_instance(library) {
            Ok(instance) => instance,
            Err(err) => {
                error!("Failed to create instance: {}", err);
                return Err(vk::VulkanError::InitializationFailed);
            }
        };

        Ok(Self { library, instance })
    }

    fn create_instance(
        library: Arc<vk::VulkanLibrary>,
    ) -> Result<Arc<vk::instance::Instance>, vk::instance::InstanceCreationError> {
        let mut create_info = vk::instance::InstanceCreateInfo::application_from_cargo_toml();
        create_info.max_api_version = Some(vk::Version::V1_3);

        vk::instance::Instance::new(library, create_info)
    }
}

impl Renderer for VkRenderer {
    fn begin_frame(&mut self) {}

    fn end_frame(&mut self) {}

    fn shutdown(&mut self) {}
}
