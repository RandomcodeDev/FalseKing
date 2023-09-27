use metal::*;
use super::Renderer;

pub struct MtlRenderer {

}

impl MtlRenderer {
    pub fn new() -> Result<Self, str> {
        Ok(Self {
        
        })
    }
}

impl Renderer for MtlRenderer {
    fn begin_frame(&mut self) {}

    fn end_frame(&mut self) {}

    fn shutdown(self) {}
}