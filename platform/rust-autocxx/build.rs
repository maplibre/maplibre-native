use std::path::PathBuf;

fn main() -> miette::Result<()> {
    autocxx_build::Builder::new(
        "src/main.rs",
        [
            &PathBuf::from("src"),
            &PathBuf::from("../../include"),
            &PathBuf::from("../../vendor/mapbox-base/include"),
            &PathBuf::from("../../vendor/mapbox-base/deps/geometry.hpp/include"),
            &PathBuf::from("../../vendor/mapbox-base/deps/variant/include"),
            &PathBuf::from("../../vendor/boost/include"),
            &PathBuf::from("../../vendor/boost/include/boost"),
            &PathBuf::from("../../platform/default/include"),
        ],
    )
    .extra_clang_args(&["-std=c++17"])
    .build()?
    .flag_if_supported("-std=c++17")
    .compile("rustlib");

    println!("cargo:rerun-if-changed=src/main.rs");

    Ok(())
}
