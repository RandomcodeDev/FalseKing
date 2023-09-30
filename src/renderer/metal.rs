use super::Renderer;
use metal::*;

pub struct MtlRenderer {}

impl MtlRenderer {
    pub fn new(backend: &dyn PlatformBackend) -> Option<Box<Self>> {
        Some(Box::new(Self {}))
    }
}

impl Renderer for MtlRenderer {
    fn begin_frame(&mut self) {}

    fn end_frame(&mut self) {}

    fn shutdown(&mut self) {}
}
