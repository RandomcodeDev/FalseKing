#[cfg(windows)]
mod dx12;
#[cfg(apple)]
mod metal;
#[cfg(not(apple))]
mod vulkan;

#[derive(Clone, clap::ValueEnum)]
pub enum BackendType {
    /// DirectX 12
    #[cfg(windows)]
    DirectX12,

    /// Vulkan (not implemented yet)
    #[cfg(not(apple))]
    Vulkan,

    /// Metal (gonna be a long time)
    #[cfg(apple)]
    Metal,
}

pub trait Renderer {
    /// Prepare to take rendering commands
    fn begin_frame(&mut self);

    /// Send rendering commands to the GPU and swap buffers or whatever
    fn end_frame(&mut self);

    /// Shut down rendering
    fn shutdown(&mut self);
}

/// Creates an instance of the requested backend, or the first one that initializes successfully
pub fn get_renderer(api: Option<BackendType>) -> Option<Box<dyn Renderer>> {
    // This function has a bunch of return statements that aren't necessarily executed
    #[allow(unreachable_code)]
    if let Some(api) = api {
        match api {
            #[cfg(windows)]
            BackendType::DirectX12 => match dx12::Dx12Renderer::new() {
                Ok(dx12) => Some(Box::new(dx12)),
                Err(err) => None,
            },
            #[cfg(not(apple))]
            BackendType::Vulkan => match vulkan::VkRenderer::new() {
                Ok(vk) => Some(Box::new(vk)),
                Err(err) => None,
            },
            #[cfg(apple)]
            BackendType::Metal => match metal::MtlRenderer::new() {
                Ok(mtl) => Some(Box::new(mtl)),
                Err(err) => None,
            },
        }
    } else {
        #[cfg(windows)]
        match dx12::Dx12Renderer::new() {
            Ok(dx12) => {
                return Some(Box::new(dx12));
            }
            Err(err) => {}
        }
        #[cfg(not(apple))]
        match vulkan::VkRenderer::new() {
            Ok(vk) => {
                return Some(Box::new(vk));
            }
            Err(err) => {}
        }
        #[cfg(apple)]
        match metal::MtlRenderer::new() {
            Ok(mtl) => {
                return Some(Box::new(mtl));
            }
            Err(err) => {}
        }

        panic!("Failed to initialize any renderer backend");
    }
}
