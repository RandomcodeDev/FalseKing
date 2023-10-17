use std::{
    io,
    path::{Path, PathBuf},
};

use clap::{Parser, Subcommand};
use common::{
    fs::{self, FileSystem},
    log, vpk,
};

#[derive(Debug, Subcommand)]
enum Commands {
    #[command(about = Some("Create a VPK file from a directory"))]
    Create {
        input_dir: PathBuf,
        output_vpk: PathBuf,
    },
    #[command(about = Some("Extract a VPK file to directory"))]
    Extract {
        input_vpk: PathBuf,
        output_dir: PathBuf,
    },
    #[command(about = Some("List the files in a VPK file"))]
    List {
        input_vpk: PathBuf,
        check_hashes: bool,
    },
}

#[derive(Debug, Parser)]
#[command(author, version, about, long_about = Some("Manipulates Valve Pack Files"))]
#[command(propagate_version = true)]
struct Args {
    #[command(subcommand)]
    command: Commands,

    #[arg(short, long, default_value_t = false)]
    check_hashes: bool,

    #[arg(short, long, default_value_t = false)]
    verbose: bool,

    #[arg(short, long, default_value_t = false)]
    debug: bool,
}

fn create(input_dir: PathBuf, output_vpk: PathBuf) {
    println!("Creating VPK {} from {}", input_dir.display(), output_vpk.display());

    let mut vpk = vpk::vpk2::Vpk2::new(&output_vpk, true).expect("Failed to create VPK");

    fn visit_dirs<T>(
        dir: &Path,
        cb: &dyn Fn(&fs::DirEntry, &mut Option<T>),
        data: &mut Option<T>,
    ) -> io::Result<()> {
        if dir.is_dir() {
            for entry in std::fs::read_dir(dir)? {
                let entry: fs::DirEntry = entry?.into();
                let path = entry.path();
                if path.is_dir() {
                    visit_dirs(&path, cb, data)?;
                } else {
                    cb(&entry, data);
                }
            }
        }
        Ok(())
    }

    visit_dirs(
        input_dir.as_ref(),
        &|entry: &fs::DirEntry, vpk: &mut Option<&mut vpk::vpk2::Vpk2>| {
            let data = std::fs::read(entry.path())
                .expect(format!("Failed to read file {}", entry.path().display()).as_str());
            vpk.as_mut().unwrap().write(entry.path(), data).expect(
                format!("Failed to write file {} into VPK", entry.path().display()).as_str(),
            );
        },
        &mut Some(&mut vpk),
    ).expect("Failed to add files to VPK");

    println!("Saving VPK to {}", output_vpk.display());
    vpk.save(output_vpk).expect("Failed to save VPK");
}

fn main() {
    let args = Args::parse();

    common::setup_logger(
        if args.debug {
            log::LevelFilter::Debug
        } else if args.verbose {
            log::LevelFilter::Info
        } else {
            log::LevelFilter::Warn
        },
        None,
        true,
    )
    .expect("Failed to initialize logging");

    match args.command {
        Commands::Create { input_dir, output_vpk } => create(input_dir, output_vpk),
        Commands::Extract { input_vpk, output_dir } => {}
        Commands::List { input_vpk, check_hashes } => {}
    }
}
