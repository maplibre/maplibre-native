# Rendering Engine API

The `multiBackend` flavor packs both the OpenGL ES and Vulkan native libraries into a single AAR and selects between them at runtime. Other flavors define a single backend at compile time in Gradle (see [Rendering Backends](./android-rendering-backends.md))

## OpenGL ES vs. Vulkan

| | OpenGL ES | Vulkan |
|---|---|---|
| Device compatibility | Any device (widest support) | Requires Vulkan 1.0 hardware (`FEATURE_VULKAN_HARDWARE_VERSION`) and API 24+ |
| Performance | Baseline | Better on modern devices |
| Stability | Battle-tested, longest track record | Newer, may have edge cases |

The `multiBackend` flavor can detect whether the device supports Vulkan and fall back to OpenGL ES if it does not. Accordingly, `multiBackend` manifest declares the Vulkan feature as optional, therefore making it available in Play Store listings on devices without Vulkan support.

## Selecting a backend

```java
// Explicit selection
MapLibre.getInstance(context, apiKey, tileServer, RenderingEngine.Type.VULKAN);

// Auto-detect: Vulkan if the device supports it, OpenGL ES otherwise
MapLibre.getInstance(context, apiKey, tileServer, null);
```

- Once `MapLibre.getInstance()` has been called for the first time in the process, the backend is locked in. Passing a different `type` afterward has no effect; it does not replace the backend that is already loaded.
- Single-backend flavors are stricter: passing a `type` other than the one compiled in throws `UnsupportedOperationException`.
- `RenderingEngine.getCurrentType()` returns the backend currently in effect. It should return the selected backend after `MapLibre.getInstance()` runs and resolves the actual backend.
