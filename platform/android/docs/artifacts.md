# Artifacts

MapLibre Android is published to Maven Central as several artifacts. They differ in their rendering backend and in whether they export the experimental [plugin API](plugin-api.md):

| Artifact | Rendering backend | Plugin API |
|---|---|---|
| `org.maplibre.gl:android-sdk` | Vulkan (the current default) | No |
| `org.maplibre.gl:android-sdk-opengl` | OpenGL ES | No |
| `org.maplibre.gl:android-sdk-vulkan` | Vulkan | No |
| `org.maplibre.gl:android-sdk-vulkan-opengl` | Both, selected at runtime | No |
| `org.maplibre.gl:android-sdk-with-plugin-api` | Vulkan (the current default) | Yes |
| `org.maplibre.gl:android-sdk-opengl-with-plugin-api` | OpenGL ES | Yes |
| `org.maplibre.gl:android-sdk-vulkan-with-plugin-api` | Vulkan | Yes |
| `org.maplibre.gl:android-sdk-vulkan-opengl-with-plugin-api` | Both, selected at runtime | Yes |

Each artifact is also published as a debug build, with a `-debug` postfix, for example `org.maplibre.gl:android-sdk-vulkan-debug` or `org.maplibre.gl:android-sdk-opengl-with-plugin-api-debug`.

Most apps should use the default artifact, `org.maplibre.gl:android-sdk`. Replace `<version>` with the [latest MapLibre Android version](https://github.com/maplibre/maplibre-native/releases?q=android&expanded=true):

```gradle
dependencies {
    implementation 'org.maplibre.gl:android-sdk:<version>'
}
```

## Rendering backends

### OpenGL ES vs. Vulkan

| | OpenGL ES | Vulkan |
|---|---|---|
| Device compatibility | Any device | Vulkan 1.0 hardware, Android 7.0 (API 24)+ |
| Performance | Baseline | Better on modern devices |
| Maturity | Longest track record | Newer, some open issues remain |

To use Vulkan with OpenGL ES as a fallback, use `org.maplibre.gl:android-sdk-vulkan-opengl`. It contains both backends and picks one at runtime instead of at build time. Because it contains two native libraries, it is about 10 MB larger than an artifact with a single backend.

### Selecting a backend

Pass a `RenderingEngine.Type` to `MapLibre.getInstance()`:

```java
// Explicit selection
MapLibre.getInstance(context, apiKey, tileServer, RenderingEngine.Type.VULKAN);

// Auto-detect
MapLibre.getInstance(context, apiKey, tileServer, null);

// Auto-detect
MapLibre.getInstance(context, apiKey, tileServer);

// Auto-detect
MapLibre.getInstance(context);
```

Every `MapLibre.getInstance()` overload without a `RenderingEngine.Type` detects the backend automatically. Auto-detection selects Vulkan when the device runs Android 7.0 (API 24) or later and reports Vulkan 1.0 hardware support through the `PackageManager.FEATURE_VULKAN_HARDWARE_VERSION` feature. Everything else gets OpenGL ES.

`RenderingEngine` is part of every artifact. With an artifact that has a single backend, requesting the other backend makes `MapLibre.getInstance()` throw an `UnsupportedOperationException`.

### Getting the active backend

```java
RenderingEngine.Type active = RenderingEngine.getCurrentType();
```

`RenderingEngine.getCurrentType()` reports the active backend once `MapLibre.getInstance()` has selected it.

The first `MapLibre.getInstance()` call loads the native library of the selected backend. The backend then stays fixed for the lifetime of the process: later `MapLibre.getInstance()` calls ignore their `RenderingEngine.Type`, so switching to another backend requires restarting the process.

## See Also

- [MapLibre Android API Documentation](https://maplibre.org/maplibre-native/android/api/)
- [Build Flavors (developer docs)](https://maplibre.org/maplibre-native/docs/book/platforms/android/android-build-flavors.html)
