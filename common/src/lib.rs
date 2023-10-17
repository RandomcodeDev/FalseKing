#![feature(fs_try_exists)]

pub mod fs;
pub mod util;
pub mod vpk;

use chrono::Local;
use fern::colors::{Color, ColoredLevelConfig};
use std::io;

pub use log;

pub fn setup_logger(level: log::LevelFilter, name: Option<String>, stdout: bool) -> Result<(), fern::InitError> {
    let dt = Local::now().format("%Y-%m-%d_%H-%M-%S").to_string();

    let colors_line = ColoredLevelConfig::new()
        .error(Color::Red)
        .warn(Color::Yellow)
        .info(Color::Green)
        .debug(Color::BrightCyan)
        .trace(Color::Cyan);

    let mut dispatch = fern::Dispatch::new()
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
        });

    if name.is_some() {
        dispatch = dispatch.chain(fern::log_file(
            name.unwrap() + "-" + &dt + ".log",
        )?);
    }

    dispatch = dispatch.level(level);

    if stdout {
        dispatch = dispatch.chain(io::stdout());
    }

    dispatch.apply()?;

    Ok(())
}