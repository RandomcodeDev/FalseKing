use super::Renderer;

pub struct GLRenderer {

}

impl GLRenderer {
    pub fn new() -> Result<Self, ()> {
        Ok(Self {
        
        })
    }
}

impl Renderer for GLRenderer {
    fn begin_frame(&mut self) {}

    fn end_frame(&mut self) {}

    fn shutdown(&mut self) {}
}