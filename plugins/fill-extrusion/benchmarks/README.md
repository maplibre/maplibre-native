# Vulkan extrusion performance

The plugin reaches render-time parity on the measured software Vulkan driver:
its steady frame times are 45–52% lower than the built-in layer's, and initial
loading is 6–15% faster. Buffer-memory parity is not reached. The built-in uses
instanced walls; the plugin deliberately retains ordinary indexed triangles.

## Reproduce

```sh
cmake --build build-plugin-vulkan --target mln-fill-extrusion-benchmark
# Select the same Vulkan driver for both processes, e.g. the Nix shell's lavapipe.
LP_NUM_THREADS=4 python3 plugins/fill-extrusion/benchmarks/run.py \
  build-plugin-vulkan/plugins/mln-fill-extrusion-benchmark \
  --side 100 --frames 40 --trials 3 --output /tmp/extrusion-benchmark.json
```

`builtin` constructs `FillExtrusionLayer` directly without registering a plugin.
`plugin` registers the replacement, parses the same layer type, and checks its
implementation identity. Each trial runs separate processes, alternating their
order. Neither process reads network resources.

Measurements below were taken on 2026-09-24 with Release/Clang 21.1.8,
Nixpkgs `a32edd7654519351e48e80372a928df336394670`, Mesa 26.2.3 lavapipe
(LLVM 21.1.8), four rasterizer threads, and an AMD Ryzen AI Max+ 395 CPU.
These are software-driver results, not claims about physical GPU performance.

Each scene contains 10,000 GeoJSON squares, a 512×512 viewport, zoom 14.5,
pitch 55°, and bearing 25°. The scenarios use constant paint, data-driven
height/color, half opacity, rounded corners, or a sprite pattern. Loading time
includes style parsing, tile production, plugin/native layout, initial buffer
uploads, pipeline creation, and the first rendered image; frontend construction
and generation of the input JSON are excluded. The ordinary driver shader cache
is retained. This is initial map loading, not a cold driver-cache measurement.

After ten warmup frames, each phase records 40 frames. Steady frames change
nothing; paint frames alternate opacity 0.5/1; state frames change height state
for 16 features. Wall times include synchronous image readback. Renderer CPU
encoding/rendering times are also recorded, without GPU timestamp queries.
Tables show the median of three trials' frame p50 values (milliseconds).
The raw captures include p95 values, draw calls, and active buffer-byte counters.

## Results after optimization

| Scene | Built-in steady | Plugin steady | Built-in load | Plugin load |
| --- | ---: | ---: | ---: | ---: |
| Solid | 28.91 | 15.02 | 262.16 | 232.71 |
| Data-driven | 26.98 | 13.17 | 255.69 | 240.97 |
| Translucent | 44.94 | 21.74 | 252.38 | 214.39 |
| Rounded | 101.33 | 49.98 | 330.11 | 287.73 |
| Pattern | 55.93 | 30.58 | 261.81 | 233.87 |

Feature-state frames measure 27.39 ms built-in versus 13.52 ms plugin.
Plugin state-frame CPU encoding dropped from 0.444 to 0.223 ms.
The built-in and plugin use the same draw-call count for the four non-rounded
scenarios. Rounded steady draw calls fell from 49 to 37 in the plugin, compared
with 13 built-in, as fewer vertices require fewer 16-bit segments.

## Measured costs and changes

The initial plugin used 28-byte vertices and emitted a separate roof vertex for
every triangle corner. Compact 12-byte records now pack integer/fractional
positions, top/bottom flags, edge distances, and normals. Roof triangles reuse
vertices within each 16-bit segment. Positions retain the original 1/128-unit
precision; normals use signed 14-bit fixed-point precision. Segmentation remains
safe even when one feature crosses a segment boundary.

Feature-state changes also dirtied every data-driven paint buffer, including
expressions that could not depend on state. The host now caches that dependency
and skips those writes while retaining state for later expression changes.
A focused regression checks both the unchanged buffer and the later transition
to a state-dependent expression.

Active vertex-buffer sizes, decimal MB, including driver buffer copies:

| Scene | Built-in | Plugin before | Plugin after | Plugin reduction |
| --- | ---: | ---: | ---: | ---: |
| Solid/translucent/pattern | 0.90 | 13.86 | 5.40 | 61% |
| Data-driven | 3.60 | 33.66 | 23.40 | 30% |
| Rounded | 3.78 | 84.42 | 27.00 | 68% |

The data-driven and rounded steady phases improved from 17.13 to 13.17 ms and
54.85 to 49.98 ms respectively. Solid and pattern steady times rose slightly
(14.62→15.02 ms and 29.92→30.58 ms), a small tradeoff for their buffer reduction.
These small timing differences should not be generalized beyond this driver.

Remaining costs are vertex/paint duplication for walls and index storage.
Plugin index buffers use 1.35 MB for ordinary scenes and 7.83 MB for rounded
scenes, versus 0.27 and 2.43 MB built-in. Plugin vertex buffers remain 6–7.1 times
larger. Conversely, its per-draw uniforms are much smaller than the built-in
instancing uniforms. Reaching memory parity would require a further design
change such as a generic instancing API or compressed paint attributes; neither
is introduced by these optimizations. Physical GPU profiling remains a separate
validation step before claiming performance parity across hardware.

[Before captures](before.json) use implementation `498ff04759e5` and the harness
committed in `2129f49cfa67`. [After captures](after.json) use compact indexed
geometry and state-dependent uploads with that same harness.

## Correctness retained

All 45 focused tests pass, including ten rounded and 56 patterned direct
comparisons, holes, and 16-bit segmentation. All 50 eligible solid renders,
four queries, and 13 n-gon renders pass. Seven of eight pattern expectations
pass; the existing `tile-buffer` failure still produces a byte-identical image
to the built-in baseline. Expectations, tolerances, and ignores are unchanged.
The plugins-disabled Vulkan runner builds, and the public API header compiles
as C11. The benchmark runner script has also been exercised independently.
