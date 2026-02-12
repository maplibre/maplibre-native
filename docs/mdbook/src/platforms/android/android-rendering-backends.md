# Rendering Backends

The rendering backend is selected at build time through Gradle product flavors.

## Available backends

| Flavor | Backend | Status | Notes |
|---|---|---|---|
| `opengl` | OpenGL ES | Stable | Default, widest device compatibility |
| `vulkan` | Vulkan | Stable | Better performance on modern devices |
| `webgpuDawn` | WebGPU (Dawn) | Experimental | Uses Vulkan internally, see [WebGPU](./android-webgpu.md) |
| `webgpuWgpu` | WebGPU (wgpu-native) | Experimental | Uses Vulkan internally, arm64-v8a only, see [WebGPU](./android-webgpu.md) |

## Building with a specific backend

The flavor name is part of the Gradle task. For the test app:

```bash
cd platform/android

# OpenGL ES
./gradlew :MapLibreAndroidTestApp:installOpenglDebug

# Vulkan
./gradlew :MapLibreAndroidTestApp:installVulkanDebug

# WebGPU (Dawn)
./gradlew :MapLibreAndroidTestApp:installWebgpuDawnDebug

# WebGPU (wgpu-native)
./gradlew :MapLibreAndroidTestApp:installWebgpuWgpuDebug
```

The same pattern applies to the library module (`MapLibreAndroid`):

```bash
./gradlew :MapLibreAndroid:assembleVulkanRelease
```

## Selecting in Android Studio

In Android Studio, use the **Build Variants** panel (View > Tool Windows > Build Variants) to pick the renderer flavor. Each module will show a dropdown combining the flavor and build type, e.g. `vulkanDebug` or `openglRelease`.

## How it works

Each flavor sets different CMake arguments:

- `opengl`: `-DMLN_WITH_OPENGL=ON`
- `vulkan`: `-DMLN_WITH_VULKAN=ON`
- `webgpuDawn`: `-DMLN_WITH_WEBGPU=ON`, `-DMLN_WEBGPU_IMPL_DAWN=ON`
- `webgpuWgpu`: `-DMLN_WITH_WEBGPU=ON`, `-DMLN_WEBGPU_IMPL_WGPU=ON`

Only one backend is active per build. Renderer-specific Java code lives in `src/opengl/`, `src/vulkan/`, or `src/webgpu/` source sets.
