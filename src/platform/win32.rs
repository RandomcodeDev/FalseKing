use super::PlatformBackend;
use log::info;
use std::{ffi, mem, ptr, sync::{Arc, Mutex}};
use windows::Win32::{
    Foundation::*, Graphics::Gdi::*, System::LibraryLoader::*, UI::WindowsAndMessaging::*,
};
use windows_core::*;

pub struct Win32Backend {
    hinstance: HINSTANCE,
    window: HWND,
    width: i32,
    height: i32,
    closed: bool,
    resized: bool,
    focused: bool,
}

impl Win32Backend {
    pub fn new() -> Option<Arc<Mutex<Self>>> {
        info!("Initializing Windows backend");

        let hinstance = unsafe {
            GetModuleHandleA(PCSTR(ptr::null()))
                .unwrap_or(HMODULE(0))
                .into()
        };
        let class_name = PCSTR("FalseKingWindowClass\0".as_ptr());

        let wndclass = WNDCLASSEXA {
            cbSize: mem::size_of::<WNDCLASSEXA>() as u32,
            style: WNDCLASS_STYLES(0),
            lpfnWndProc: Some(Self::window_procedure),
            cbClsExtra: 0,
            cbWndExtra: 0,
            hInstance: hinstance,
            hIcon: HICON(0),
            hCursor: HCURSOR(0),
            hbrBackground: HBRUSH(0),
            lpszClassName: class_name,
            lpszMenuName: PCSTR(ptr::null()),
            hIconSm: HICON(0),
        };
        unsafe { RegisterClassExA(ptr::addr_of!(wndclass)) };

        let width = 1024;
        let height = 576;

        // Make the window 1024x576 internally
        let mut client_area = RECT {
            left: 0,
            right: width,
            top: 0,
            bottom: height,
        };
        let _ = unsafe {
            AdjustWindowRect(
                ptr::addr_of_mut!(client_area),
                WS_OVERLAPPEDWINDOW,
                BOOL(false as i32),
            )
        };

        info!("Creating {width}x{height} window {}", crate::GAME_NAME);

        let title = ffi::CString::new(crate::GAME_NAME).unwrap();

        let window = unsafe {
            CreateWindowExA(
                WINDOW_EX_STYLE(0),
                wndclass.lpszClassName,
                PCSTR(title.as_ptr() as *const u8),
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                client_area.right - client_area.left,
                client_area.bottom - client_area.top,
                HWND(0),
                HMENU(0),
                hinstance,
                None,
            )
        };

        unsafe { ShowWindow(window, SW_SHOWNORMAL) };

        Some(Arc::new(Mutex::new(Self {
            hinstance,
            window,
            width,
            height,
            closed: false,
            resized: false,
            focused: false,
        })))
    }

    extern "system" fn window_procedure(
        window: HWND,
        message: u32,
        wparam: WPARAM,
        lparam: LPARAM,
    ) -> LRESULT {
        let self_ = match unsafe { Self::get_self_ptr(window) } {
            None => unsafe {
                return DefWindowProcA(window, message, wparam, lparam);
            },
            Some(self_) => self_,
        };

        match message {
            WM_SIZE => {
                let mut client_area: RECT = unsafe { mem::zeroed() };

                let _ = unsafe { GetClientRect(window, ptr::addr_of_mut!(client_area)) };
                let new_width = client_area.right - client_area.left;
                let new_height = client_area.bottom - client_area.top;

                if new_width != self_.width || new_height != self_.height {
                    info!(
                        "Window resized from {}x{} to {}x{}",
                        self_.width, self_.height, new_width, new_height
                    );
                    self_.resized = true;
                    self_.width = new_width;
                    self_.height = new_height;
                }

                LRESULT(0)
            }
            WM_ACTIVATEAPP => {
                self_.focused = wparam != WPARAM(0);

                info!(
                    "Window {}",
                    if self_.focused {
                        "focused"
                    } else {
                        "unfocused"
                    }
                );

                LRESULT(0)
            }
            WM_CLOSE => {
                info!("Window closed");
                self_.closed = true;
                LRESULT(0)
            }
            _ => unsafe { DefWindowProcA(window, message, wparam, lparam) },
        }
    }

    unsafe fn get_self_ptr<'a>(window: HWND) -> Option<&'a mut Self> {
        match GetWindowLongPtrA(window, GWLP_USERDATA) {
            0 => {
                None
            }
            addr => (addr as *mut Self).as_mut(),
        }
    }

    unsafe fn set_self_ptr(&mut self) {
        SetWindowLongPtrA(
            self.window,
            GWLP_USERDATA,
            ptr::addr_of_mut!(*self) as isize,
        );
        SetWindowPos(
            self.window,
            HWND(0),
            0,
            0,
            0,
            0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED,
        )
        .expect("Failed to update self pointer in window");
    }
}

impl PlatformBackend for Win32Backend {
    fn shutdown(&mut self) {
        info!("Shutting down Windows backend");

        unsafe {
            let _ = DestroyWindow(self.window);
        }
    }

    fn update(&mut self) -> bool {
        unsafe { self.set_self_ptr() };

        let mut msg: MSG = unsafe { mem::zeroed() };
        while unsafe { PeekMessageA(ptr::addr_of_mut!(msg), HWND(0), 0, 0, PM_REMOVE) } != BOOL(0) {
            unsafe {
                TranslateMessage(ptr::addr_of!((msg)));
                DispatchMessageA(ptr::addr_of!(msg));
            }
        }

        !self.closed
    }

    fn get_handle(&self) -> usize {
        self.window.0 as usize
    }

    fn has_resized(&self) -> bool {
        self.resized
    }

    fn get_width(&self) -> u32 {
        self.width as u32
    }

    fn get_height(&self) -> u32 {
        self.height as u32
    }

    fn is_focused(&self) -> bool {
        self.focused
    }

    fn enable_vulkan_extensions(&self, extensions: &mut vulkano::instance::InstanceExtensions) {
        extensions.khr_win32_surface = true;
    }

    fn create_vulkan_surface(
        &self,
        instance: std::sync::Arc<vulkano::instance::Instance>,
    ) -> std::result::Result<
        Arc<vulkano::swapchain::Surface>,
        vulkano::swapchain::SurfaceCreationError,
    > {
        info!("Creating Windows surface");
        unsafe {
            vulkano::swapchain::Surface::from_win32(
                instance,
                self.hinstance.0 as *const ffi::c_void,
                self.window.0 as *const ffi::c_void,
                None,
            )
        }
    }
}
