# Rendering Backends

MapLibre Native for iOS supports multiple rendering backends. The backend is selected at build time through CMake presets.

## Available backends

| Preset | Backend | Status | Notes |
|---|---|---|---|
| `ios-metal` | Metal | Stable | Default, recommended for production |
| `ios-webgpu-dawn` | WebGPU (Dawn) | Experimental | Uses Metal internally, see [WebGPU](./ios-webgpu.md) |
| `ios-webgpu-wgpu` | WebGPU (wgpu-native) | Experimental | Uses Metal internally, see [WebGPU](./ios-webgpu.md) |

## Building with a specific backend

Each preset generates an Xcode project in its own build directory:

```bash
# Metal (default)
cmake --preset ios-metal -DDEVELOPMENT_TEAM_ID=YOUR_TEAM_ID
xed build-ios/MapLibre\ Native.xcodeproj

# WebGPU (Dawn)
cmake --preset ios-webgpu-dawn -DDEVELOPMENT_TEAM_ID=YOUR_TEAM_ID
xed build-ios-webgpu-dawn/MapLibre\ Native.xcodeproj

# WebGPU (wgpu-native)
cmake --preset ios-webgpu-wgpu -DDEVELOPMENT_TEAM_ID=YOUR_TEAM_ID
xed build-ios-webgpu-wgpu/MapLibre\ Native.xcodeproj
```

Then build the `app` target in Xcode, or from the command line:

```bash
cmake --build build-ios-webgpu-wgpu -- -sdk iphonesimulator -arch arm64
```

## How it works

Each preset passes different CMake variables that control which native rendering code gets compiled:

- `ios-metal`: `-DMLN_WITH_METAL=ON`
- `ios-webgpu-dawn`: `-DMLN_WITH_WEBGPU=ON -DMLN_WEBGPU_IMPL_DAWN=ON`
- `ios-webgpu-wgpu`: `-DMLN_WITH_WEBGPU=ON -DMLN_WEBGPU_IMPL_WGPU=ON`

Only one backend is active per build. The Objective-C layer is shared across backends, with renderer-specific code selected at compile time via `#if MLN_WEBGPU_IMPL_DAWN` / `#elif MLN_WEBGPU_IMPL_WGPU` conditionals in `MLNMapView+WebGPU.mm`.
