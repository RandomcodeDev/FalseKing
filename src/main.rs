mod platform;

use platform::PlatformBackend;

fn main() {
    let mut backend = platform::get_backend_for_platform();

    while backend.update() {

    }

    backend.shutdown();
}
