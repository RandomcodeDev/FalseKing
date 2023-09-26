mod platform;

use chrono::Local;
use fern::colors::{Color, ColoredLevelConfig};
use platform::PlatformBackend;
use std::{io};

pub const GAME_NAME: &str = "False King";

fn main() {
    setup_logger().expect("Failed to set up logger");

    let mut backend = platform::get_backend_for_platform().unwrap();

    while backend.update() {

    }

    backend.shutdown();
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
            out.finish(format_args!(
                "[{} \x1B[{}m{}\x1B[0m {}] {}",
                dt.format("%Y-%m-%d %H:%M:%S"),
                colors_line.get_color(&record.level()).to_fg_str(),
                record.level(),
                record.target(),
                message
            ))
        })
        .chain(fern::log_file(
            String::from(crate::GAME_NAME) + " " + &dt + ".log",
        )?);

    #[cfg(build = "debug")]
    let dispatch = dispatch.level(log::LevelFilter::Debug);
    #[cfg(all(not(build = "debug"), feature = "release_log"))]
    let dispatch = dispatch.level(log::LevelFilter::Info);
    #[cfg(feature = "verbose_log")]
    let dispatch = dispatch.level(log::LevelFilter::Trace);
    #[cfg(any(build = "debug", all(not(build = "debug"), feature = "release_log")))]
    let dispatch = dispatch.chain(io::stdout());

    dispatch.apply()?;

    Ok(())
}
