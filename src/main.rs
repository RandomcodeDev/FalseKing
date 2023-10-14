#![cfg_attr(
    all(windows, not(any(build = "debug", feature = "extra_log"))),
    windows_subsystem = "windows"
)]
#![feature(fs_try_exists)]

mod platform;
mod renderer;

use chrono::Local;
use clap::Parser;
use fern::colors::{Color, ColoredLevelConfig};
//use legion::*;
use platform::PlatformBackend;
use std::io;

pub const GAME_NAME: &str = "False King";
pub const GAME_EXECUTABLE_NAME: &str = "false_king";

#[derive(Parser)]
#[command(author, version, about, long_about = None)]
struct Args {
    /// Graphics API
    #[arg(short, long, value_name = "RENDER_API")]
    render_api: Option<renderer::RenderApi>,
}

fn main() {
    setup_logger().expect("Failed to set up logger");

    let args = Args::parse();

    //let filesystem = fs::StdFileSystem::new();
    let backend = platform::get_backend_for_platform().unwrap();
    let renderer = renderer::get_renderer(backend.clone(), args.render_api);

    //let world = World::default();

    while backend.try_lock().unwrap().update() {
        renderer.try_lock().unwrap().begin_frame();

        renderer.try_lock().unwrap().end_frame();
    }

    renderer.try_lock().unwrap().shutdown();
    backend.try_lock().unwrap().shutdown();
}

fn setup_logger() -> Result<(), fern::InitError> {
    let dt = Local::now().format("%Y-%m-%d_%H-%M-%S").to_string();

    let colors_line = ColoredLevelConfig::new()
        .error(Color::Red)
        .warn(Color::Yellow)
        .info(Color::Green)
        .debug(Color::BrightCyan)
        .trace(Color::Cyan);

    let dispatch = fern::Dispatch::new()
        .format(move |out, message, record| {
            let dt = Local::now();

            let mut location = String::from(record.target());
            if let Some(file) = record.file() {
                if let Some(line) = record.line() {
                    location = format!("{file}:{line}");
                }
            }

            let level = record.level().as_str().to_lowercase();

            out.finish(format_args!(
                "[{} \x1B[{}m{}\x1B[0m {}] {}",
                dt.format("%Y/%m/%d %H:%M:%S"),
                colors_line.get_color(&record.level()).to_fg_str(),
                level,
                location,
                message
            ))
        })
        .chain(fern::log_file(
            String::from(crate::GAME_EXECUTABLE_NAME) + "-" + &dt + ".log",
        )?);

    #[cfg(all(build = "debug", feature = "extra_log"))]
    let dispatch = dispatch.level(log::LevelFilter::Trace);
    #[cfg(build = "debug")]
    let dispatch = dispatch.level(log::LevelFilter::Debug);
    #[cfg(all(not(build = "debug"), feature = "extra_log"))]
    let dispatch = dispatch.level(log::LevelFilter::Info);
    #[cfg(feature = "verbose_log")]
    let dispatch = dispatch.level(log::LevelFilter::Trace);
    #[cfg(any(build = "debug", all(not(build = "debug"), feature = "extra_log")))]
    let dispatch = dispatch.chain(io::stdout());

    dispatch.apply()?;

    Ok(())
}
