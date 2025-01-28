use std::env;
use std::path::PathBuf;

fn main() {
    let manifest_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let project_root = PathBuf::from(manifest_dir)
        .parent().unwrap() // "platform/"
        .parent().unwrap() // "maplibre-native/"
        .to_path_buf();

    let mut cmake_cfg = cmake::Config::new(&project_root);
    cmake_cfg
        .define("MLN_WITH_CORE_ONLY", "ON")
        .generator("Ninja")
        .build_target("mbgl-core");

    let build_output = cmake_cfg.build();

    // The library is in build_output/build/libmbgl-core.a
    let lib_dir = build_output.join("build");
    println!("cargo:rustc-link-search=native={}", lib_dir.display());
    println!("cargo:rustc-link-lib=static=mbgl-core");

    // If on macOS/Clang:
    println!("cargo:rustc-link-lib=c++");
    // If on Linux/GCC:
    // println!("cargo:rustc-link-lib=stdc++");

    let mut cxx = cxx_build::bridge("src/lib.rs");
    cxx.include(project_root.join("include"))
        .flag_if_supported("-std=c++20")
        .compile("maplibre_rust_bindings");
}
