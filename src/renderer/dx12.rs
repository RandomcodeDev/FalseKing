use std::sync::{Arc, Mutex};

use super::Renderer;
use crate::platform::PlatformBackend;
//use windows::core::Result;

pub struct Dx12Renderer {}

impl Dx12Renderer {
    pub fn new(_backend: Arc<Mutex<dyn PlatformBackend>>) -> Option<Arc<Mutex<Self>>> {
        Some(Arc::new(Mutex::new(Self {})))
    }
}

impl Renderer for Dx12Renderer {
    fn begin_frame(&mut self) {}

    fn end_frame(&mut self) {}

    fn shutdown(&mut self) {}
}
