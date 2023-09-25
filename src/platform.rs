#[cfg(windows)]
pub mod win32;

pub trait PlatformBackend {
    // Clean up the backend
    fn shutdown(self);

    // Handle events
    fn update(&mut self) -> bool;

    // Get the width of the screen
    fn get_width(&self) -> u32;

    // Get the height of the screen
    fn get_height(&self) -> u32;
}

pub fn get_backend_for_platform() -> impl PlatformBackend {
    #[cfg(windows)]
    return win32::Win32Backend::new();
    #[cfg(unix)]
    return unix::UnixBackend::new();
}
