# Rendering Backends

The rendering backend is selected at build time through Gradle product flavors.

## Build flavors

| Flavor | Backend | Plugin API | Status | Maven Central artifact | Notes |
|---|---|---|---|---|---|
| `opengl` | OpenGL ES | No | Stable | `org.maplibre.gl:android-sdk-opengl` | Widest device compatibility |
| `vulkan` | Vulkan | No | Stable | `org.maplibre.gl:android-sdk-vulkan`, `org.maplibre.gl:android-sdk` | Default, better performance on modern devices |
| `multiBackend` | OpenGL ES + Vulkan | No | Experimental | `org.maplibre.gl:android-sdk-vulkan-opengl` | Both backends in one AAR, chosen at runtime |
| `openglWithPluginApi` | OpenGL ES | Yes | Experimental | `org.maplibre.gl:android-sdk-opengl-with-plugin-api` | |
| `vulkanWithPluginApi` | Vulkan | Yes | Experimental | `org.maplibre.gl:android-sdk-vulkan-with-plugin-api`, `org.maplibre.gl:android-sdk-with-plugin-api` | |
| `multiBackendWithPluginApi` | OpenGL ES + Vulkan | Yes | Experimental | `org.maplibre.gl:android-sdk-vulkan-opengl-with-plugin-api` | |
| `webgpuDawn` | WebGPU (Dawn) | No | Experimental | not published | Uses Vulkan internally, see [WebGPU](./android-webgpu.md) |
| `webgpuWgpu` | WebGPU (wgpu-native) | No | Experimental | not published | Uses Vulkan internally, arm64-v8a only, see [WebGPU](./android-webgpu.md) |

Every published flavor is published as a debug build too, with a `-debug` postfix (e.g. `android-sdk-vulkan-debug` or `android-sdk-opengl-with-plugin-api-debug`).

The `*WithPluginApi` flavors are built like the renderer flavor they are named after, and additionally export the experimental C plugin API ([`include/mln/plugin/plugin_api.h`](https://github.com/maplibre/maplibre-native/blob/main/include/mln/plugin/plugin_api.h)) from the native library. The plugin API does not support WebGPU. The [n-gon layer demo app](https://github.com/maplibre/maplibre-native/tree/main/plugins/android) in `plugins/android` builds with these flavors.

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

The same pattern applies to the library module (`MapLibreAndroid`), which also has the `*WithPluginApi` flavors:

```bash
./gradlew :MapLibreAndroid:assembleVulkanRelease
./gradlew :MapLibreAndroid:assembleVulkanWithPluginApiRelease
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
- `*WithPluginApi`: the arguments of the renderer flavor, and `-DMLN_WITH_PLUGINS=ON`

For `opengl`, `vulkan`, and the WebGPU flavors, only one backend is compiled in and it's fixed for the lifetime of the app. `multiBackend` is the exception - it ships both the OpenGL ES and Vulkan native libraries and picks one at runtime.

Renderer-specific Java code is split between flavor-specific source sets (`src/opengl/`, `src/vulkan/`, `src/multiBackend/`) and shared per-backend code in `src/sharedRenderer/opengl/` and `src/sharedRenderer/vulkan/`. The `*WithPluginApi` flavors use the source sets of their renderer flavor.
