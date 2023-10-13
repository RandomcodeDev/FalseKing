use std::{
    env, fs,
    path::{Path, PathBuf},
    process::Command,
};

fn put_file<P: AsRef<Path>, Q: AsRef<Path>>(from: P, to: Q) {
    #[cfg(windows)]
    let _ = match fs::metadata(&from) {
        Ok(metadata) => {
            if metadata.is_dir() {
                std::os::windows::fs::symlink_dir(from, to)
            } else {
                std::os::windows::fs::symlink_file(from, to)
            }
        }
        Err(err) => Err(err),
    };
    #[cfg(unix)]
    let _ = std::os::unix::fs::symlink(from, to);
}

fn main() {
    let profile = env::var("PROFILE").unwrap();
    let root = env::current_dir().unwrap();
    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());

    //let target_family = env::var("CARGO_CFG_TARGET_FAMILY").unwrap();
    let target_os = env::var("CARGO_CFG_TARGET_OS").unwrap();
    let target_arch = env::var("CARGO_CFG_TARGET_ARCH").unwrap();

    let output_dir = Path::new("out");
    let _ = fs::create_dir_all("out");
    put_file(
        root.join("assets_000.vpk"),
        output_dir.join("assets_000.vpk"),
    );
    put_file(
        root.join("assets_dir.vpk"),
        output_dir.join("assets_dir.vpk"),
    );

    if target_os == "windows" && target_arch == "x86_64" {
        put_file(root.join("gdk").join("Assets"), output_dir.join("Assets"));
        put_file(
            root.join("gdk").join("MicrosoftGameConfig.mgc"),
            output_dir.join("MicrosoftGame.config"),
        );
        if let Ok(gamedk) = env::var("GameDK") {
            let gdk_dir = PathBuf::from(gamedk);
            let makepkg = gdk_dir.join("bin").join("makepkg.exe");
            let layout = out_dir.join("layout.xml");

            let mut extra_args = Vec::new();
            if env::var("CARGO_FEATURE_XBOX").is_ok() {
            } else {
                extra_args.push("/pc")
            }

            match Command::new(makepkg.clone())
                .arg("genmap")
                .arg("/f")
                .arg(layout.clone())
                .arg("/d")
                .arg(output_dir)
                .spawn()
            {
                Ok(_) => {}
                Err(err) => println!("cargo:warning={err}"),
            }
            match Command::new(makepkg.clone())
                .arg("pack")
                .arg("/f")
                .arg(layout.clone())
                .arg("/lt")
                .arg("/d")
                .arg(output_dir)
                .arg("/nogameos")
                .arg("/pd")
                .arg(root)
                .args(extra_args)
                .spawn()
            {
                Ok(_) => {}
                Err(err) => println!("cargo:warning={err}"),
            }
        }
    }

    println!("cargo:rustc-cfg=build={:?}", profile);
}
