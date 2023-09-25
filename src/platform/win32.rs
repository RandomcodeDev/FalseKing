use windows::Win32::UI::WindowsAndMessaging;

use super::PlatformBackend;

pub struct Win32Backend {
    window: isize,
    width: u32,
    height: u32
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
}
