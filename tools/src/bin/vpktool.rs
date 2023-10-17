use std::path::PathBuf;

use clap::{Parser, Subcommand};
use common::{log::{self, warn, info, debug}, vpk};

#[derive(Debug, Subcommand)]
enum Commands {
    #[command(about = Some("Create a VPK file from a directory"))]
    Create{input_dir: PathBuf, output_vpk: PathBuf},
    #[command(about = Some("Extract a VPK file to directory"))]
    Extract{input_vpk: PathBuf, output_dir: PathBuf},
    #[command(about = Some("List the files in a VPK file"))]
    List{input_vpk: PathBuf, check_hashes: bool}
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

}

fn main() {
    let args = Args::parse();

    common::setup_logger(if args.debug {
        log::LevelFilter::Debug
    } else if args.verbose {
        log::LevelFilter::Info
    } else {
        log::LevelFilter::Warn
    }, None, true).expect("Failed to initialize logging");

    
}
