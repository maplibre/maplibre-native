# Rendering Backends

The rendering backend is selected at build time through Gradle product flavors.

## Available backends

| Flavor | Backend | Status | Maven Central artifact | Notes |
|---|---|---|---|---|
| `opengl` | OpenGL ES | Stable | `org.maplibre.gl:android-sdk-opengl` | Default, widest device compatibility |
| `vulkan` | Vulkan | Stable | `org.maplibre.gl:android-sdk-vulkan` | Better performance on modern devices |
| `webgpuDawn` | WebGPU (Dawn) | Experimental | not published | Uses Vulkan internally, see [WebGPU](./android-webgpu.md) |
| `webgpuWgpu` | WebGPU (wgpu-native) | Experimental | not published | Uses Vulkan internally, arm64-v8a only, see [WebGPU](./android-webgpu.md) |
| `multiBackend` | OpenGL ES + Vulkan | Experimental | `org.maplibre.gl:android-sdk-vulkan-opengl` | Both backends in one AAR, chosen at runtime |

`org.maplibre.gl:android-sdk` is also published and currently maps to the `vulkan` flavor's release build. `-debug` suffixed artifact (e.g. `android-sdk-vulkan-debug`) is published for each flavor as well.

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

# Both OpenGL ES and Vulkan, backend chosen at runtime
./gradlew :MapLibreAndroidTestApp:installMultiBackendDebug
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
- `multiBackend`: `-DMLN_ANDROID_MULTI_BACKEND=ON`

For `opengl`, `vulkan`, and the WebGPU flavors, only one backend is compiled in and it's fixed for the lifetime of the app. `multiBackend` is the exception - it ships both the OpenGL ES and Vulkan native libraries and picks one at runtime.

Renderer-specific Java code is split between flavor-specific source sets (`src/opengl/`, `src/vulkan/`, `src/multiBackend/`) and shared per-backend code in `src/sharedRenderer/opengl/` and `src/sharedRenderer/vulkan/`.
