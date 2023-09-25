use windows::Win32::UI::WindowsAndMessaging;

use super::PlatformBackend;

pub struct Win32Backend {
    window: isize,
    width: i32,
    height: i32
}

impl Win32Backend {
    pub fn new() -> Self {
        Self {}
    }
}

impl PlatformBackend for Win32Backend {
    fn shutdown(mut self) {

    }

    fn update(&mut self) -> bool {
        return true;
    }

    fn get_width(&self) -> u32 {
        (u32)(self.width)
    }

    fn get_height(&self) -> u32 {
        (u32)(self.height)
    }
}
