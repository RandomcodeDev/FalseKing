use lazy_static::lazy_static;
use std::{
    fs, io,
    path::{Path, PathBuf},
};

lazy_static! {
    pub static ref DATA_DIR: PathBuf = {
        let base_dirs = directories::BaseDirs::new().unwrap();
        let mut subdir_path = PathBuf::from(base_dirs.data_dir());
        subdir_path.push(crate::GAME_NAME);
        subdir_path
    };
    pub static ref LOG_DIR: PathBuf = {
        let mut log_dir = DATA_DIR.clone();
        log_dir.push("logs");
        log_dir
    };
}

pub fn data_dir() -> &'static Path {
    DATA_DIR.as_path()
}

pub fn log_dir() -> &'static Path {
    LOG_DIR.as_path()
}

pub fn make_dirs() -> io::Result<()> {
    fs::create_dir_all(data_dir())?;
    fs::create_dir_all(log_dir())?;
    Ok(())
}
