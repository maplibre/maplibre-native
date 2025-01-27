fn main() {
    // use src/main.rs to generate a header
    // use cmake crate with flags to compile that header + rust-ffi.cc + core -> staticlib

    // println what to link with
    println!("cargo:rustc-link-lib=static=rust-ffi");


    //
    // cxx_build::bridge("src/main.rs")
    //     .file("src/blobstore.cc")
    //     .std("c++20")
    //     .compile("cxxbridge-demo");
    //
    // println!("cargo:rerun-if-changed=src/main.rs");
    // println!("cargo:rerun-if-changed=src/blobstore.cc");
    // println!("cargo:rerun-if-changed=include/blobstore.h");
}
