use std::collections::HashSet;
use std::env;
use std::fs;
use std::path::{Path, PathBuf};
use walkdir::WalkDir;

// Helper that returns a new cmake::Config with common settings.
fn create_cmake_config(project_root: &Path) -> cmake::Config {
    let mut cfg = cmake::Config::new(project_root);
    cfg.generator("Ninja");
    cfg.define("CMAKE_C_COMPILER_LAUNCHER", "ccache");
    cfg.define("CMAKE_CXX_COMPILER_LAUNCHER", "ccache");
    cfg.define("MLN_DRAWABLE_RENDERER", "ON");
    cfg.define("MLN_WITH_OPENGL", "OFF");
    cfg.define("MLN_WITH_METAL", "ON");
    cfg.define("MLN_WITH_VULKAN", "OFF");
    cfg.define("MLN_WITH_WERROR", "OFF");
    cfg
}

fn main() {
    // Determine the project root (use the parent of CARGO_MANIFEST_DIR's parent to allow in‐source Windows builds).
    let project_root = {
        let manifest = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
        manifest.parent().unwrap().parent().unwrap().to_path_buf()
    };

    // ------------------------------------------------------------------------
    // 1. Build the "mbgl-core-deps" target first so that mbgl-core-deps.txt is generated.
    // Since CMake installs targets into a "build" subdirectory, we look for the file there.
    // ------------------------------------------------------------------------
    let deps_build_dir = create_cmake_config(&project_root)
        .build_target("mbgl-core-deps")
        .build();
    let deps_file = deps_build_dir.join("build").join("mbgl-core-deps.txt");
    let deps_contents = fs::read_to_string(&deps_file)
        .unwrap_or_else(|_| panic!("Failed to read {}", deps_file.display()));

    // Parse linker flags from the deps file.
    let tokens: Vec<&str> = deps_contents.split_whitespace().collect();
    let mut token_iter = tokens.iter().peekable();
    let mut added_search_paths = HashSet::new();

    while let Some(&token) = token_iter.next() {
        if token == "-framework" {
            if let Some(&framework) = token_iter.next() {
                println!("cargo:rustc-link-lib=framework={}", framework);
            } else {
                panic!("Expected a framework name after '-framework'");
            }
        } else if token.starts_with("-l") {
            let libname = &token[2..];
            println!("cargo:rustc-link-lib={}", libname);
        } else if token.ends_with(".a") {
            let lib_path = Path::new(token);
            let file_stem = lib_path.file_stem().expect("Library file has no stem");
            let file_stem = file_stem.to_str().expect("Library file stem is not UTF-8");
            let lib_name = file_stem.strip_prefix("lib").unwrap_or(file_stem);

            // The .a libraries are located relative to the build_dir's "build" subdirectory.
            let static_lib_base = deps_build_dir.join("build");
            let search_dir = match lib_path.parent() {
                Some(parent) if !parent.as_os_str().is_empty() => static_lib_base.join(parent),
                _ => static_lib_base.clone(),
            };
            if added_search_paths.insert(search_dir.clone()) {
                println!("cargo:rustc-link-search=native={}", search_dir.display());
            }
            println!("cargo:rustc-link-lib=static={}", lib_name);
        } else {
            // Pass any other token directly to the linker.
            println!("cargo:rustc-link-arg={}", token);
        }
    }

    // ------------------------------------------------------------------------
    // 2. Build the actual "mbgl-core" target.
    // ------------------------------------------------------------------------
    let core_build_dir = create_cmake_config(&project_root)
        .build_target("mbgl-core")
        .build();

    // Static libraries are placed in the "build" subdirectory.
    let static_lib_base = core_build_dir.join("build");
    println!(
        "cargo:rustc-link-search=native={}",
        static_lib_base.display()
    );

    // ------------------------------------------------------------------------
    // 3. Gather include directories and build the C++ bridge using cxx_build.
    // ------------------------------------------------------------------------
    let mut include_dirs = vec![
        project_root.join("include"),
        project_root.join("platform/default/include"),
    ];
    for entry in WalkDir::new(project_root.join("vendor")) {
        let entry = entry.expect("Failed reading vendor directory");
        if entry.file_type().is_dir() && !entry.path_is_symlink() && entry.file_name() == "include"
        {
            include_dirs.push(entry.path().to_path_buf());
        }
    }

    cxx_build::bridge("src/lib.rs")
        .includes(include_dirs)
        .file("src/wrapper.cpp")
        .flag_if_supported("-std=c++20")
        .compile("maplibre_rust_bindings");

    // ------------------------------------------------------------------------
    // 4. Instruct Cargo when to re-run the build script.
    // ------------------------------------------------------------------------
    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=src/wrapper.cpp");
    println!("cargo:rerun-if-changed=include/tile_server_options.h");
}
