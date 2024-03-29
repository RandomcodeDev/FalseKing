use std::{
    io,
    path::{Path, PathBuf},
};

use clap::{Parser, Subcommand};
use common::{
    fs::{self, FileSystem},
    log::{self, error, info},
    vpk,
};

#[derive(Debug, Subcommand)]
enum Commands {
    #[command(about = Some("Create a VPK file from a directory"))]
    Create {
        input_dir: PathBuf,
        output_vpk: Option<PathBuf>,
    },
    #[command(about = Some("Extract a VPK file to directory"))]
    Extract {
        input_vpk: PathBuf,
        output_dir: Option<PathBuf>,
    },
    #[command(about = Some("List the files in a VPK file"))]
    List {
        input_vpk: PathBuf,
        #[arg(short, long)]
        check_hashes: bool,
    },
}

#[derive(Debug, Parser)]
#[command(author, version, about, long_about = Some("Manipulates Valve Pack Files"))]
#[command(propagate_version = true)]
struct Args {
    #[command(subcommand)]
    command: Commands,

    /// Check hashes when listing files
    #[arg(short, long, default_value_t = false)]
    check_hashes: bool,

    /// Be verbose
    #[arg(short, long, default_value_t = false)]
    verbose: bool,

    /// Be extremely verbose
    #[arg(short, long, default_value_t = false)]
    debug: bool,
}

fn create(input_dir: PathBuf, output_vpk: Option<PathBuf>) -> io::Result<()> {
    let mut output_vpk = if output_vpk.is_some() {
        output_vpk.unwrap()
    } else {
        input_dir.clone()
    };
    if output_vpk.ends_with("/") || output_vpk.ends_with("\\") {
        output_vpk.pop();
    }
    output_vpk.set_extension(vpk::VPK_EXTENSION);

    println!(
        "Creating VPK {} from {}",
        input_dir.display(),
        output_vpk.display()
    );

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
            let mut path = entry.path();
            path = PathBuf::from(
                path.strip_prefix(input_dir.as_path())
                    .expect("Failed to make path relative"),
            );
            info!("{} -> {}", entry.path().display(), path.display());
            vpk.as_mut()
                .unwrap()
                .write(path.as_path(), data.as_slice())
                .expect(
                    format!("Failed to write file {} into VPK", entry.path().display()).as_str(),
                );
        },
        &mut Some(&mut vpk),
    )
    .expect("Failed to add files to VPK");

    println!("Saving VPK to {}", output_vpk.display());
    vpk.save(output_vpk.as_path()).expect("Failed to save VPK");

    Ok(())
}

fn extract(input_vpk: PathBuf, output_dir: Option<PathBuf>) -> io::Result<()> {
    let base_path = vpk::vpk2::Vpk2::get_base_path(input_vpk.as_path())
        .expect("Failed to get base name of VPK");

    let mut output_dir = if output_dir.is_some() {
        output_dir.unwrap()
    } else {
        base_path.clone()
    };
    output_dir.set_extension("");

    println!(
        "Extracting VPK {} to {}",
        base_path.display(),
        output_dir.display()
    );

    let vpk = vpk::vpk2::Vpk2::new(base_path.as_path(), false).expect("Failed to open VPK");

    for entry in vpk.read_dir(PathBuf::from("").as_path())? {
        match entry {
            Ok(entry) => {
                let mut output_path = output_dir.clone();
                output_path.push(entry.path().file_name().expect("No path"));
                info!("{} -> {}", entry.path().display(), output_path.display());
            }
            Err(err) => {
                error!("Failed to extract file: {err}");
            }
        }
    }

    Ok(())
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

    let _ = match args.command {
        Commands::Create {
            input_dir,
            output_vpk,
        } => create(input_dir, output_vpk),
        Commands::Extract {
            input_vpk,
            output_dir,
        } => extract(input_vpk, output_dir),
        Commands::List {
            input_vpk,
            check_hashes,
        } => Ok(()),
    };
}
