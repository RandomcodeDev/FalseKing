#[cfg(unix)]
mod unix;
#[cfg(windows)]
mod win32;

use std::sync::Arc;

#[cfg(not(apple))]
use vulkano as vk;

pub trait PlatformBackend {
    /// Clean up the backend
    fn shutdown(self);

    /// Handle events
    fn update(&mut self) -> bool;

    /// Get the native window handle
    fn get_handle(&self) -> usize;

    /// Has the window been resized
    fn has_resized(&self) -> bool;

    /// Get the width of the screen
    fn get_width(&self) -> u32;

    /// Get the height of the screen
    fn get_height(&self) -> u32;

    /// Is the game focused
    fn is_focused(&self) -> bool;

    #[cfg(not(apple))]
    /// Enables the Vulkan instance extensions that the platform needs
    fn enable_vulkan_extensions(&self, extensions: &mut vk::instance::InstanceExtensions);

    #[cfg(not(apple))]
    fn check_vulkan_present_support(
        &self,
        device: Arc<vk::device::physical::PhysicalDevice>,
        device_name: &String,
        queue_family_index: u32,
    ) -> Option<bool>;

    #[cfg(not(apple))]
    /// Create a Vulkan surface (renderer doesn't need the details of this)
    fn create_vulkan_surface(
        &self,
        instance: Arc<vk::instance::Instance>,
    ) -> Result<Arc<vulkano::swapchain::Surface>, vk::swapchain::SurfaceCreationError>;
}

/// Creates an instance of the appropriate backend for the platform
pub fn get_backend_for_platform() -> Option<impl PlatformBackend> {
    #[cfg(windows)]
    return win32::Win32Backend::new();
    #[cfg(unix)]
    return unix::UnixBackend::new().ok();
}
