use super::Renderer;

type VkResult<T> = Result<T, vulkano::VulkanError>;

pub struct VkRenderer {

}

impl VkRenderer {
    pub fn new() -> VkResult<Self> {
        Ok(Self {
        
        })
    }
}

impl Renderer for VkRenderer {
    fn begin_frame(&mut self) {}

    fn end_frame(&mut self) {}

    fn shutdown(&mut self) {}
}