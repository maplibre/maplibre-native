use std::path::{Path, PathBuf};
use std::{env, fs};

use build_support::parse_deps;
use walkdir::WalkDir;

trait CfgBool {
    fn define_bool(&mut self, key: &str, value: bool);
}

impl CfgBool for cmake::Config {
    fn define_bool(&mut self, key: &str, value: bool) {
        self.define(key, if value { "ON" } else { "OFF" });
    }
}

/// Helper that returns a new cmake::Config with common settings.
/// It selects the renderer based on Cargo features: the user must enable exactly one of:
/// "metal", "opengl", or "vulkan". If none are explicitly enabled, on iOS/macOS the default is metal,
/// and on all other platforms the default is vulkan.
fn create_cmake_config(project_root: &Path) -> cmake::Config {
    let mut cfg = cmake::Config::new(project_root);
    cfg.generator("Ninja");
    cfg.define("CMAKE_C_COMPILER_LAUNCHER", "ccache");
    cfg.define("CMAKE_CXX_COMPILER_LAUNCHER", "ccache");
    cfg.define_bool("MLN_DRAWABLE_RENDERER", true);
    cfg.define_bool("MLN_WITH_OPENGL", false);

    let with_opengl = env::var("CARGO_FEATURE_OPENGL").is_ok();
    let mut with_metal = env::var("CARGO_FEATURE_METAL").is_ok();
    let mut with_vulkan = env::var("CARGO_FEATURE_VULKAN").is_ok();

    if !with_opengl && !with_metal && !with_vulkan {
        if cfg!(any(target_os = "ios", target_os = "macos")) {
            with_metal = true;
        } else {
            with_vulkan = true;
        }
    }

    if ((with_metal as u8) + (with_opengl as u8) + (with_vulkan as u8)) > 1 {
        panic!("Features 'metal', 'opengl', and 'vulkan' are mutually exclusive. Please enable only one.");
    }

    cfg.define_bool("MLN_WITH_OPENGL", with_opengl);
    cfg.define_bool("MLN_WITH_METAL", with_metal);
    cfg.define_bool("MLN_WITH_VULKAN", with_vulkan);
    cfg.define_bool("MLN_WITH_WERROR", false);
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
    for instr in parse_deps(&deps_contents, &deps_build_dir.join("build"), true) {
        println!("{instr}");
    }

    // ------------------------------------------------------------------------
    // 2. Build the actual "mbgl-core" static library target.
    // ------------------------------------------------------------------------
    let core_build_dir = create_cmake_config(&project_root)
        .build_target("mbgl-core")
        .build()
        .join("build");
    let static_lib_base = core_build_dir.to_str().unwrap();
    println!("cargo:rustc-link-search=native={static_lib_base}",);

    // ------------------------------------------------------------------------
    // 3. Gather include directories and build the C++ bridge using cxx_build.
    // ------------------------------------------------------------------------
    // TODO: This is a temporary solution. We should get this list from CMake as well.
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

    // Link mbgl-core after the bridge - or else `cargo test` won't be able to find the symbols.
    println!("cargo:rustc-link-lib=static=mbgl-core");

    // Link to SQLite in order for `cargo test` to work
    // TODO: Ensure cmake always uses statically linking to sqlite3
    // Note that this works, but we do not want to depend on dynamic bindings in Rust
    println!("cargo:rustc-link-lib=dylib=sqlite3");
    // Note that linker automatically gets this passed from core, but that seems to not be enough to solve the issues bellow
    // println!("cargo:rustc-link-lib=static=mbgl-vendor-sqlite");
    //
    // FIXME: This is a hack to link the SQLite library on Linux.
    //        This does NOT work for `cargo run`, but works for `cargo test`.
    //
    // cargo run linking errors:
    // platform/rust/target/debug/build/maplibre-native-d736790a198d5b5a/out/build/libmbgl-core.a(sqlite3.cpp.o): in function `mapbox::sqlite::initialize()':
    // platform/default/src/mbgl/storage/sqlite3.cpp:117:(.text._ZN6mapbox6sqliteL10initializeEv+0x1f): undefined reference to `sqlite3_libversion_number'
    // platform/default/src/mbgl/storage/sqlite3.cpp:119:(.text._ZN6mapbox6sqliteL10initializeEv+0x44): undefined reference to `sqlite3_libversion_number'
    // platform/default/src/mbgl/storage/sqlite3.cpp:130:(.text._ZN6mapbox6sqliteL10initializeEv+0xd5): undefined reference to `sqlite3_config'
    //
    // println!("cargo:rustc-link-search=native=/usr/lib/x86_64-linux-gnu");
    // println!("cargo:rustc-link-lib=static=sqlite3");
    // ------------------------------------------------------------------------
    // 4. Instruct Cargo when to re-run the build script.
    // ------------------------------------------------------------------------

    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=src/wrapper.cpp");
    println!("cargo:rerun-if-changed=include/tile_server_options.h");
    println!("cargo:rerun-if-changed=include/map_renderer.h");
}
