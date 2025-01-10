# Rust

We have added experimental support for intergrating Rust code into the source tree.

## Rust Bridge

The Rust bridge lives in `rustutils`. To regenerate the C++ bridge run the following script from the root of the repository.

```
rustutils/cpp/generate.sh
```

Check in the generated files under `rustutils/cpp`.

We might intergrate this generation process into the build, but the tools to do so are experimental and immature as of January 2025.

## Building

You need to have the correct Rust toolchain(s) installed. See [Install Rust](https://www.rust-lang.org/tools/install) to install Rust.

You can use `rustup` to manage toolchains. Which toolchain you needs depends on your host platform and for what platform you are trying to build. If your host and target platform are the same, you probably have the correct toolchain installed after installing Rust. For example when building for **Android** and building on a **x84 Linux** host you would use the following command:

```
rustup target add --toolchain stable-x86_64-unknown-linux-gnu aarch64-linux-android armv7-linux-androideabi i686-linux-android x86_64-linux-android
```

See [Platform Support](https://doc.rust-lang.org/nightly/rustc/platform-support.html) in the Rust documentation for more details. You will get a descriptive error message when the correct toolchain is not available, so we don't list all possible combinations here.

### CMake

Set `MLN_USE_RUST=ON` when generating a configuration with CMake.

### Bazel

Pass the `--//:use_rust` flag to Bazel commands.

Note that when [generating an Xcode project](./ios/README.md) you should not pass this option to Bazel directly, but as follows:

```
bazel run //platform/ios:xcodeproj --@rules_xcodeproj//xcodeproj:extra_common_flags="--//:renderer=metal --//:use_rust"
```