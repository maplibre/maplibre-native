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
        .generator("Ninja")
        .define("CMAKE_C_COMPILER_LAUNCHER", "ccache")
        .define("CMAKE_CXX_COMPILER_LAUNCHER", "ccache")
        .define("MLN_WITH_CORE_ONLY", "ON")
        .build_target("mbgl-core");

    let build_output = cmake_cfg.build();

    let lib_dir = build_output.join("build");
    println!("cargo:rustc-link-search=native={}", lib_dir.display());
    println!("cargo:rustc-link-lib=static=mbgl-core");

    // cxx build
    let mut cxx = cxx_build::bridge("src/lib.rs");
    cxx.include(project_root.join("include"))
       .flag_if_supported("-std=c++20")
       .compile("maplibre_rust_bindings");
}
