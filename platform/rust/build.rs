use std::path::{Path, PathBuf};
use std::{env, fs};

use build_support::parse_deps;
use walkdir::WalkDir;

/// Helper that returns a new cmake::Config with common settings.
/// It selects the renderer based on Cargo features: the user must enable exactly one of:
/// "metal", "opengl", or "vulkan". If none are explicitly enabled, on iOS/macOS the default is metal,
/// and on all other platforms the default is vulkan.
fn create_cmake_config(project_root: &Path) -> cmake::Config {
    let mut cfg = cmake::Config::new(project_root);
    cfg.generator("Ninja");
    cfg.define("CMAKE_C_COMPILER_LAUNCHER", "ccache");
    cfg.define("CMAKE_CXX_COMPILER_LAUNCHER", "ccache");
    cfg.define("MLN_DRAWABLE_RENDERER", "ON");
    cfg.define("MLN_WITH_OPENGL", "OFF");

    let (metal_enabled, opengl_enabled, vulkan_enabled) = {
        let metal = env::var("CARGO_FEATURE_METAL").is_ok();
        let opengl = env::var("CARGO_FEATURE_OPENGL").is_ok();
        let vulkan = env::var("CARGO_FEATURE_VULKAN").is_ok();
        if !metal && !opengl && !vulkan {
            if cfg!(target_os = "ios") || cfg!(target_os = "macos") {
                (true, false, false)
            } else {
                (false, false, true)
            }
        } else {
            (metal, opengl, vulkan)
        }
    };

    let num_enabled = (metal_enabled as u8) + (opengl_enabled as u8) + (vulkan_enabled as u8);
    if num_enabled > 1 {
        panic!("Features 'metal', 'opengl', and 'vulkan' are mutually exclusive. Please enable only one.");
    }

    if opengl_enabled {
        cfg.define("MLN_WITH_OPENGL", "ON");
        cfg.define("MLN_WITH_METAL", "OFF");
        cfg.define("MLN_WITH_VULKAN", "OFF");
    } else if metal_enabled {
        cfg.define("MLN_WITH_OPENGL", "OFF");
        cfg.define("MLN_WITH_METAL", "ON");
        cfg.define("MLN_WITH_VULKAN", "OFF");
    } else if vulkan_enabled {
        cfg.define("MLN_WITH_OPENGL", "OFF");
        cfg.define("MLN_WITH_METAL", "OFF");
        cfg.define("MLN_WITH_VULKAN", "ON");
    }

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

    // Parse the deps file into a list of Cargo instructions.
    let instructions = parse_deps(&deps_contents, &deps_build_dir.join("build"));
    for instr in instructions {
        println!("{}", instr);
    }

    // ------------------------------------------------------------------------
    // 2. Build the actual "mbgl-core" target.
    // ------------------------------------------------------------------------
    let core_build_dir = create_cmake_config(&project_root)
        .build_target("mbgl-core")
        .build();
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
