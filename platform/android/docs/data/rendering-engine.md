# Rendering Engine (OpenGL ES / Vulkan)

MapLibre Android is published as several artifacts on Maven Central, each built with a different rendering backend:

| Artifact | Backend |
|---|---|
| `org.maplibre.gl:android-sdk` | Vulkan (the current default) |
| `org.maplibre.gl:android-sdk-opengl` | OpenGL ES |
| `org.maplibre.gl:android-sdk-vulkan` | Vulkan |
| `org.maplibre.gl:android-sdk-vulkan-opengl` | Both, selected at runtime |

Most apps should use the default configuration: `org.maplibre.gl:android-sdk`

If you'd like to use Vulkan but still want to have the OpenGL fallback option, then use should use `org.maplibre.gl:android-sdk-vulkan-opengl` artifact. It packs both backends into a single AAR and picks between them at runtime rather than at build time.

PS: Since `org.maplibre.gl:android-sdk-vulkan-opengl` artifact carries multiple native libraries, it's larger than a single-backend build by the size of the second library which is ~10 MB.

## Adding the dependency

```gradle
dependencies {
    implementation 'org.maplibre.gl:android-sdk-vulkan-opengl:<version>'
}
```

Replace `<version>` with the [latest MapLibre Android version](https://github.com/maplibre/maplibre-native/releases?q=android&expanded=true).

## Selecting a backend

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

Auto-detection is enabled by default. Every `MapLibre.getInstance()` overload that does not take a `RenderingEngine.Type` automatically detects backend.

Auto-detection selects Vulkan when the device runs Android 7.0 (API 24) or later and reports Vulkan 1.0 hardware support through `PackageManager.FEATURE_VULKAN_HARDWARE_VERSION` feature. Everything else gets OpenGL ES.

PS: `RenderingEngine` ships in every artifact. If you try to set unsupported `RenderingEngine` backend in single-backend build, then `MapLibre.getInstance()` throws `UnsupportedOperationException` exception.

## Get active backend

```java
// Active backend
RenderingEngine.Type active = RenderingEngine.getCurrentType();
```

`RenderingEngine.getCurrentType()` reports the active backend once `MapLibre.getInstance()` has resolved it.

The first `MapLibre.getInstance()` call loads the native library for the selected backend, then the backend is fixed for the lifetime of the process. Any subsequent `MapLibre.getInstance()` calls with a different `RenderingEngine.Type` will be ignored. The process restart is required to apply a different rendering backend.

## OpenGL ES vs. Vulkan

| | OpenGL ES | Vulkan |
|---|---|---|
| Device compatibility | Any device | Vulkan 1.0 hardware, Android 7.0 (API 24)+ |
| Performance | Baseline | Better on modern devices |
| Maturity | Longest track record | Newer, some open issues remain |

## See Also

- [MapLibre Android API Documentation](https://maplibre.org/maplibre-native/android/api/)
- [Rendering Backends (developer docs)](https://maplibre.org/maplibre-native/docs/book/platforms/android/android-rendering-backends.html)
