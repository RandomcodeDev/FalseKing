use log::info;
use std::mem;
use super::PlatformBackend;
use xcb::x;

pub struct UnixBackend {
    connection: xcb::Connection,
    window: x::Window,
    width: u16,
    height: u16,
    closed: bool,
    resized: bool,
    focused: bool
}

impl UnixBackend {
    pub fn new() -> xcb::Result<Self> {
        info!("Initializing Unix backend");

        let (connection, screen_number) = xcb::Connection::connect(None)?;

        let setup = connection.get_setup();
        let screen = setup.roots().nth(screen_number as usize).unwrap();

        let window: x::Window = connection.generate_id();

        let width: u16 = 1024;
        let height: u16 = 576;

        info!("Creating {}x{} window {}", width, height, crate::GAME_NAME);

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
            data: crate::GAME_NAME.as_bytes(),
        });
        connection.check_request(cookie)?;

        connection.send_request(&x::MapWindow { window });

        let wm_protocols = Self::get_intern_atom(&connection, "WM_PROTOCOLS")?;
        let wm_delete_window = Self::get_intern_atom(&connection, "WM_DELETE_WINDOW")?;

        let _ = connection.check_request(connection.send_request_checked(&x::ChangeProperty {
            mode: x::PropMode::Replace,
            window,
            property: wm_protocols,
            r#type: x::ATOM_ATOM,
            data: &[wm_delete_window]
        }));

        Ok(Self {
            connection,
            window,
            width,
            height,
            closed: false,
            resized: false,
            focused: false
        })
    }

    fn get_intern_atom(connection: &xcb::Connection, name: &str) -> xcb::Result<x::Atom> {
        let cookie = connection.send_request(&x::InternAtom {
            only_if_exists: true,
            name: name.as_bytes()
        });

        Ok(connection.wait_for_reply(cookie)?.atom())
    }
}

impl PlatformBackend for UnixBackend {
    fn shutdown(self) {
        info!("Shutting down Unix backend");

        self.connection.send_request(&x::DestroyWindow {
            window: self.window
        });
    }

    fn update(&mut self) -> bool {
        if let Ok(Some(xcb::Event::X(event))) = self.connection.poll_for_event() {
            match event {
                x::Event::ConfigureNotify(ev) => {
                    let new_width = ev.width();
                    let new_height = ev.height();

                    if new_width != self.width || new_height != self.height {
                        self.resized = true;
                        info!(
                            "Window resized from {}x{} to {}x{}",
                            self.width, self.height, new_width, new_height
                        );
                        self.width = new_width;
                        self.height = new_height;
                    }
                }
                x::Event::FocusIn(_) => {
                    info!("Window focused");
                    self.focused = true;
                }
                x::Event::FocusOut(_) => {
                    info!("Window unfocused");
                    self.focused = false;
                }
                x::Event::ClientMessage(ev) => {
                    if let x::ClientMessageData::Data32(atom) = ev.data() {
                        info!("Window closed");
                        if let Ok(delete_atom) = Self::get_intern_atom(&self.connection, "WM_DELETE_WINDOW") {
                            let atom = unsafe { mem::transmute::<u32, x::Atom>(atom[0]) };
                            self.closed = atom == delete_atom;
                        }
                    }
                }
                _ => {}
            }
        }

        !self.closed
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
