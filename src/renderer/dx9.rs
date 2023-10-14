use std::mem::MaybeUninit;
use std::sync::{Arc, Mutex};
use std::{mem, ptr};

use log::{error, info};

use super::Renderer;
use crate::platform::win32::{FALSE, TRUE};
use crate::platform::PlatformBackend;
use windows::{
    core::Result,
    Win32::{Foundation::*, Graphics::Direct3D9::*},
};

macro_rules! D3DCOLOR_ARGB {
    ($a:expr, $r:expr, $g:expr, $b:expr) => {
        ((($a as u32 & 0xff) << 24u32)
            | (($r as u32 & 0xff) << 16u32)
            | (($g as u32 & 0xff) << 8u32)
            | ($b as u32 & 0xff)) as u32
    };
}

macro_rules! D3DCOLOR_XRGB {
    ($r:expr, $g:expr, $b:expr) => {
        D3DCOLOR_ARGB!(0xff, $r, $g, $b)
    };
}

macro_rules! D3DCOLOR_RGBA {
    ($r:expr, $g:expr, $b:expr, $a:expr) => {
        D3DCOLOR_ARGB!($a, $r, $g, $b)
    };
}

pub struct Dx9Renderer {
    // Probably needs to outlive device or smth
    #[allow(dead_code)]
    d3d: IDirect3D9,
    device: IDirect3DDevice9,
}

impl Dx9Renderer {
    pub fn new(backend: Arc<Mutex<dyn PlatformBackend>>) -> Option<Arc<Mutex<Self>>> {
        info!("Initializing DirectX 9 renderer");

        let backend = backend.try_lock().unwrap();

        unsafe {
            info!("Creating IDirect3D9");
            let d3d = Direct3DCreate9(D3D_SDK_VERSION)?;

            info!("Creating IDirect3DDevice9");
            let mut present_parameters = D3DPRESENT_PARAMETERS {
                Windowed: TRUE,
                SwapEffect: D3DSWAPEFFECT_COPY,
                ..Default::default()
            };

            // rustc is bamboozled, need mut for as_mut_ptr
            #[allow(unused_mut)]
            let mut device = MaybeUninit::new(mem::zeroed());
            match d3d.CreateDevice(
                D3DADAPTER_DEFAULT,
                D3DDEVTYPE_HAL,
                HWND(backend.get_handle() as isize),
                D3DCREATE_SOFTWARE_VERTEXPROCESSING as u32,
                ptr::addr_of_mut!(present_parameters),
                device.as_mut_ptr(),
            ) {
                Ok(_) => {}
                Err(err) => {
                    error!("Failed to create IDirect3DDevice9: {err} ({err:?})");
                }
            };

            // Has been checked for errors
            let mut device = device.assume_init().unwrap();

            Some(Arc::new(Mutex::new(Self { d3d, device })))
        }
    }
}

impl Renderer for Dx9Renderer {
    fn begin_frame(&mut self) {
        unsafe {
            let _ = self.device.Clear(
                0,
                ptr::null(),
                D3DCLEAR_TARGET as u32,
                D3DCOLOR_ARGB!(
                    super::CLEAR_COLOR_A,
                    super::CLEAR_COLOR_R,
                    super::CLEAR_COLOR_G,
                    super::CLEAR_COLOR_B
                ),
                1.0,
                0,
            );

            let _ = self.device.BeginScene();
        }
    }

    fn end_frame(&mut self) {
        unsafe {
            let _ = self.device.EndScene();

            let _ = self
                .device
                .Present(ptr::null(), ptr::null(), HWND(0), ptr::null());
        }
    }

    fn shutdown(&mut self) {}
}
