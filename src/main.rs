#![cfg_attr(
    all(windows, not(any(build = "debug", feature = "extra_log"))),
    windows_subsystem = "windows"
)]
#![feature(fs_try_exists)]

mod platform;
mod renderer;

use clap::Parser;
use common::fs;
//use legion::*;
use log::info;
use platform::PlatformBackend;
use std::{
    env,
    sync::{Arc, Mutex},
};

#[derive(Parser)]
#[command(author, version, about, long_about = None)]
struct Args {
    /// Graphics API
    #[arg(short, long, value_name = "RENDER_API")]
    render_api: Option<renderer::RenderApi>,
}

fn main() {
    #[cfg(all(build = "debug", feature = "extra_log"))]
    let level = log::LevelFilter::Trace;
    #[cfg(build = "debug")]
    let level = log::LevelFilter::Debug;
    #[cfg(all(not(build = "debug"), feature = "extra_log"))]
    let level = log::LevelFilter::Info;
    #[cfg(feature = "verbose_log")]
    let level = log::LevelFilter::Trace;
    #[cfg(any(build = "debug", all(not(build = "debug"), feature = "extra_log")))]
    let stdout = true;
    #[cfg(not(any(build = "debug", all(not(build = "debug"), feature = "extra_log"))))]
    let stdout = false;

    common::dirs::make_dirs().expect("Failed to create data directories");

    common::setup_logger(
        level,
        Some(String::from(common::GAME_EXECUTABLE_NAME)),
        stdout,
    )
    .expect("Failed to set up logger");

    let args = Args::parse();

    if let Ok(cwd) = env::current_dir() {
        info!(
            "Running in {} with data directory {}",
            cwd.display(),
            common::dirs::data_dir().display()
        );
    }

    let filesystem = Arc::new(Mutex::new(fs::StdFileSystem::new()));
    let backend = platform::get_backend_for_platform().unwrap();
    let renderer = renderer::get_renderer(backend.clone(), filesystem.clone(), args.render_api);

    //let world = World::default();

    while backend.try_lock().unwrap().update() {
        renderer.try_lock().unwrap().begin_frame();

        renderer.try_lock().unwrap().end_frame();
    }

    renderer.try_lock().unwrap().shutdown();
    backend.try_lock().unwrap().shutdown();
}
