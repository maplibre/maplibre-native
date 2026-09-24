# Vulkan fill extrusion plugin

The built-in `fill-extrusion` layer remains the default. Link `mln-fill-extrusion`
and register it before loading styles to opt into the plugin:

```cpp
#include <fill_extrusion.hpp>
char error[512]{};
auto status = mln_fill_extrusion_register(mln_plugin_register_v1, error, sizeof(error));
// Accept OK or ALREADY_REGISTERED; report error otherwise.
```

Registration is process-wide. Style JSON keeps its ordinary `fill-extrusion`
type and properties. Existing built-in layer objects keep their implementation
identity, including their renderer and layout dispatch. Another plugin cannot
claim an already registered replacement.

The plugin owns ring classification, holes, Earcut roof triangulation, walls,
16-bit index segmentation, lighting shaders, and footprint queries. The host
owns tiles, GPU buffers, property expressions and transitions, feature-state
bindings, light evaluation, translation matrices, and pass orchestration.

Supported paint properties are color (black), base (0), height (0), opacity (1),
vertical-gradient (true), translate ([0, 0]), and translate-anchor (map).
Base, height, and color support feature, composite, and feature-state
expressions. The remaining properties support camera expressions. Numeric,
color, and translation values transition; boolean and anchor values are discrete.
Data-driven base and height use the built-in shader's nonnegative clamping.
Color and light calculations match the built-in Vulkan shaders, including their
alpha semantics. Patterns and rounded corners are not implemented in the solid
milestone and return explicit unsupported-property errors.

## API changes

The unstable v1 descriptors change in place. `replace_builtin` explicitly
permits a built-in name replacement; it never permits replacing another plugin.
`is_3d` controls layer metadata, ordering, matrices, and drawable dimensionality.
`draw_passes` declares ordered depth-test/write, color-write, premultiplied-blend,
culling, and layer-stencil-deduplication settings. Passes run across all tiles
in array order. A zero count retains the original single 2D default pass.

`evaluate_layer` receives evaluated camera properties before render orchestration.
Its output enables passes and specifies host-applied translation/anchor.
A failed callback, nonfinite translation, or invalid pass mask suppresses that
evaluation. Geometry does not depend on opacity. Opaque extrusions enable one
color pass; translucent ones enable depth-only then deduplicated color passes.
Zero opacity disables rendering. `enable_near_clipped_matrix` selects the
unaligned near-clipped projection. Uniform contexts include evaluated light
color, intensity, and bearing-adjusted Cartesian direction.

Boolean properties use `MLN_PLUGIN_VALUE_BOOLEAN` (0 or 1) and the
`MLN_PLUGIN_PROPERTY_ENCODING_BOOLEAN_FLOAT` shader binding. They parse,
serialize, evaluate expressions, and validate independently of numeric values.
All metadata is copied at registration, callback memory remains borrowed, and
all callbacks must return normally. The n-gon example uses an explicit pass
matching its original rendering settings.

## Build and checks

Use the project's native dependencies, or the local uncommitted `flake.nix`.

```sh
cmake -S . -B build-plugin-vulkan -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DMLN_WITH_PLUGINS=ON \
  -DMLN_WITH_VULKAN=ON -DMLN_WITH_OPENGL=OFF \
  -DMLN_WITH_GLFW=OFF -DMLN_WITH_X11=OFF -DMLN_WITH_WAYLAND=OFF
cmake --build build-plugin-vulkan --target mbgl-render-test-runner \
  mln-fill-extrusion-render-tests mln-plugin-api-tests mln-plugin-render-tests mln-ngon-tests
build-plugin-vulkan/plugins/mln-plugin-api-tests
build-plugin-vulkan/plugins/mln-ngon-tests
build-plugin-vulkan/mbgl-render-test-runner --manifestPath plugins/fill-extrusion/render-tests/manifest.json
build-plugin-vulkan/plugins/mln-fill-extrusion-render-tests --manifestPath plugins/fill-extrusion/render-tests/manifest.json
build-plugin-vulkan/plugins/mln-plugin-render-tests --manifestPath plugins/ngon-layer/render-tests/manifest.json
```

Run the two render executables separately with the same Vulkan driver. The
manifest uses the existing Linux expectations, ignores, and `metrics/cache-style.db`.
Copy the first HTML report before the second run if comparing reports. The
selection file records every solid render/query candidate and deferred pattern
fixture. No expectations, tolerances, or ignores were changed.

Bazel provides `//plugins/fill-extrusion` and
`//plugins/fill-extrusion:geometry-tests`. The current repository's Bazel core
has no Vulkan backend configuration; use CMake for Vulkan render tests.
Earcut is vendored from the repository's existing copy with its ISC license.
