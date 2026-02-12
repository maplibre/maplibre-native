# WebGPU on Android

Two WebGPU implementations are supported: **Dawn** (Google, C++) and **wgpu-native** (Rust). Both use Vulkan internally on Android. The Gradle product flavors are `webgpuDawn` and `webgpuWgpu`, sharing the `src/webgpu/` Java source set.

## Building with Dawn

Dawn is fetched and built from source via CMake `FetchContent`, so no extra setup is needed. First build is slow (~20 min) because of Dawn's dependencies (abseil, SPIRV-Tools, Tint, etc.).

```bash
cd platform/android
./gradlew :MapLibreAndroidTestApp:installWebgpuDawnDebug
```

## Building with wgpu-native

wgpu-native requires a pre-built static library for the target ABI. The `webgpuWgpu` flavor is currently limited to `arm64-v8a`.

### 1. Cross-compile wgpu-native for Android

Install the Rust Android target:

```bash
rustup target add aarch64-linux-android
```

Point cargo at the NDK linker. Create or edit `~/.cargo/config.toml`:

```toml
[target.aarch64-linux-android]
linker = "<NDK_PATH>/toolchains/llvm/prebuilt/<host>/bin/aarch64-linux-android26-clang"
```

Replace `<NDK_PATH>` with the path to your Android NDK and `<host>` with your platform (e.g. `darwin-x86_64` on macOS).

Then build:

```bash
cd vendor/wgpu-native
cargo build --release --target aarch64-linux-android
```

This produces `vendor/wgpu-native/target/aarch64-linux-android/release/libwgpu_native.a`, which the CMake build picks up automatically.

### 2. Build and install

```bash
cd platform/android
./gradlew :MapLibreAndroidTestApp:installWebgpuWgpuDebug
```

## Debugging wgpu-native

wgpu-native logs are routed to Android logcat under the `wgpu-native` tag (warnings and errors by default). Rust panic messages appear under `wgpu-native-stderr`. Filter with:

```bash
adb logcat -s wgpu-native:W wgpu-native-stderr:E
```

To increase verbosity, change `WGPULogLevel_Warn` to `WGPULogLevel_Info` (or `Debug` / `Trace`) in `android_webgpu_renderer_backend.cpp`.

## Notes

The native backend is in `MapLibreAndroid/src/cpp/android_webgpu_renderer_backend.cpp`. It uses `#if MLN_WEBGPU_IMPL_DAWN` / `#elif MLN_WEBGPU_IMPL_WGPU` for the API differences, same pattern as the desktop backends.

wgpu-native's adapter request forces `backendType = Vulkan` — without this it may pick GL on some emulators, which can't present to an ANativeWindow surface.
