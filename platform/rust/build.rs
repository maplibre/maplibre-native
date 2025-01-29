use std::env;
use std::path::PathBuf;

//noinspection RsConstantConditionIf
fn main() {
    let project_root = {
        let manifest = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
        let root = manifest.join("maplibre-native-src");
        if root.is_symlink() || root.is_dir() {
            // Use the symlinked directory to allow packaging
            root
        } else {
            // Modeled from the kuzu project
            // If the path is not directory, this is probably an in-source build on windows where the symlink is unreadable.
            manifest.parent().unwrap().parent().unwrap().to_path_buf()
        }
    };

    let build_dir = cmake::Config::new(&project_root)
        .generator("Ninja")
        .define("CMAKE_C_COMPILER_LAUNCHER", "ccache")
        .define("CMAKE_CXX_COMPILER_LAUNCHER", "ccache")
        .define("MLN_WITH_CORE_ONLY", "ON")
        .build_target("mbgl-core")
        .build();

    let lib_dir = build_dir.join("build");
    println!("cargo:rustc-link-search=native={}", lib_dir.display());
    if rustversion::cfg!(since(1.82)) {
        // `cargo test` wouldn't work with Rust v1.82+ without this
        // FIXME: this MIGHT significantly increase the size of the final binary, needs to be tested
        println!("cargo:rustc-link-lib=static:+whole-archive=mbgl-core");
    } else {
        println!("cargo:rustc-link-lib=static=mbgl-core");
    }

    // cxx build
    let mut cxx = cxx_build::bridge("src/lib.rs");
    cxx.include(project_root.join("include"))
        .flag_if_supported("-std=c++20")
        .compile("maplibre_rust_bindings");
}
