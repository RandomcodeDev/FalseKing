use super::Renderer;
use windows::core::Result;

pub struct Dx12Renderer {}

impl Dx12Renderer {
    pub fn new() -> Result<Self> {
        Ok(Self {})
    }
}

impl Renderer for Dx12Renderer {
    fn begin_frame(&mut self) {}

    fn end_frame(&mut self) {}

    fn shutdown(&mut self) {}
}
