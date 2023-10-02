use super::Renderer;
use crate::platform::PlatformBackend;

pub struct VulkanRenderer {}

impl VulkanRenderer {
    pub fn new(_backend: &dyn PlatformBackend) -> Option<Box<Self>> {
        Some(Box::new(Self {}))
    }
}

impl Renderer for VulkanRenderer {
    fn begin_frame(&mut self) {}

    fn end_frame(&mut self) {}

    fn shutdown(&mut self) {}
}
