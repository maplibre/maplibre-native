# Remove the C++ Custom Drawable Layer

## Summary

Remove `CustomDrawableLayer` as a hard breaking cleanup, with no deprecation aliases or compatibility implementation. The C plugin API becomes the preferred host-owned drawable extension mechanism, while its current feature gaps are documented explicitly.

This does not remove the backend-direct `CustomLayer` API or the unrelated `CustomGeometrySource`.

## Implementation changes

- Delete `CustomDrawableLayer`, `CustomDrawableLayerHost`, their factory, immutable state, render layer, and the programmatic `custom-drawable` layer type.
- Remove the Objective-C `MLNCustomDrawableStyleLayer` wrapper, peer factory, umbrella-header export, examples, menu entries, and UI test.
- Remove the GLFW custom-drawable example and its custom-only assets.
- Remove custom-drawable API tests and fixtures.
- Delete the custom geometry and symbol-icon shaders, UBOs, shader IDs, backend registrations, and hash-test entries that have no consumer after the layer is gone.
- Clean CMake and Bazel source lists and the obsolete `MLN_LAYER_CUSTOM_DRAWABLE_*` conditionals.
- Retain generic drawable machinery used by raw `CustomLayer`, including the drawable `isCustom` path and `DrawableCustomLayerHostTweaker`.

## Plugin API limitations

Document the capabilities that the C plugin API does not replace yet and the use cases each future capability would enable:

- source-less plugin layers for transient application overlays and debug geometry;
- per-layer instances and dynamic drawable updates for simulations, moving objects, and streaming data;
- uploaded textures on ordinary plugin drawables for icons, decals, and textured meshes;
- host primitive generators for line tessellation, polygon triangulation, and screen-aligned symbols;
- stable per-drawable and per-frame uniform identity for independent object animation;
- generic programmatic platform construction of registered plugin layers.

Document built-in style layers with GeoJSON or `CustomGeometrySource`, source-bound C plugins, existing-layer extensions, and raw backend-specific `CustomLayer` as the current alternatives.

## Validation

- Ensure live code contains no references to the removed layer, Objective-C wrapper, or custom-only shaders.
- Build OpenGL, Metal, Vulkan, and WebGPU core targets.
- Run drawable, shader, layer-manager, raw `CustomLayer`, and plugin registry tests.
- Build the GLFW example and iOS SDK/sample after removing their custom-drawable UI.
- Run the external shared plugin render-test runner against the modified core.

## Decisions

- Loss of source-less programmatic custom drawables, custom textures, and per-frame drawable mutation is accepted.
- No compatibility shim or replacement plugin is added in this change.
- `CustomLayer` and `CustomGeometrySource` remain supported.
- Reserved plugin source kinds remain reserved and are documented rather than enabled.
