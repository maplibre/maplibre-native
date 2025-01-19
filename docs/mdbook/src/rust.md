# Rust

We have added experimental support for intergrating Rust code into the source tree.

## Rust Bridge

The Rust bridge lives in the root `rustutils` directory.

We use [CXX](https://cxx.rs/) to allow interop between Rust and C++.

## Building

### CMake

When building with CMake, need to have the correct Rust toolchain(s) installed. See [Install Rust](https://www.rust-lang.org/tools/install) to install Rust.

You can use `rustup` to manage toolchains. Which toolchain you needs depends on your host platform and for what platform you are trying to build. If your host and target platform are the same, you probably have the correct toolchain installed after installing Rust. For example when building for **Android** and building on a **x84 Linux** host you would use the following command:

```
rustup target add --toolchain stable-x86_64-unknown-linux-gnu aarch64-linux-android armv7-linux-androideabi i686-linux-android x86_64-linux-android
```

See [Platform Support](https://doc.rust-lang.org/nightly/rustc/platform-support.html) in the Rust documentation for more details. You will get a descriptive error message when the correct toolchain is not available, so we don't list all possible combinations here.

You also need to have cxxbridge installed:

```
cargo install cxxbridge-cmd
```

Set `-DMLN_USE_RUST=ON` when generating a configuration with CMake.

### Bazel

Pass the `--//:use_rust` flag to Bazel commands.

Note that when [generating an Xcode project](./ios/README.md) you should not pass this option to Bazel directly, but as follows:

```
bazel run //platform/ios:xcodeproj --@rules_xcodeproj//xcodeproj:extra_common_flags="--//:renderer=metal --//:use_rust"
```

## Creating a new Module

To create a new module:

1. Add a new source file to `rustutils/src/example.rs`.
2. Implement it, see the [CXX documentation](https://cxx.rs/index.html) or see `rustutils/src/color.rs` for an example.
3. Create a C++ source file that will use the generated C++ header. See `src/mbgl/util/color.rs.cpp` for an example. Import the generated header with
    ```
    #include <rustutils/example.hpp>
    ```
4. Conditionally include either the `*.rs.cpp` file or the `*.cpp` file it replaces in CMake and Bazel. Here is what it looks like for CMake:
    ```
    ${PROJECT_SOURCE_DIR}/src/mbgl/util/color$<IF:$<BOOL:${MLN_USE_RUST}>,.rs.cpp,.cpp>
    ```
    And here for Bazel:
    ```
    select({
      "//:rust": [
          "src/mbgl/util/color.rs.cpp",
      ],
      "//conditions:default": [
          "src/mbgl/util/color.cpp",
      ],
    })
    ```