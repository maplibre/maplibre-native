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

You need to have the correct Rust toolchain(s) installed.

### CMake

Set `MLN_USE_RUST` to `ON`.

### Bazel

Pass `--//:use_rust` to Bazel.