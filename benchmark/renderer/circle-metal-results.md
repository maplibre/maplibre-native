# Metal circle plugin benchmark — 2026-09-14

## Latest update: dense Metal and retained-feature follow-up

The [follow-up report](circle-followup-results.md) records an isolated shader
experiment and another 112-process comparison. Matching native Metal buffer
address spaces reduces isolated dense plugin time **3.839 → 3.196 ms**, with
native at **3.194 ms**. The full comparison also reduces 100k reloads
**172.28 → 157.97 ms** and plugin peak RSS **972.0 → 884.0 MiB**. Sparse bounds
scan less data, but sparse-update wall-time improvement remains inconclusive.
All **48 plugin/query tests, 58 circle fixtures and 13 ngon fixtures** pass.
See that report for committed evidence, confidence intervals and limitations.

## Previous update: submission-path fixes

The [submission investigation and follow-up report](circle-submission-investigation.md#full-benchmark-rerun-after-the-fixes)
now includes another complete **189-process** comparison at `659f20016033`,
with [committed per-process results](../results/metal-circle/submission-fixed/).
The original findings were committed before implementation; paint snapshot
sharing, retained shader groups, generic uniform scopes/batching, the circle
uniform split, and actual Metal submission counters are separate commits.

The eight-layer case now submits **90 inline-byte calls / 6,752 bytes** per
warm frame, down from **130 / 15,712**. Native submits **90 / 4,960**. Thus
inline bytes fall **57.0%** and the excess over native falls **83.3%**. The
original investigation found no duplicate rendering or warm geometry/pipeline
churn; the fixes target verified map-copy/lookup and uniform-organization costs.

Eight-layer plugin CPU encoding falls from **65.44 to 47.88 µs** (26.8% lower
mean), but native moves from **40.72 to 52.01 µs**. The new paired CPU ratio
is **0.958, 95% CI [0.691, 1.225]**: desktop timing variation is substantial,
and this does **not establish parity or a precise per-fix speedup**. No outliers
were excluded. Dense wall time remains **3.754 ms plugin / 3.113 ms native**,
about **20.6% slower**, with a narrow interval. Large-source memory/reloads and
sparse state updates retain gaps; the detailed report records them too.

Validation: **45 plugin/query tests, 58 circle fixtures, 13 ngon fixtures**, and
circle descriptor/geometry tests pass on Metal. Expected images and tolerances
are unchanged. Release builds with plugins on/off pass. This round adds explicit
uniform scopes to the unpublished v1 C descriptor, requiring plugins to rebuild;
there are no Darwin SDK changes. OpenGL/Vulkan are not validated by this run.

**Metric correction:** references below to 48 unchanged-frame uniform-upload
bytes mean backing-storage updates, not total bytes submitted to Metal. The
new counters separately measure actual inline encoder submissions.

## First optimization round (historical): five improvements

**The improvements helped substantially, but the plugin is not yet
performance-equivalent to native circle.** The largest gains are memory and
layout/reload costs. Dense rendering improved but retains a measurable gap;
multi-layer CPU encoding remains expensive. Sparse-state wall-time improvement
is inconclusive in this experiment.

- 100k constant-circle plugin peak RSS: **2,619.6 → 983.1 MiB** (62.5% lower).
- Eight-layer plugin peak RSS: **2,246.6 → 149.7 MiB** (93.3% lower).
- 100k style reload: **428.59 → 170.03 ms** (60.3% lower).
- Eight-layer style reload: **261.10 → 25.67 ms** (90.2% lower).
- Dense plugin warm wall time: **5.448 → 3.759 ms** (31.0% lower), still
  **19.5% slower than matched native**, 95% CI [18.1%, 20.8%].
- Warm plugin draw counts now match native. Unchanged-paint uniform uploads
  fall from 1,008 to 48 bytes for one layer and from 7,728 to 48 for eight layers.

The percentages above compare complete before/after runs, not isolated timings
for each commit. Native controls and process-repeat intervals below help separate
implementation improvements from machine variation. These are desktop Metal
results, not a claim about intrinsic C ABI overhead or mobile performance.

### Evidence and validation

The [committed datasets](../results/metal-circle/) include the original baseline,
the final optimized comparison, five incremental smoke runs, opt-in worker
profiling and a successful Instruments allocation export. Each full comparison
contains **189 fresh process runs**, seven repeats per workload/variant, 30
warmups and 300 measured requests per normal phase. Ten reloads are measured.
Per-process aggregates, sample counts, percentiles, confidence intervals,
machine configuration and binary hashes are committed; large per-frame logs
and the full Instruments trace remain local.

The final uninstrumented run is `circle-metal-optimized-final`, exported as
[`optimized`](../results/metal-circle/optimized/), source revision
`73c3b279e6bd`. It was run from scratch after Instruments finished; the interrupted
sweep is excluded. The baseline is the original pooled run, not an unpooled or
partially completed run. No builds, render tests or Instruments recordings ran
concurrently with the final sweep. OS/driver caches were not purged, and this
was not an isolated laboratory machine. Some intervals are consequently wide.

Validation after all changes: **58/58 supported original circle render fixtures**,
**42/42 plugin/query tests**, and the standalone circle descriptor, geometry,
segmentation, map-mode and query tests pass. Expected images and tolerances are
unchanged. Metal Release builds with plugins enabled and disabled both pass.
There are no Darwin SDK changes or new public C ABI requirements for these
optimizations. No OpenGL/Vulkan timing claim is made.

### Warm rendering: before and after

“Native” below is the final plugins-enabled, unregistered control. Wall time
includes Metal headless completion, without readback. Encoding is a separate
host CPU counter; it is **not** GPU time. The ratio intervals are paired
Student-t intervals across seven process means within the final run. An
interval including 1 does not establish equivalence.

| Workload | Plugin before ms | Plugin after ms | Native after ms | Final plugin/native wall ratio, 95% CI | Final CPU µs, plugin / native |
|---|---:|---:|---:|---:|---:|
| 1k constant | 0.411 | 0.387 | 0.406 | 0.963 [0.872, 1.054] | 22.42 / 20.24 |
| 10k constant | 0.484 | 0.449 | 0.410 | 1.098 [0.890, 1.306] | 23.88 / 19.70 |
| 100k constant | 1.371 | 0.840 | 0.870 | 0.972 [0.886, 1.057] | 32.17 / 30.11 |
| 10k camera | 0.488 | 0.413 | 0.410 | 1.006 [0.980, 1.032] | 23.41 / 20.24 |
| 10k feature | 0.485 | 0.421 | 0.415 | 1.015 [0.993, 1.037] | 23.22 / 19.91 |
| 10k composite | 0.521 | 0.481 | 0.449 | 1.072 [0.940, 1.203] | 24.54 / 20.77 |
| 10k state, unchanged | 0.498 | 0.431 | 0.431 | 1.002 [0.978, 1.027] | 23.65 / 20.44 |
| 10k dense | 5.448 | 3.759 | 3.147 | 1.195 [1.181, 1.208] | 41.93 / 39.73 |
| Eight layers, 10k features | 1.234 | 0.877 | 0.855 | 1.028 [0.960, 1.096] | 65.44 / 40.72 |

Single-layer CPU overhead is now approximately 2–4 µs. Eight layers still add
24.72 µs: uniform-upload elimination did **not** eliminate CPU callback/property
preparation costs. Its plugin encoding mean improved only from 67.92 to 65.44 µs
between runs, insufficient evidence of a substantial CPU speedup.

Merely enabling plugin support still shows no consistent warm CPU penalty:
eight of nine enabled-native/disabled-native intervals include 1; the remaining
feature case is slightly faster, 0.974 [0.952, 0.997]. These comparisons are not
adjusted for multiple testing and should not be interpreted as proof of a gain.

### Lifecycle, updates and memory

| Operation | Plugin before ms | Plugin after ms | Native after ms | Final plugin/native ratio, 95% CI |
|---|---:|---:|---:|---:|
| 100k startup/readback | 357.47 | 196.54 | 182.38 | 1.078 [1.051, 1.104] |
| 100k style reload | 428.59 | 170.03 | 137.40 | 1.238 [1.213, 1.262] |
| Eight-layer style reload | 261.10 | 25.67 | 34.47 | 0.745 [0.709, 0.781] |
| 10k state, change 1% | 0.943 | 0.864 | 0.912 | 0.975 [0.774, 1.177] |
| 10k state, change 100% | 9.996 | 9.748 | 11.333 | 0.860 [0.849, 0.871] |

Sparse-state native time also moved from 0.611 to 0.912 ms between runs. The
plugin's lower final mean does not establish either parity or an end-to-end
speedup from indexing alone. The full-state plugin remains faster in this
workload, as it was before the optimizations.

| Workload | Plugin peak RSS before MiB | Plugin after MiB | Native after MiB |
|---|---:|---:|---:|
| 10k feature | 346.1 | 129.2 | 119.5 |
| 100k constant | 2,619.6 | 983.1 | 750.2 |
| Eight layers, 10k features | 2,246.6 | 149.7 | 176.4 |

RSS is the mean of seven process high-water marks, not live plugin allocation
size. The 100k case still uses about 31% more peak memory than native. GPU buffer
memory remains nearly equal: final 10k feature native/plugin is
1,286,868 / 1,287,316 bytes. Snapshot sharing removed the multiplicative CPU
storage without making each tile's retained geometry/properties free.

### Did each improvement help?

| Separate commit | Change | Evidence and limits |
|---|---|---|
| `f7e6fb5c1cbf` | Share immutable feature snapshots across binders/layers | Yes: the first smoke run already removed most RSS inflation; the full final run confirms large RSS and reload reductions. Ownership and constant-to-expression tests pass. This is not an attribution of every saved byte to one allocation site. |
| `88a38f600b13` | Native-like conditional/half-precision Metal paint varyings | The final dense gap shrank from 73.0% to 19.5%, with essentially unchanged native dense wall time. Consistent with less shader work, but no isolated repeated A/B or GPU timestamps establish this commit's exact share. |
| `29f924984312` | Remove unused stencil masks | Yes, deterministically: warm draws changed 5→4, 17→16 and 33→32. A regression test asserts no unused stencil updates. |
| `94c71a7e3776` | Cache uniform storage/values and skip identical GPU uploads | Yes for allocation/upload work: unchanged uniform uploads are now 48 bytes, including eight layers. Callbacks still run each frame, preserving semantics. Multi-layer encoding remains expensive. |
| `5842416c75eb` | Index drawable ranges by feature ID | Yes for sparse range lookup: only affected ranges are evaluated. End-to-end sparse-state timing remains inconclusive; bounds statistics still scan the paint vertices. |

The five intermediate datasets have only one repeat and three measured frames.
They are smoke checks, not statistically reliable per-commit speedup estimates.
The circle implementation (`b47ff67ba75f`), benchmark (`67c7c53ed112`) and profiling
infrastructure (`73c3b279e6bd`) are separate from these performance commits.

### Isolated diagnostics and Instruments

The separate opt-in worker run records cumulative worker durations, which may
overlap and must not be summed as main-thread wall time. At 100k constant
startup: geometry layout 32.90 ms, snapshot construction 65.99 ms, initial paint
binding 0.045 ms. Eight feature-driven layers: 5.10 / 10.09 / 19.56 ms. These
single-repeat diagnostics point to snapshot construction for large constant
layers and binding for many data-driven layers as remaining investigation areas.

Both the one-layer and eight-layer 10k cases create **21,438 feature snapshots**,
not eight times as many. Counts exceed source feature counts because they sum
tile-local copies, including tile buffers. Sparse-state updates evaluate 218
ranges versus 21,438 for full-state updates, with no new snapshots or initial
binding. Measured binder update totals are 0.167 and 6.428 ms respectively;
these include the remaining full-array bounds-statistics scan.

The [successful Instruments capture and exported allocation statistics](../results/metal-circle/allocations/)
cover the optimized eight-layer workload through ten reloads. It records
9,818,144 heap allocations totaling 1,581,442,688 bytes over the entire process,
almost all transient—not 1.58 GB simultaneously live. Typed categories contain
44 shared `PluginFeatureData` allocations and 352 shared paint vertex-vector
allocations, consistent with snapshot sharing and per-layer paint data.
The export does not resolve all generic malloc categories to call stacks, and
there is no matching pre-optimization allocation capture. Consequently it
cannot prove a before/after allocation-count reduction or absence of leaks.

The successful capture required a debugger-enabled temporary executable copy
outside the sandbox; the measured benchmark executable was not modified.
Instrumented timing/RSS is excluded from the comparison tables.

### Remaining work

1. Investigate the remaining dense Metal shader/pipeline gap using GPU timestamps
   and an isolated repeated shader A/B; do not ascribe it to C dispatch.
2. Profile multi-layer callback/property preparation: eight-layer encoding still
   costs about 61% more than native despite matching draw counts and fewer uploads.
3. Reduce snapshot construction/retention for constant layers while preserving
   runtime switches to expressions and source-feature lifetime safety.
4. Explore incremental query-bound statistics for sparse-state updates, then
   repeat the state benchmark under tighter machine-load control.

The original baseline analysis follows for comparison.

## Original baseline

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

The original run is `benchmark/results/circle-metal-pooled/`. Committed
[baseline aggregates](../results/metal-circle/baseline/) preserve machine and
executable/dylib SHA-256 hashes, per-process measurements and confidence intervals.
Only the large raw per-frame logs remain local. The harness is
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

## Prioritized follow-up identified from the baseline

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
were not collected for this original baseline. Do not extrapolate these figures to Android, iOS, OpenGL or
Vulkan, or present them as an intrinsic overhead of a C ABI.
