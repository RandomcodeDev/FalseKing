use super::Renderer;
use crate::platform::PlatformBackend;
use log::{error, info, warn};
use std::{
    fmt::Display,
    sync::{Arc, Mutex},
};
use vulkano::{
    command_buffer::{
        allocator::StandardCommandBufferAllocator, AutoCommandBufferBuilder, CommandBufferUsage,
        RenderPassBeginInfo, SubpassContents,
    },
    device::{physical::PhysicalDevice, Device, DeviceExtensions, Queue},
    instance::{debug::DebugUtilsMessenger, InstanceExtensions},
    pipeline::graphics::viewport::Viewport,
    render_pass::{Framebuffer, RenderPass},
    swapchain::{
        self, AcquireError, Swapchain, SwapchainCreateInfo, SwapchainCreationError,
        SwapchainPresentInfo,
    },
    sync::GpuFuture,
    sync::{self, FlushError},
};

// boilerplate stuff
mod stuff;

struct Gpu {
    device: Arc<PhysicalDevice>,
    queue_family_index: u32,
}

impl Display for Gpu {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(&self.device.properties().device_name)
    }
}

pub struct VkRenderer {
    backend: Arc<Mutex<dyn PlatformBackend>>,

    #[allow(dead_code)]
    debug_messenger: Option<DebugUtilsMessenger>,

    #[allow(dead_code)]
    gpus: Vec<Gpu>,
    #[allow(dead_code)]
    gpu_index: usize,
    device: Arc<Device>,
    queue: Arc<Queue>,
    swapchain: Arc<Swapchain>,
    command_buffer_allocator: StandardCommandBufferAllocator,
    render_pass: Arc<RenderPass>,
    viewport: Viewport,
    framebuffers: Vec<Arc<Framebuffer>>,

    swapchain_dirty: bool,
    previous_frame_end: Option<Box<dyn GpuFuture>>,
}

impl VkRenderer {
    pub fn new(backend: Arc<Mutex<dyn PlatformBackend>>) -> Option<Arc<Mutex<Self>>> {
        info!("Initializing Vulkan renderer");

        let mut instance_extensions = InstanceExtensions {
            khr_surface: true,
            #[cfg(build = "debug")]
            ext_debug_utils: true,
            ..Default::default()
        };
        backend
            .try_lock()
            .unwrap()
            .enable_vulkan_extensions(&mut instance_extensions);
        let instance_layers = vec![String::from("VK_LAYER_KHRONOS_validation")];

        let (instance, debug_messenger) =
            Self::create_instance(instance_extensions, instance_layers);
        let instance = match instance {
            Ok(instance) => instance,
            Err(err) => {
                error!("Failed to create instance: {err} ({err:?})");
                return None;
            }
        };
        let debug_messenger = if let Some(debug_messenger) = debug_messenger {
            match debug_messenger {
                Ok(debug_messenger) => Some(debug_messenger),
                Err(err) => {
                    error!("Failed to create debug messenger: {err} ({err:?})");
                    return None;
                }
            }
        } else {
            None
        };

        let surface = match backend
            .try_lock()
            .unwrap()
            .create_vulkan_surface(instance.clone())
        {
            Ok(surface) => surface,
            Err(err) => {
                error!("Failed to create surface: {err} ({err:?})");
                return None;
            }
        };

        let device_extensions = DeviceExtensions {
            khr_swapchain: true,
            ..Default::default()
        };

        let (gpus, gpu_index) =
            match Self::choose_gpu(instance.clone(), &device_extensions, surface.clone()) {
                Ok(pair) => pair,
                Err(err) => {
                    error!("Failed to find usable GPU: {err} ({err:?})");
                    return None;
                }
            };

        let (device, mut queues) = match Self::create_device(&gpus[0], device_extensions) {
            Ok(pair) => pair,
            Err(err) => {
                error!("Failed to create device: {err} ({err:?})");
                return None;
            }
        };

        let queue = match queues.next() {
            Some(queue) => queue,
            None => {
                error!("Failed to get queue");
                return None;
            }
        };

        let (swapchain, swapchain_images) =
            match Self::create_swapchain(backend.clone(), device.clone(), surface.clone()) {
                Ok(pair) => pair,
                Err(err) => {
                    error!("Failed to create swapchain: {err} ({err:?})");
                    return None;
                }
            };

        info!("Creating command buffer allocator");
        let command_buffer_allocator =
            StandardCommandBufferAllocator::new(device.clone(), Default::default());

        // shaders go here

        info!("Creating renderpass");
        let render_pass = match vulkano::single_pass_renderpass!(
            device.clone(),
            attachments: {
                color: {
                    load: Clear,
                    store: Store,
                    format: swapchain.image_format(),
                    samples: 1
                }
            },
            pass: {
                color: [color],
                depth_stencil: {}
            }
        ) {
            Ok(render_pass) => render_pass,
            Err(err) => {
                error!("Failed to create render pass: {err} ({err:?})");
                return None;
            }
        };

        let mut viewport = Viewport {
            origin: [0.0, 0.0],
            dimensions: [0.0, 0.0],
            depth_range: 0.0..1.0,
        };

        let framebuffers = match Self::create_framebuffers(
            &swapchain_images,
            render_pass.clone(),
            &mut viewport,
        ) {
            Ok(framebuffers) => framebuffers,
            Err(err) => {
                error!("Failed to create framebuffers: {err} ({err:?})");
                return None;
            }
        };

        let previous_frame_end: Option<Box<dyn GpuFuture>> =
            Some(Box::new(sync::now(device.clone())));

        Some(Arc::new(Mutex::new(Self {
            backend: backend.clone(),

            debug_messenger,

            gpus,
            gpu_index,
            device,
            queue,
            swapchain,
            command_buffer_allocator,
            render_pass,
            viewport,
            framebuffers,

            swapchain_dirty: false,
            previous_frame_end,
        })))
    }
}

// Can panic/expect because this isn't recoverable, unlike new where another renderer might be created after it returns
impl Renderer for VkRenderer {
    fn begin_frame(&mut self) {
        self.previous_frame_end
            .as_mut()
            .take()
            .unwrap()
            .cleanup_finished();

        if self.swapchain_dirty {
            let backend = self.backend.try_lock().unwrap();
            let image_extent = [backend.get_width(), backend.get_height()];

            let (new_swapchain, new_images) = match self.swapchain.recreate(SwapchainCreateInfo {
                image_extent,
                ..self.swapchain.create_info()
            }) {
                Ok(pair) => pair,
                Err(SwapchainCreationError::ImageExtentNotSupported { .. }) => return,
                Err(err) => {
                    panic!("Failed to recreate swapchain: {err} ({err:?})");
                }
            };

            self.swapchain = new_swapchain;
            self.framebuffers = Self::create_framebuffers(
                &new_images,
                self.render_pass.clone(),
                &mut self.viewport,
            )
            .expect("Failed to recreate framebuffers");
            self.swapchain_dirty = false;
        }

        let (image_index, suboptimal, acquire_future) =
            match swapchain::acquire_next_image(self.swapchain.clone(), None) {
                Ok(set) => set,
                Err(AcquireError::OutOfDate) => {
                    warn!("Swapchain is out of date");
                    self.swapchain_dirty = true;
                    return;
                }
                Err(AcquireError::Timeout) => {
                    warn!("Timed out waiting for next image");
                    return;
                }
                Err(err) => panic!("Failed to acquire next image: {err} ({err:?})"),
            };

        if suboptimal {
            warn!("Swapchain is suboptimal");
            self.swapchain_dirty = true;
        }

        let clear_values = vec![Some(
            [
                super::CLEAR_COLOR_R as f32 / 255.0,
                super::CLEAR_COLOR_G as f32 / 255.0,
                super::CLEAR_COLOR_B as f32 / 255.0,
                super::CLEAR_COLOR_A as f32 / 255.0,
            ]
            .into(),
        )];

        let mut command_buffer_builder = AutoCommandBufferBuilder::primary(
            &self.command_buffer_allocator,
            self.queue.queue_family_index(),
            CommandBufferUsage::OneTimeSubmit,
        )
        .unwrap();

        command_buffer_builder
            .begin_render_pass(
                RenderPassBeginInfo {
                    clear_values,
                    ..RenderPassBeginInfo::framebuffer(
                        self.framebuffers[image_index as usize].clone(),
                    )
                },
                SubpassContents::Inline,
            )
            .unwrap()
            .end_render_pass()
            .unwrap();

        let command_buffer = command_buffer_builder.build().unwrap();

        let future = self
            .previous_frame_end
            .take()
            .unwrap()
            .join(acquire_future)
            .then_execute(self.queue.clone(), command_buffer)
            .unwrap()
            .then_swapchain_present(
                self.queue.clone(),
                SwapchainPresentInfo::swapchain_image_index(self.swapchain.clone(), image_index),
            )
            .then_signal_fence_and_flush();

        self.previous_frame_end = match future {
            Ok(future) => Some(Box::new(future)),
            Err(FlushError::OutOfDate) => {
                warn!("Swapchain is out of date");
                self.swapchain_dirty = true;
                Some(Box::new(sync::now(self.device.clone())))
            }
            Err(err) => {
                error!("Failed to flush future: {err} ({err:?})");
                Some(Box::new(sync::now(self.device.clone())))
            }
        }
    }

    fn end_frame(&mut self) {}

    fn shutdown(&mut self) {}
}
