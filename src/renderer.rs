#[cfg(windows)]
mod dx12;
#[cfg(apple)]
mod metal;
#[cfg(not(apple))]
mod opengl;
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

    /// OpenGL (might be soon)
    #[cfg(not(apple))]
    OpenGL,
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
pub fn get_renderer(api: Option<BackendType>) -> Box<dyn Renderer> {
    // This function has a bunch of return statements that aren't necessarily executed
    #[allow(unreachable_code)]
    if let Some(api) = api {
        match api {
            #[cfg(windows)]
            BackendType::DirectX12 => {
                return Box::new(dx12::Dx12Renderer::new().ok().unwrap());
            }
            #[cfg(not(apple))]
            BackendType::Vulkan => {
                return Box::new(vulkan::VkRenderer::new().ok().unwrap());
            }
            #[cfg(apple)]
            BackendType::Metal => {
                return Box::new(metal::MtlRenderer::new().ok().unwrap());
            }
            #[cfg(not(apple))]
            BackendType::OpenGL => {
                return Box::new(opengl::GLRenderer::new().ok().unwrap());
            }
        }
    } else {
        #[cfg(windows)]
        if let Ok(dx12) = dx12::Dx12Renderer::new() {
            return Box::new(dx12);
        }
        #[cfg(not(apple))]
        if let Ok(vk) = vulkan::VkRenderer::new() {
            return Box::new(vk);
        }
        #[cfg(apple)]
        if let Ok(metal) = metal::MtlRenderer::new() {
            return Box::new(metal);
        }
        #[cfg(not(apple))]
        if let Ok(opengl) = opengl::GLRenderer::new() {
            return Box::new(opengl);
        }

        panic!("Failed to initialize any renderer backend");
    }
}
