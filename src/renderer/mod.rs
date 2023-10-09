#[cfg(windows)]
mod dx12;
#[cfg(not(apple))]
mod vulkan;
#[cfg(windows)]
mod dx9;
#[cfg(apple)]
mod metal;

use std::{fmt::Display, sync::{Arc, Mutex}};

use crate::platform::PlatformBackend;
use log::error;

/// Background colour
const CLEAR_COLOR_R: u8 = 135;
const CLEAR_COLOR_G: u8 = 0;
const CLEAR_COLOR_B: u8 = 255;
const CLEAR_COLOR_A: u8 = 255;

#[derive(Clone, clap::ValueEnum)]
pub enum RenderApi {
    /// DirectX 12 (not implemented yet)
    #[cfg(windows)]
    Dx12,

    /// Vulkan
    #[cfg(not(apple))]
    Vulkan,

    /// DirectX 9 (for Windows XP)
    #[cfg(windows)]
    Dx9,

    /// Metal (gonna be a long time)
    #[cfg(apple)]
    Metal,
}

impl Display for RenderApi {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(match self {
            #[cfg(windows)]
            Self::Dx12 => "DirectX 12",
            #[cfg(not(apple))]
            Self::Vulkan => "Vulkan",
            #[cfg(windows)]
            Self::Dx9 => "DirectX 9",
            #[cfg(apple)]
            Self::Metal => "Metal",
        })
    }
}

pub trait Renderer {
    /// Prepare to take rendering commands
    fn begin_frame(&mut self);

    /// Send rendering commands to the GPU and swap buffers or whatever
    fn end_frame(&mut self);

    /// Shut down rendering (TODO: is this actually necessary when using safe-ish high-level-ish bindings?)
    fn shutdown(&mut self);
}

/// Creates an instance of the requested backend, or the first one that initializes successfully
pub fn get_renderer(backend: Arc<Mutex<dyn PlatformBackend>>, api: Option<RenderApi>) -> Arc<Mutex<dyn Renderer>> {
    // This function has a bunch of return statements that aren't necessarily executed
    #[allow(unreachable_code)]
    if let Some(api) = api {
        match api {
            #[cfg(windows)]
            RenderApi::Dx12 => match dx12::Dx12Renderer::new(backend.clone()) {
                Some(dx12) => {
                    return dx12;
                }
                None => {
                    panic!("Failed to create DirectX 12 renderer");
                }
            },
            #[cfg(not(apple))]
            RenderApi::Vulkan => match vulkan::VkRenderer::new(backend.clone()) {
                Some(vk) => return vk,
                None => {
                    panic!("Failed to create Vulkan renderer");
                }
            },
            #[cfg(windows)]
            RenderApi::Dx9 => match dx9::Dx9Renderer::new(backend.clone()) {
                Some(dx9) => {
                    return dx9;
                }
                None => {
                    panic!("Failed to create DirectX 9 renderer");
                }
            },
            #[cfg(apple)]
            RenderApi::Metal => match metal::MtlRenderer::new(backend.clone()) {
                Some(mtl) => return mtl,
                None => {
                    panic!("Failed to create Metal renderer");
                }
            },
        }
    } else {
        /*#[cfg(windows)]
        match dx12::Dx12Renderer::new(backend.clone()) {
            Some(dx12) => {
                return dx12;
            }
            None => {
                error!("Failed to create DirectX 12 renderer");
            }
        }*/
        #[cfg(not(apple))]
        match vulkan::VkRenderer::new(backend.clone()) {
            Some(vk) => {
                return vk;
            }
            None => {
                error!("Failed to create Vulkan renderer");
            }
        }
        #[cfg(windows)]
        match dx9::Dx9Renderer::new(backend.clone()) {
            Some(dx9) => {
                return dx9;
            }
            None => {
                error!("Failed to create DirectX 9 renderer");
            }
        }
        #[cfg(apple)]
        match metal::MtlRenderer::new(backend.clone()) {
            Some(mtl) => {
                return mtl;
            }
            None => {
                error!("Failed to create Metal renderer");
            }
        }
    }

    panic!("Failed to create any renderer!");
}
