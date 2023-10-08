use super::Renderer;
use metal::*;
use std::sync::{Arc, Mutex};

pub struct MtlRenderer {}

impl MtlRenderer {
    pub fn new(backend: Arc<Mutex<dyn PlatformBackend>>) -> Option<Arc<Mutex<Self>>> {
        Some(Arc::new(Mutex::new(Self {})))
    }
}

impl Renderer for MtlRenderer {
    fn begin_frame(&mut self) {}

    fn end_frame(&mut self) {}

    fn shutdown(&mut self) {}
}
