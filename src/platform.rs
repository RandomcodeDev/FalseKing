#[cfg(windows)]
pub mod win32;
#[cfg(unix)]
pub mod unix;

pub trait PlatformBackend {
    // Clean up the backend
    fn shutdown(self);

    // Handle events
    fn update(&mut self) -> bool;

    // Has the window been resized
    fn has_resized(&self) -> bool;

    // Get the width of the screen
    fn get_width(&self) -> u32;

    // Get the height of the screen
    fn get_height(&self) -> u32;

    // Is the game focused
    fn is_focused(&self) -> bool;
}

pub fn get_backend_for_platform() -> Option<impl PlatformBackend> {
    #[cfg(windows)]
    return win32::Win32Backend::new();
    #[cfg(unix)]
    return unix::UnixBackend::new().ok();
}
