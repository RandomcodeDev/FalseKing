use super::PlatformBackend;
use log::{info};
use std::{mem, ptr};
use windows::Win32::{
    Foundation::*, Graphics::Gdi::*, System::LibraryLoader::*, UI::WindowsAndMessaging::*,
};
use windows_core::*;

pub struct Win32Backend {
    window: HWND,
    width: i32,
    height: i32,
    closed: bool,
    resized: bool,
    focused: bool,
}

impl Win32Backend {
    pub fn new() -> Option<Self> {
        info!("Initializing Windows backend");

        let hinstance = unsafe {
            GetModuleHandleA(PCSTR(ptr::null()))
                .unwrap_or(HMODULE(0))
                .into()
        };
        let class_name = PCSTR("FalseKingWindowClass".as_ptr());

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

        info!("Creating {}x{} window {}", width, height, crate::GAME_NAME);

        let window = unsafe {
            CreateWindowExA(
                WINDOW_EX_STYLE(0),
                wndclass.lpszClassName,
                PCSTR(crate::GAME_NAME.as_ptr()),
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                width,
                height,
                HWND(0),
                HMENU(0),
                hinstance,
                None,
            )
        };

        unsafe { ShowWindow(window, SW_SHOWNORMAL) };

        Some(Self {
            window,
            width,
            height,
            closed: false,
            resized: false,
            focused: false,
        })
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

                unsafe { GetClientRect(window, ptr::addr_of_mut!(client_area)) }
                    .expect("Failed to get window client area");
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
                return None;
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
    fn shutdown(self) {
        info!("Shutting down Windows backend");

        unsafe {
            DestroyWindow(self.window).expect("Failed to destroy window");
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

        return !self.closed;
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
}
