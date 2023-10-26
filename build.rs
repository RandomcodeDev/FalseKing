use shaderc;
use std::{
    env, fs,
    path::{Path, PathBuf},
    process::Command, mem,
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
                Err(err) => println!("cargo:warning={err} ({err:?})"),
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
                Err(err) => println!("cargo:warning={err} ({err:?})"),
            }
        }
    } else if target_os == "windows" && (target_arch == "i586" || target_arch == "i686") {
        println!("cargo:rustc-link-arg-bin=false_king=/SUBSYSTEM:CONSOLE,5.01")
    }

    if target_os != "macos" {
        let compiler = shaderc::Compiler::new().unwrap();
        let mut options = shaderc::CompileOptions::new();

        fn compile_shader(
            compiler: &shaderc::Compiler,
            input_file_name: &str,
            output_file_name: &str,
            shader_kind: shaderc::ShaderKind,
            additional_options: Option<&shaderc::CompileOptions<'_>>,
        ) {
            let source_text = fs::read_to_string(input_file_name).unwrap();
            let result = compiler
                .compile_into_spirv(
                    source_text.as_str(),
                    shader_kind,
                    input_file_name,
                    "main",
                    additional_options,
                )
                .unwrap();
            fs::write(output_file_name, unsafe {
                mem::transmute::<&[u32], &[u8]>(result.as_binary())
            })
            .unwrap();
        }

        for entry in fs::read_dir("assets/shaders").unwrap() {
            let entry = entry.unwrap();
        }
    }

    println!("cargo:rustc-cfg=build={:?}", profile);
}
