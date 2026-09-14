# Metal circle follow-up: shaders, feature ownership and bounds

2026-09-14, Apple M3 Max, macOS 26.6.2. This continues the
[submission investigation](circle-submission-investigation.md).

## Outcome

The dense Metal gap was largely explained by the plugin's shader buffer address
spaces, not duplicate rendering or drawable dirty flags. In an **isolated
28-process experiment**, changing only the circle shader's buffer declarations
from `constant` to `device const`, matching built-in circle, reduced plugin wall
time **3.839 → 3.196 ms**. Matched native was **3.194 ms**. The paired
after/before plugin ratio is **0.833, 95% CI [0.827, 0.838]**; native is
**1.009 [0.994, 1.024]**. No geometry or coverage math changed.

A subsequent **112-process comparison** confirms a dense improvement and smaller
large-source reload/memory costs. It does not establish universal parity or a
reliable sparse-state wall-time improvement.

## Separate implementation commits

| Commit | Change | Scope |
|---|---|---|
| `825e262d5c7c` | Match native circle Metal buffer address spaces | Circle shader only; identical core executable in the isolated experiment. |
| `6c88b593661c` | Retain owned tile feature views instead of deep snapshots | Generic plugin buckets own the source layer, enclosing the lifetime of retained views. No eager cloning of geometry/property maps. |
| `39038abd739b` | Cache paint bounds in 128-vertex blocks | Generic binders recompute touched blocks and merge cached extrema, including decreasing extrema. |
| `98055713adce` | Add an interleaved before/after runner | Benchmark-only code, recording process means and paired intervals. |
| `cb600455445c` | Test retained MVT/MLT feature lifetimes | Additional coverage beyond GeoJSON ownership, expression changes and bounds tests. |

The feature owner is declared before the views, so it is destroyed after them.
`GeometryTileData` permits a returned layer to outlive its data; features must not
outlive their layer. Construction happens on the layout worker, followed by
render-thread evaluation. Lazy feature caches are not read concurrently across
these phases. Tests release original tile data before exercising retained MVT,
MLT and GeoJSON properties/geometry. Constant layers can still switch to
data-driven expressions without losing source information.

The bounds cache does not change drawable dirty flags or buffer uploads. A small
state update still uploads a whole changed paint attribute; this removes CPU
bounds scanning, not GPU upload bytes.

## Method and evidence

[Committed datasets](../results/metal-circle/followup/) contain metadata, binary
SHA-256s, process phase means/sample counts, paired comparisons and separate
opt-in diagnostics. Large per-frame logs remain local under
`/tmp/circle-metal-followup.Dgy0XT`. See [reproduction instructions](circle-metal.md)
and [`compare-circle-metal.mjs`](compare-circle-metal.mjs).

Both experiments use seven fresh-process repeats per variant, 100 warmups,
300 measured requests per normal phase and ten style reloads. Each repeat
interleaves before-native, before-plugin, after-native and after-plugin in a
rotated/reversed order. No builds, render tests or profiling ran concurrently.
No outliers were excluded. Desktop background activity, OS/driver caches and
thermal conditions were not controlled. Intervals are Student-t 95% intervals
over seven process means or within-repeat ratios, not independent frames.
Ratio means therefore need not equal ratios of timing means. There is no
multiple-comparison adjustment.

Baseline binaries are from submission-fixed source (`659f20016033`, documented
at `f2b662b8841d`). The isolated metadata records `f2b662b8841d` plus the
then-uncommitted shader change; **before/after core hashes match**. Its after-plugin
hash matches the final plugin. The full comparison records `cb600455445c` with an
empty tracked source diff. Both use Release Metal, plugins enabled, autorelease
pooling enabled and LTO disabled; native means the same executable without plugin
registration. This is not a new plugin-off sweep. All **28 + 112 processes** and
**8,400 + 33,600 warm samples** completed.

Warm wall time includes headless Metal completion without readback. CPU encoding
is a separate host counter. **Neither is a hardware GPU timestamp.** The isolated
result attributes a workload improvement to address-space declarations, not a
particular compiler instruction or cache mechanism. No OpenGL, Vulkan or mobile
performance claim follows from these runs.

## Isolated shader result

| Dense warm wall time | Before ms | After ms |
|---|---:|---:|
| Native | 3.164 | 3.194 |
| Plugin | 3.839 | 3.196 |

After plugin/native ratio: **1.001 [0.994, 1.008]**. This tightly bounds this
particular workload, not all circle styles. The 16.7% plugin improvement preceded
the feature-view and bounds changes. CPU encoding differences are too noisy for
a precise CPU speedup claim.

## Full follow-up comparison

| Warm workload | Plugin before ms | Plugin after ms | Native after ms | After plugin/native ratio, 95% CI |
|---|---:|---:|---:|---|
| 10k dense | 3.858 | 3.277 | 3.252 | 1.008 [0.979, 1.038] |
| Eight layers, 10k features | 1.043 | 1.006 | 1.061 | 0.972 [0.798, 1.146] |
| 100k constant | 0.908 | 0.970 | 0.920 | 1.066 [0.868, 1.264] |
| 10k state, unchanged | 0.406 | 0.430 | 0.443 | 0.984 [0.815, 1.154] |

Full-run dense after/before plugin ratio: **0.850 [0.810, 0.890]**; native:
**0.991 [0.971, 1.011]**. Other warm wall-time comparisons are inconclusive.
Eight-layer CPU encoding is **49.1 → 50.8 µs** plugin, **40.4 → 41.3 µs** native;
final paired plugin/native CPU ratio **1.223 [0.914, 1.532]**. This round does not
demonstrate a multi-layer CPU improvement. Submission counts remain as previously
documented.

| Operation | Plugin before ms | Plugin after ms | Native after ms | Plugin after/before ratio, 95% CI |
|---|---:|---:|---:|---|
| 100k startup/readback | 208.69 | 201.61 | 190.08 | 0.970 [0.906, 1.033] |
| 100k style reload | 172.28 | 157.97 | 140.34 | 0.917 [0.909, 0.925] |
| Eight-layer style reload | 26.07 | 24.11 | 34.89 | 0.925 [0.883, 0.967] |
| 10k state, change 1% | 0.829 | 0.765 | 0.699 | 0.972 [0.571, 1.373] |
| 10k state, change 100% | 10.225 | 10.070 | 11.461 | 0.985 [0.973, 0.997] |

100k reloads improve about **8.3%**; native after/before ratio:
**1.014 [0.991, 1.037]**. The plugin remains **12.6% slower** on that operation,
ratio **1.126 [1.102, 1.150]**. Startup improvement is inconclusive. Full-state
updates remain faster than native, but their small before/after gain is similar
to native's movement and should not be attributed wholly to the bounds cache.

| Mean process peak RSS | Plugin before MiB | Plugin after MiB | Native before / after MiB |
|---|---:|---:|---:|
| Dense | 177.0 | 155.8 | 127.5 / 132.1 |
| Eight layers | 155.6 | 143.6 | 173.1 / 177.5 |
| 100k constant | 972.0 | 884.0 | 761.0 / 755.9 |
| State | 167.7 | 155.2 | 142.1 / 140.0 |

100k plugin peak RSS falls **9.1%**, but remains **16.9% above native**. These
are process high-water marks, not allocations attributed solely to feature views.
Only the shader change has a separate isolated timing experiment.

## Sparse bounds: less work, wall-time speedup not established

The separate state diagnostic counts **85,752 paint vertices** across tiles for
10k source points. Updating 1% of source IDs touches 218 tile-feature ranges and
revisits **1,024 vertices** for bounds, **98.8% fewer** than the old full scan.
An unchanged pass revisits none; changing all states revisits all 85,752. IDs
recur in tile buffers; different spatial distributions can touch more blocks.

Tests assert visited counts for initial, unchanged, cross-block and partial-tail
updates, and correct extrema after decreasing values. The separate 100k
diagnostic retains 214,347 views and records 39.24 ms of view setup; this is not
a controlled timing comparison with historical snapshot diagnostics. Diagnostic
timings/RSS are excluded from the tables.

## Validation and remaining limits

- **48/48 plugin/query tests**, **58/58 circle Metal fixtures**, and **13/13 ngon
  Metal fixtures** pass. Expected images and tolerances are unchanged.
- Circle descriptor, geometry, segmentation, map-mode and query tests pass.
- The updated plugins-enabled Metal Release build passes. The matched disabled
  build passed in the previous round; no new disabled-build timing claim is made.
- No Darwin SDK files changed. OpenGL/Vulkan are not validated here.

The prioritized follow-up is implemented. Remaining opportunities are controlled
multi-layer CPU profiling, large-source retained memory/reloads, and partial
paint-buffer uploads for sparse state changes. Those need separate measurements
and correctness work. Nothing here supports speculative changes to drawable
dirty flags or skipping required renderer callbacks.
