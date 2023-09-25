use std::ffi;

use xcb::{x, Xid};

use super::PlatformBackend;

pub struct UnixBackend {
    connection: xcb::Connection,
    window: x::Window,
    width: u32,
    height: u32,
}

impl UnixBackend {
    pub fn new() -> xcb::Result<Self> {
        let (connection, screen_number) = xcb::Connection::connect(None)?;

        let setup = connection.get_setup();
        let screen = setup.roots().nth(screen_number as usize).unwrap();

        let window: x::Window = connection.generate_id();

        let width = 1024;
        let height = 576;

        let window_cookie = connection.send_request_checked(&x::CreateWindow {
            depth: x::COPY_FROM_PARENT as u8,
            wid: window,
            parent: screen.root(),
            x: 0,
            y: 0,
            width,
            height,
            border_width: 0,
            class: x::WindowClass::InputOutput,
            visual: screen.root_visual(),
            value_list: &[
                x::Cw::BackPixel(screen.white_pixel()),
                x::Cw::EventMask(
                    x::EventMask::NO_EVENT
                        | x::EventMask::KEY_PRESS
                        | x::EventMask::KEY_RELEASE
                        | x::EventMask::BUTTON_PRESS
                        | x::EventMask::BUTTON_RELEASE
                        | x::EventMask::ENTER_WINDOW
                        | x::EventMask::LEAVE_WINDOW
                        | x::EventMask::POINTER_MOTION
                        | x::EventMask::POINTER_MOTION_HINT
                        | x::EventMask::BUTTON1_MOTION
                        | x::EventMask::BUTTON2_MOTION
                        | x::EventMask::BUTTON3_MOTION
                        | x::EventMask::BUTTON4_MOTION
                        | x::EventMask::BUTTON5_MOTION
                        | x::EventMask::BUTTON_MOTION
                        | x::EventMask::KEYMAP_STATE
                        | x::EventMask::EXPOSURE
                        | x::EventMask::VISIBILITY_CHANGE
                        | x::EventMask::STRUCTURE_NOTIFY
                        | x::EventMask::RESIZE_REDIRECT
                        | x::EventMask::SUBSTRUCTURE_NOTIFY
                        | x::EventMask::SUBSTRUCTURE_REDIRECT
                        | x::EventMask::FOCUS_CHANGE
                        | x::EventMask::PROPERTY_CHANGE
                        | x::EventMask::COLOR_MAP_CHANGE
                        | x::EventMask::OWNER_GRAB_BUTTON,
                ), // every kind of event
            ],
        });
        connection.check_request(window_cookie)?;

        let cookie = connection.send_request_checked(&x::ChangeProperty {
            mode: x::PropMode::Replace,
            window,
            property: x::ATOM_WM_NAME,
            r#type: x::ATOM_STRING,
            data: b"False King",
        });
        connection.check_request(cookie)?;

        connection.send_request(&x::MapWindow { window });

        Self {}
    }
}

impl PlatformBackend for UnixBackend {
    fn shutdown(mut self) {}

    fn update(&mut self) -> bool {
        return true;
    }

    fn get_width(&self) -> u32 {
        self.width
    }

    fn get_height(&self) -> u32 {
        self.height
    }
}
