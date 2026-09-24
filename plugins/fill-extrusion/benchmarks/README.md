# Vulkan extrusion performance

The plugin now uses instanced walls. On the measured software Vulkan driver,
steady frames take 6–12% less time than the built-in layer. Instancing reduces
plugin vertex-buffer use by 77–86% versus its previous indexed implementation,
and brings geometry/index storage and draw counts close to the built-in.
The tradeoff is substantial: instanced frames take 63–96% longer than the
previous indexed plugin on lavapipe. Data-driven paint still uses more vertex
buffer space than the built-in's packed attributes.

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
order. Neither process reads network resources. Builds and correctness tests
were idle during the measurements.

Measurements were taken on 2026-09-24 with Release/Clang 21.1.8,
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
The instancing capture also records `buffer_bytes`, the allocator's total active
buffer bytes. Resource-role counters can alias: the native instancing path
wraps shared vertex allocations in uniform/storage-buffer views, so adding
`vertex_bytes`, `index_bytes`, and `uniform_bytes` does not measure total memory.
`buffer_bytes` includes alignment and allocations pending frame-safe reclamation;
it excludes textures and CPU-side geometry.

## Fresh built-in versus instanced plugin

| Scene | Built-in steady | Plugin steady | Built-in load | Plugin load |
| --- | ---: | ---: | ---: | ---: |
| Solid | 29.27 | 27.11 | 246.02 | 247.31 |
| Data-driven | 26.97 | 24.79 | 253.07 | 250.44 |
| Translucent | 45.30 | 42.67 | 245.57 | 247.52 |
| Rounded | 102.80 | 96.38 | 331.64 | 318.65 |
| Pattern | 56.51 | 49.96 | 252.75 | 255.75 |

Feature-state frames measure 27.72 ms built-in versus 25.04 ms plugin;
CPU encoding is 0.497 versus 0.224 ms. Initial load results are within 4% of
the built-in across all scenes. Paint-update frames are faster through the
plugin except for solid (31.24 versus 29.19 ms, 7% slower).

## Instancing changes and tradeoffs

Walls generate four corners from `gl_VertexIndex` for each instance. Two
attributes read adjacent records from one packed eight-byte outline stream.
Ring terminators suppress connecting walls; roof triangles index the same
records. Oversized outlines retain instanced walls while roof copies are
remapped into safe 16-bit segments. Matching roof/wall feature mappings share
host paint binders, buffers, and state updates.

The Vulkan binding path now includes record offset, stride, and input rate
when identifying shared bindings. This fixes adjacent-record attributes
previously reading the same position and producing invisible walls.

Active vertex-buffer sizes, decimal MB:

| Scene | Built-in | Indexed plugin | Instanced plugin | Reduction |
| --- | ---: | ---: | ---: | ---: |
| Solid/translucent/pattern | 0.90 | 5.40 | 0.90 | 83% |
| Data-driven | 3.60 | 23.40 | 5.40 | 77% |
| Rounded | 3.78 | 27.00 | 3.78 | 86% |

Steady total buffer allocation, decimal MB:

| Scene | Built-in | Instanced plugin |
| --- | ---: | ---: |
| Solid | 1.172 | 1.172 |
| Data-driven | 3.872 | 5.672 |
| Translucent/pattern | 1.173 | 1.174 |
| Rounded | 6.212 | 6.212 |

Constant-paint buffer allocations are within 0.11% of the built-in. Data-driven
steady allocations remain 47% higher. In the state-update phase they are
6.57 MB built-in versus 5.90 MB plugin: the native path replaces both color and
height buffers, while the plugin skips state-independent color uploads.

Index buffers fall from 1.35 to 0.27 MB for ordinary scenes and from 7.83 to
2.43 MB for rounded scenes, matching the built-in within 36 bytes. Steady draw
calls match exactly: nine for solid/data, 17 for translucent/pattern, and 13
for rounded. The indexed plugin needed 37 draws for rounded geometry.

| Scene | Previous indexed steady | Instanced steady | Change |
| --- | ---: | ---: | ---: |
| Solid | 15.02 | 27.11 | +81% |
| Data-driven | 13.17 | 24.79 | +88% |
| Translucent | 21.74 | 42.67 | +96% |
| Rounded | 49.98 | 96.38 | +93% |
| Pattern | 30.58 | 49.96 | +63% |

The previous measurements used the same scene/harness and environment; they
were taken before this change, rather than rerunning the old implementation.
Fresh built-in steady times remain within roughly 2% of those earlier trials.
The larger instancing slowdown is in synchronized rendering/readback, while
plugin steady CPU encoding remains below 0.2 ms. The measurements locate the
cost in the render path but do not isolate shader execution from lavapipe's
instance processing. Profiling a physical Vulkan GPU is needed before selecting
an approach for GPU throughput based on these CPU-driver results.

The remaining data-driven vertex-memory gap comes from generic float color
samples (32 bytes per record) versus the native packed color samples (16 bytes).
Height adds eight bytes in both paths. Sharing paint buffers removes the former
roof/wall duplication; packing color would be a separate encoding change.
State-independent paint uploads continue to be skipped, preserving the earlier
feature-state optimization.

[Initial captures](before.json) use implementation `498ff04759e5` and the harness
committed in `2129f49cfa67`. [Indexed captures](after.json) use compact 12-byte
geometry and state-dependent uploads from `2bf7cc57a27f`.
[Instanced captures](instanced.json) use the new instancing implementation and
the same timing loop with the additional allocation-memory counter.

## Correctness retained

All 49 focused tests pass, including ten rounded and 56 patterned direct
comparisons, holes, shifted instance bounds, paint sharing, and outlines beyond
the 16-bit index limit. Built-in and plugin each pass all 50 eligible solid
renders and four queries. All 13 n-gon renders and the n-gon unit executable
pass. Seven of eight pattern expectations pass; the existing `tile-buffer`
failure still produces a byte-identical image to the built-in baseline.
Expectations, tolerances, and ignores are unchanged.
The plugins-disabled Vulkan runner builds, the public API header compiles as
C11, and the direct host comparisons pass with the Khronos validation layer
enabled and no validation errors reported.
