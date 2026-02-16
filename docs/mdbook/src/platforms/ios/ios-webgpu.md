# WebGPU on iOS

The iOS platform supports WebGPU as a rendering backend alongside Metal. There are two WebGPU implementations: **Dawn** (Google) and **wgpu-native** (Rust). Both use Metal as the GPU backend on iOS.

## Building with Dawn

Dawn is fetched and built from source via CMake `FetchContent`, so no extra setup is needed. The first build takes a while because Dawn pulls in abseil, SPIRV-Tools, Tint, etc. Subsequent builds are incremental.

```bash
cmake --preset ios-webgpu-dawn
cmake --build build-ios-webgpu-dawn -- -sdk iphonesimulator -arch arm64
```

## Building with wgpu-native

wgpu-native requires a pre-built static library for the target architecture. The CMake build will attempt to build it from source if cargo is available, but cross-compilation for iOS needs some setup.

### 1. Install the Rust iOS targets

```bash
# For simulator (Apple Silicon Mac)
rustup target add aarch64-apple-ios-sim

# For device
rustup target add aarch64-apple-ios
```

### 2. Cross-compile wgpu-native

For the iOS Simulator:

```bash
cd vendor/wgpu-native
BINDGEN_EXTRA_CLANG_ARGS="--target=arm64-apple-ios -isysroot $(xcrun --sdk iphoneos --show-sdk-path)" \
  cargo build --release --target aarch64-apple-ios-sim \
  --no-default-features --features metal,wgsl,spirv,glsl
```

For a physical device:

```bash
cd vendor/wgpu-native
BINDGEN_EXTRA_CLANG_ARGS="--target=arm64-apple-ios -isysroot $(xcrun --sdk iphoneos --show-sdk-path)" \
  cargo build --release --target aarch64-apple-ios \
  --no-default-features --features metal,wgsl,spirv,glsl
```

The `BINDGEN_EXTRA_CLANG_ARGS` environment variable is required so that bindgen's macro fallback can find iOS system headers when generating FFI bindings.

This produces `vendor/wgpu-native/target/<target>/release/libwgpu_native.a`, which the CMake build picks up automatically.

### 3. Configure and build

```bash
cmake --preset ios-webgpu-wgpu
cmake --build build-ios-webgpu-wgpu -- -sdk iphonesimulator -arch arm64
```

If the pre-built library is not found and cargo is available, CMake will attempt to build wgpu-native automatically during configuration.

## Implementation notes

The native backend lives in `platform/ios/src/MLNMapView+WebGPU.mm`. It uses `#if MLN_WEBGPU_IMPL_DAWN` / `#elif MLN_WEBGPU_IMPL_WGPU` conditionals for the API differences between the two implementations, following the same pattern as the Android and desktop backends.

Both backends use a plain `UIView` with a `CAMetalLayer` (not `MTKView`) for the WebGPU surface. The rendering view has `userInteractionEnabled = NO` so touch events pass through to the parent `MLNMapView`.
