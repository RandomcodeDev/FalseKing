use std::env;

fn main() {
    let profile = env::var("PROFILE").unwrap();

    println!("cargo:rustc-cfg=build={:?}", profile);
}
