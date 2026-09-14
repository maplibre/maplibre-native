# Metal circle plugin benchmark — 2026-09-14

## Conclusion

Enabling plugin support alone showed no consistent warm-rendering penalty in
this experiment. The circle plugin is visually compatible with the supported
fixtures, but **is not performance-equivalent to the built-in implementation**.
Ordinary 10k-point cases add roughly 5–6 microseconds of CPU encoding per frame.
Dense overdraw, multiple layers, startup/reload time and especially process
memory expose larger costs that should be addressed before calling the port
production-ready.

These measurements compare complete implementations, not the isolated cost of
a C function call. The plugin's shaders, uniform organization and generic
property binders differ from the native implementation.

## Configuration and validation

- Apple M3 Max CPU/GPU; macOS 26.6.2; AppleClang Release builds, LTO disabled.
- Matched plugins-off/native, plugins-on/native and plugins-on/plugin processes.
  The plugin is a separately loaded dylib with no MapLibre C++ dependency.
- 1024×768, pixel ratio 1, four background workers, deterministic local GeoJSON.
- Nine workloads, seven fresh process repeats per variant: **189 process runs**.
  Thirty warmups and 300 measured requests per normal phase; ten style reloads.
  Variant order rotates and reverses between repeats.
- **58/58 original supported circle render fixtures pass** on Metal, without
  changing expected images or tolerances. Only `circle-sort-key` is excluded.
  The circle geometry tests and all **37 plugin/query regressions** also pass.
- No Android/iOS SDK changes or Linux/other-renderer performance claims.

The finalized run is `benchmark/results/circle-metal-pooled/`. Its `metadata.json`
records the machine and executable/dylib SHA-256 hashes; individual JSONL files
contain raw measurements, and `summary.json` contains process means and intervals.
Raw generated results remain local and are not checked into Git. The harness is
committed in `67c7c53ed112`, building on core/plugin commits `fddaa2667d40` and
`b47ff67ba75f`. See [reproduction instructions](circle-metal.md).

## Warm rendering

Here “native” means plugins enabled but **not registered**. Wall time includes
Metal headless submission and GPU completion, without image readback. CPU
encoding is the separate host counter, not a GPU timestamp. Times are arithmetic
means of the seven process means.

| Workload | Native wall ms | Plugin wall ms | Plugin/native wall ratio, 95% CI | Native CPU µs | Plugin CPU µs |
|---|---:|---:|---:|---:|---:|
| 1k constant | 0.383 | 0.411 | 1.074 [1.064, 1.084] | 19.07 | 24.28 |
| 10k constant | 0.417 | 0.484 | 1.161 [1.134, 1.187] | 19.76 | 24.89 |
| 100k constant | 0.901 | 1.371 | 1.541 [1.382, 1.699] | 33.76 | 39.27 |
| 10k camera expression | 0.417 | 0.488 | 1.169 [1.142, 1.195] | 20.81 | 26.23 |
| 10k feature expression | 0.423 | 0.485 | 1.147 [1.114, 1.181] | 20.67 | 26.81 |
| 10k composite expression | 0.448 | 0.521 | 1.164 [1.121, 1.208] | 22.11 | 26.61 |
| 10k feature-state expression, unchanged | 0.437 | 0.499 | 1.140 [1.104, 1.176] | 20.37 | 25.91 |
| 10k densely overlapping | 3.149 | 5.448 | 1.730 [1.704, 1.756] | 36.85 | 40.32 |
| Eight layers, 10k shared features | 0.814 | 1.234 | 1.517 [1.498, 1.536] | 42.20 | 67.92 |

Ratios and their Student-t intervals use paired process means, so they need not
equal the ratio of the rounded overall means. Frames are not treated as
independent statistical samples.

For plugins-on/native versus plugins-off/native, all nine warm CPU-encoding
ratio intervals include 1.0. This does not prove zero overhead; it means this
experiment did not resolve a consistent penalty from merely enabling support.
For example, the 10k constant CPU ratio is 1.003 [0.960, 1.046].

## Lifecycle and updates

| Workload/operation | Native ms | Plugin ms | Plugin/native ratio, 95% CI |
|---|---:|---:|---:|
| 100k first frame with readback | 181.03 | 357.47 | 1.975 [1.936, 2.014] |
| 100k full style reload | 134.54 | 428.59 | 3.186 [3.140, 3.233] |
| 10k composite, zoom update | 0.437 | 0.520 | 1.189 [1.155, 1.223] |
| 10k state, change 1% of features | 0.611 | 0.943 | 1.551 [1.447, 1.655] |
| 10k state, change all features | 11.392 | 9.996 | 0.878 [0.858, 0.897] |

Startup includes parsing, source processing, layout, binding, shader/pipeline
setup and rendering. Reload includes rebuilding the style/source; neither is an
isolated worker-layout timer or an incremental GeoJSON-update benchmark. State
measurements include the state API calls and subsequent rendering. The full-state
case being faster does not cancel out the sparse-update regression.

## Memory and submission counters

Mean process peak RSS across repeats, in MiB (not live plugin allocations):

| Workload | Native peak RSS | Plugin peak RSS | Native/plugin warm draw calls |
|---|---:|---:|---:|
| 10k feature expression | 120.9 | 346.1 | 4 / 5 |
| 100k constant | 765.1 | 2619.6 | 16 / 17 |
| Eight layers, 10k shared features | 174.1 | 2246.6 | 32 / 33 |

GPU buffer memory is nearly equal: for the 10k feature case, 1,286,868 versus
1,287,664 bytes. Uniform uploads differ: 496 versus 1,008 bytes per warm frame;
the eight-layer case uploads 3,632 versus 7,728 bytes. Thus GPU buffer allocation
alone cannot explain the RSS difference. Peak RSS includes temporary allocations,
compiler work and allocator high-water marks; it is not evidence of a leak.

The harness uses both the existing internal Metal autorelease-pool option and
outer frame pools matching an application event loop. An initial unpooled run
stalled the native baseline by exhausting command buffers. Outer pooling did
not eliminate the plugin's high-memory result. Only the finalized pooled run
is used in the tables above.

## Prioritized follow-up

1. **Share immutable feature snapshots per bucket.**
   [Each property binder currently clones feature properties and geometry](../../src/mln/renderer/buckets/plugin_bucket.cpp),
   before checking whether its value is data-driven. Eleven circle properties
   and multiple layers multiply the same snapshots. This is confirmed duplication
   and a strong candidate for the startup/RSS costs, not an allocation-profile
   attribution of every measured byte. Preserve runtime constant-to-expression
   updates without retaining separate snapshots in every binder.
2. **Match native Metal shader specialization.** The
   [plugin generator](../../plugins/circle-layer/scripts/generate-shaders.mjs)
   always passes float paint/color varyings; the
   [native shader](../../include/mln/shaders/mtl/circle.hpp) uses conditional
   varyings, half-precision values and fragment-stage uniform reads. Dense
   overdraw has a much larger wall-time gap than CPU-encoding gap. Investigate
   shader bandwidth/arithmetic before attributing that gap to the C API.
3. **Remove unnecessary stencil work and cache uniforms.** The
   [generic host](../../src/mln/renderer/layers/render_plugin_style_layer.cpp)
   supplies stencil tiles despite disabling stencil on these drawables. It
   records one additional draw call. The
   [plugin tweaker](../../src/mln/renderer/layers/plugin_layer_tweaker.cpp)
   also allocates/rebuilds each uniform block per drawable each frame, unlike
   native circle's separately cached layer properties.
4. **Profile sparse-state updates and isolated layout/binding.** Add worker and
   allocation profiling after fixing snapshot duplication; rerun this same
   benchmark and unchanged render fixtures for each optimization.

Results are specific to this desktop Metal implementation and machine. Driver
caches were not purged; GPU elapsed timestamps and dedicated allocation traces
were not collected. Do not extrapolate these figures to Android, iOS, OpenGL or
Vulkan, or present them as an intrinsic overhead of a C ABI.
