use super::Renderer;
use crate::platform::PlatformBackend;
use common::fs;
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
    instance::{debug::DebugUtilsMessenger, InstanceCreationError, InstanceExtensions},
    pipeline::graphics::viewport::Viewport,
    render_pass::{Framebuffer, Subpass, RenderPass},
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

pub struct VkRenderer<R> {
    backend: Arc<Mutex<dyn PlatformBackend>>,
    filesystem: Arc<Mutex<dyn fs::FileSystem<ReadDir = R>>>,

    // Needs to live
    #[allow(dead_code)]
    debug_messenger: Option<DebugUtilsMessenger>,

    // Will be used for dynamically selecting the GPU at runtime
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

impl<R> VkRenderer<R> {
    pub fn new(
        backend: Arc<Mutex<dyn PlatformBackend>>,
        filesystem: Arc<Mutex<dyn fs::FileSystem<ReadDir = R>>>,
    ) -> Option<Arc<Mutex<Self>>> {
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

        let (mut instance, mut debug_messenger) =
            Self::create_instance(instance_extensions, instance_layers);
        let instance = match instance {
            Ok(instance) => instance,
            Err(InstanceCreationError::LayerNotPresent) => {
                warn!("Missing validation layer, trying to create instance without them");
                (instance, debug_messenger) =
                    Self::create_instance(instance_extensions, Vec::new());
                match instance {
                    Ok(instance) => instance,
                    Err(err) => {
                        error!("Failed to create instance: {err} ({err:?})");
                        return None;
                    }
                }
            }
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

        let (vertex_shader, pixel_shader) =
            match Self::load_shader(filesystem.clone(), device.clone(), "main") {
                Ok(pair) => pair,
                Err(err) => {
                    error!("Failed to load shader main: {err} ({err:?})");
                    return None;
                }
            };

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

        info!("Creating graphics pipeline");
        let pipeline = match
        GraphicsPipeline::start()
            .vertex_input_state(BuffersDefinition::new().vertex::<Vertex>())
            .vertex_shader(vertex_shader.entry_point("main").unwrap(), ())
            .input_assembly_state(InputAssemblyState::new())
            .viewport_state(ViewportState::vieport_dynamic_scissor_irrelevant())
            .fragment_shader(pixel_shader.entry_point("main").unwrap(), ())
            .render_pass(Subpass::from(render_pass.clone(), 0).unwrap())
            .build(device.clone()) {
            Ok(pipeline) => pipeline,
            Err(err) => {
                error!("Failed to create pipeline: {err} ({err:?})");
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

        // clippy is tweaking it's literally in a mutex
        #[allow(clippy::arc_with_non_send_sync)]
        Some(Arc::new(Mutex::new(Self {
            backend: backend.clone(),
            filesystem: filesystem.clone(),

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

// Can panic/unwrap/expect because this isn't recoverable, unlike new where another renderer might be created after it returns
impl<R> Renderer<R> for VkRenderer<R> {
    fn begin_frame(&mut self) {
        self.previous_frame_end
            .as_mut()
            .take()
            .unwrap()
            .cleanup_finished();

        if self.swapchain_dirty {
            info!("Recreating swapchain");

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
            return;
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
