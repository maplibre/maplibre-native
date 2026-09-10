# Native plugin examples

This opt-in foundation supports source-bound geometry layers with host-owned
layout buckets, drawables, shader programs, and dynamic paint-property binders.
Plugins register through the pure-C header in `include/mln/plugin/plugin_api.h`
before constructing a renderer or loading a dependent style. Descriptor metadata
is copied; callback code must remain loaded for the process lifetime. There is
no unregistration.

Layout callbacks run on tile workers (possibly concurrently for different tiles).
Uniform callbacks run on the render thread. Callback inputs are borrowed;
geometry returned by `finish_layout` is copied before the callback result is
released. Callbacks must not throw across the C boundary. The host owns GPU
resource lifetimes and handles paint updates, feature state, and zoom interpolation.

The initial API intentionally omits built-in-layer extensions, raster sources,
file loading, textures, offscreen passes, and platform packaging.
Drawables use indexed triangles, read-only depth, premultiplied alpha, and the
translucent pass. Layout is responsible for point ownership; marks are not
clipped to tile boundaries.

## Build and test

Plugins are **off by default**. Enable them explicitly:

```sh
bazel test --//:plugins=true --//:renderer=metal \
  //plugins:host-tests //plugins/ngon-layer:unit-tests
bazel run --//:plugins=true --//:renderer=metal //plugins:render-tests -- \
  --manifestPath "$PWD/plugins/ngon-layer/render-tests/manifest.json"
```

On Linux, use `--//:renderer=drawable` for OpenGL. Use Metal on macOS: the
existing macOS OpenGL headless backend does not support the renderer's GLES 3
shader contract. The plugin also supplies Vulkan shaders
for CMake builds with `MLN_WITH_VULKAN=ON`.
If Homebrew ICU is not on your linker search path, add
`--linkopt=-L/opt/homebrew/opt/icu4c/lib` to the Bazel commands.

For CMake, add `-DMLN_WITH_PLUGINS=ON` to the usual desktop configuration,
then build `mln-ngon-tests`, `mln-plugin-host-tests`, and
`mln-plugin-render-tests`. Run the render-test executable with the same
`--manifestPath` argument shown above. No Android or iOS package is introduced.

The new host sources and registration symbol are excluded when plugins are off.
The older experimental C++ `PluginLayer` already on the parent branch is
unchanged; its existing Apple bindings are outside this PR.

## GLFW

Only the Bazel GLFW target links and registers the example plugin:

```sh
bazel run --//:plugins=true --//:renderer=metal //platform/glfw:glfw_app -- \
  --style "$PWD/plugins/ngon-layer/render-tests/ngon/corners-and-rotation/style.json" \
  --lon 0 --lat 0 --zoom 10
```

This offline example uses feature-driven corner count, color, and rotation.
Any style using registered `ngon` layers can be supplied through `--style`.
