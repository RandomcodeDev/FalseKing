use std::path::PathBuf;

use clap::{Parser, Subcommand};
use common::vpk;

#[derive(Subcommand)]
enum Commands {
    Create{input_dir: PathBuf, output_vpk: PathBuf},
    Extract{input_vpk: PathBuf, output_dir: PathBuf},
    List{input_vpk: PathBuf, check_hashes: bool}
}

#[derive(Parser)]
#[command(author, version, about, long_about = Some("Manipulates Valve Pack Files"))]
#[command(propagate_version = true)]
struct Args {
    #[command(subcommand)]
    command: Commands,

    #[arg(short, long, default_value_t = false)]
    verbose: bool,

    #[arg(short, long, default_value_t = false)]
    debug: bool,
}

fn main() {}
