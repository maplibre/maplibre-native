# Circle plugin submission investigation

**Latest:** the [dense-shader, feature-ownership and bounds follow-up](circle-followup-results.md)
implements the priorities identified below and records isolated and interleaved
before/after results. This document preserves the earlier investigation.

Investigated `perf/circle-plugin-metal` at `c5f90bafc72d`, using the Metal
benchmark on Apple M3 Max. The initial findings were committed separately as
`e5db20e05af0`, before applying fixes. Temporary probes and diagnostic switches
were removed, the normal benchmark rebuilt, and a restored-binary smoke run
passed. The subsequent implementation and benchmark rerun are recorded below;
the diagnostic evidence in the original sections is preserved.

## Finding

The suggestion is partly right: there is unnecessary work at the generic
render-layer/plugin boundary and a less efficient uniform-submission layout.
However, the measured unchanged-frame workload does **not** show duplicate
rendering, drawable recreation, repeated geometry layout, dirty-flag churn or
repeated Metal pipeline construction.

Two concrete inefficiencies were identified:

1. `PluginBucket::synchronizePaint()` compares the paint map, then assigns it
   into `latestPaintProperties` even when it is unchanged. The generic layer
   calls it for each layer/tile on every frame, before taking the retained-drawable
   fast path. In the eight-layer case this is 32 unnecessary map assignments
   per frame, plus repeated map comparisons/lookups. Each map contains the eleven
   circle paint properties. This is not a claim that each assignment allocates
   eleven new nodes; the standard-library implementation can reuse storage.
2. The circle plugin combines tile/camera inputs and paint values in one
   240-byte per-drawable UBO used by both shader stages. Native circle separates
   layer-wide paint from tile data and consolidates its drawable UBOs. MapLibre's
   small-buffer Metal path calls `setVertexBytes`/`setFragmentBytes` when binding
   these resources, even if their backing storage has not changed. The plugin
   consequently copies substantially more inline bytes into the encoder.

Relevant source locations in the restored checkout:

- [Paint synchronization](https://github.com/maplibre/maplibre-native/blob/c5f90bafc72d/src/mln/renderer/buckets/plugin_bucket.cpp#L472)
- [Per-frame plugin layer update](https://github.com/maplibre/maplibre-native/blob/c5f90bafc72d/src/mln/renderer/layers/render_plugin_style_layer.cpp#L153)
- [Plugin uniform preparation](https://github.com/maplibre/maplibre-native/blob/c5f90bafc72d/src/mln/renderer/layers/plugin_layer_tweaker.cpp#L23)
- [Native circle uniform batching](https://github.com/maplibre/maplibre-native/blob/c5f90bafc72d/src/mln/renderer/layers/circle_layer_tweaker.cpp#L27)
- [Metal inline-byte binding](https://github.com/maplibre/maplibre-native/blob/c5f90bafc72d/src/mln/mtl/buffer_resource.cpp#L168)
- [Uniform stage binding](https://github.com/maplibre/maplibre-native/blob/c5f90bafc72d/src/mln/mtl/uniform_buffer.cpp#L42)

## Exact warm-frame counts

Fixture: 10,000 feature-driven circles, eight layers sharing one source, four
visible tile drawables per layer, fixed camera. After 100 warmups, all listed
counters were identical on **each of 100 measured frames**, not merely on average.

| Event per frame | Native | Plugin |
|---|---:|---:|
| Rendered frames per benchmark request | 1 | 1 |
| Metal drawable upload-path visits | 32 | 32 |
| Metal drawable draws | 32 | 32 |
| Vertex attribute rebuilds | 0 | 0 |
| Index buffer rebuilds | 0 | 0 |
| Pipeline lookups due to a missing drawable pipeline | 0 | 0 |
| Depth/stencil state creation | 0 | 0 |
| Vertex/index update bytes | 0 / 0 | 0 / 0 |

The plugin had exactly 8 layer updates, 8 layer-tweaker calls, 32 paint-sync
calls, 32 drawable-tweaker visits and 32 C uniform callbacks per frame. These
match the layer/tile population, rather than indicating duplicate invocation.
There were zero paint evaluations, changed paint snapshots, binder refills,
paint dirty-set operations, drawable additions, sync-requested rebuilds,
layout/snapshot/binding work or plugin-uniform backing-store updates.

Calling the Metal drawable `upload()` path every frame is normal for both
implementations. The guards correctly avoided rebuilding its resources here.
The broader pipeline-cache lookup counter does not fire either, so this is
stronger evidence than merely observing no shader recompilation.

## A misleading upload metric

| Metal calls / bytes per frame | Native | Plugin |
|---|---:|---:|
| Existing `uniformUpdateBytes` counter | 3,632 | 48 |
| `setVertexBytes` calls | 49 | 65 |
| Vertex inline bytes | 4,272 | 7,856 |
| `setFragmentBytes` calls | 41 | 65 |
| Fragment inline bytes | 688 | 7,856 |
| **Total inline-byte calls** | **90** | **130** |
| **Total inline bytes** | **4,960** | **15,712** |
| Vertex `MTLBuffer` bindings | 64 | 64 |
| Fragment `MTLBuffer` bindings | 0 | 0 |

The inline totals include the shared global paint UBO and the four-byte drawable
index sent to each stage; those costs are equal between implementations. The
plugin's extra 10,752 bytes come from the uniform organization, not geometry
uploads. Fragment inline data alone is over eleven times the native amount.

**Correction to interpretation of the earlier report:** 48 bytes measures
updates to MapLibre's buffer storage, not total data supplied to the Metal
encoder. It does not mean only 48 bytes are submitted. Inline bytes are encoder
copies; these numbers are not PCIe traffic or GPU hardware bandwidth readings.

## Timing evidence and limits

The first separately instrumented eight-layer run attributed 19.07 microseconds
to plugin layer updates, versus 1.59 to native updates. Paint synchronization
accounted for 13.98 microseconds within that 19.07. Plugin uniform preparation
was 4.42 microseconds versus native's 1.61. Thus, in that sample, the layer-update
path was the larger CPU difference, not the C uniform callback path.

A temporary switch that omitted **only unchanged paint-map assignment** reduced
the isolated sync timer to 6.16 microseconds, and layer update to 11.15, in the
corresponding single diagnostic run. A second switch bypassed already-initialized
uniform preparation only for fixed-camera warm frames. It reduced the tweaker
timer as expected, but is deliberately **not a valid general fix**: animated,
stateful, camera-dependent and changing-paint callbacks must remain correct.

The follow-up unprofiled experiment used seven fresh process repeats, 500 warm
samples per process, 100 warmups, rotated/reversed native/plugin/diagnostic
variant order, and one- and eight-layer workloads. Timing varied substantially:
for eight layers the paired CPU saving from skipping the copy was -2.40 ± 43.00
microseconds (95% Student-t interval half-width). That interval includes zero.
It does **not** support a reliable end-to-end speedup percentage. Subsequent
instrumented timings also varied while operation counts remained exact.
No thermal warning was reported; other desktop applications were active.

The exact counts and unconditional source assignment establish unnecessary
work. The isolated timers help prioritize it, but do not quantitatively attribute
all of the earlier benchmark's CPU gap or establish a production speedup.

## Suggested fixes at the time of investigation

1. Stop replacing unchanged paint maps; then avoid deep comparisons/copies on
   every tile by retaining an immutable evaluated snapshot/revision. Camera,
   transition and feature-state changes must invalidate the correct state.
2. Cache per-layer shader-group resolution and `ProgramParameters` instead of
   constructing/checking them each update. The current registry prevents shader
   recompilation, but the lookup/string preparation still happens.
3. Support/reuse layer-wide uniform storage and batched drawable storage in the
   generic host; split circle paint from tile/camera data and restrict stage
   bindings appropriately. Avoid bypassing C callbacks without an explicit
   dependency/lifecycle contract.
4. Add actual Metal inline-byte call/byte counters to the benchmark, distinct
   from storage-update counters. Retain tests for zero warm drawable/pipeline
   churn and for correct updates after paint, zoom and feature-state changes.

This diagnosis covers the measured stable-camera feature-driven workload. It
does not rule out separate invalidation bugs during tile churn, transitions,
feature-state changes or context restoration, nor does it cover other backends.

## Reproduction artifacts

The original diagnostic artifacts below describe the pre-fix implementation.

[Committed measurements](../results/metal-circle/submission-investigation/) preserve
the per-process timing means, diagnostic counts and direct Metal binding records.
The temporary executable and diagnostic switches are not part of production code.

The local directory `/tmp/circle-submission-investigation.Iw11CN` retains the exact temporary `instrumentation.patch`, a diagnostic
`run.mjs`, raw per-frame logs and `profile-means.json`/`timing-means.json`. The
direct byte-count recordings are `binding-native.jsonl` and `binding-plugin.jsonl`.
The source patch is intentionally **not applied** to the checkout.

The saved instrumented executable is `mln-circle-benchmark-instrumented`
(SHA-256 `7f21eef7b7059ac6f3dfe6454e5d011d983b8384a43602fac4b9746005b3aed7`),
with `libmln-circle-layer.dylib`
(`fc52ae84617290be677ecdba74f07de2ab79c0504617e47aba9e418a4d638d8f`). It includes
the later inline-byte probes; `artifacts.json` records the earlier binary used
for the initial profile and seven-repeat experiment. Rerunning `run.mjs` would
replace its result files, so copy the directory first to preserve this evidence.

## Follow-up implementation

The findings and each implementation step are separate commits:

| Commit | Change |
|---|---|
| `e5db20e05af0` | Preserve the original findings and diagnostic measurements before fixing anything. |
| `4b4eba1f09ca` | Retain one immutable evaluated paint snapshot per render layer; tiles compare shared identity, not eleven-property maps. Recompute query bounds on zoom changes independently. |
| `3b196fdbc68d` | Resolve immutable shader groups once per render layer, matching built-in layers' ownership. Keep late plugin registration supported. |
| `85056643d70a` | Add explicit drawable, layer, and drawable-array uniform scopes. Share layer buffers; batch drawable buffers on Metal. Validate scopes and reject tile interpolation factors in layer-wide storage. Suppress draws after failed callbacks and retry safely. |
| `7a603669218c` | Split circle's combined 240-byte block into a 160-byte vertex-only tile block and an 80-byte layer-wide paint block. Metal shaders index the batched tile blocks. |
| `659f20016033` | Count actual Metal inline-byte calls/bytes and buffer binds, separately from backing-storage updates. Include them in benchmark exports and exact-count regression tests. |

The unpublished C API remains v1, but plugins must rebuild against its new uniform
descriptor. Layer scope explicitly permits one callback per layer/shader/frame;
ordinary and array scopes still invoke the callback per drawable/frame. In the
eight-layer circle workload this means 32 tile callbacks plus 8 paint callbacks,
not skipping arbitrary stateful callbacks. The batching is a storage/submission
optimization, not duplicate geometry or a promise of fewer C calls overall.
OpenGL and Vulkan currently use individual tile blocks for array declarations;
this follow-up validates and benchmarks **Metal only**.

Validation after the changes:

- **45/45 plugin/query tests** pass, including retained paint ownership,
  unchanged snapshots, zoom-bound invalidation, scope validation, late registration,
  per-layer versus per-drawable callback counts, live paint changes, tile-count
  shrink/growth, callback failure/recovery, and exact Metal binding counts.
- **58/58 circle fixtures** and **13/13 ngon fixtures** pass on Metal, with no
  expected-image or tolerance changes. Circle descriptor/geometry/segmentation/
  map-mode/query tests also pass.
- Release Metal builds pass with plugins enabled and disabled. No Darwin SDK
  files were modified. No OpenGL/Vulkan performance or correctness claim is made.

The new tests exercise resource updates and failure recovery, not complete
context-loss or all pipeline-invalidation paths. The original temporary pipeline
and dirty-flag probes were not added to the production renderer.

## Full benchmark rerun after the fixes

The [`submission-fixed` dataset](../results/metal-circle/submission-fixed/) was
captured at `659f20016033` with an empty tracked source diff, on the same Apple
M3 Max/macOS 26.6.2. Both Release builds include the same lightweight encoder
counters. All **189 fresh processes** completed: nine workloads × three variants
× seven repeats, 30 warmups and 300 measured requests per normal phase, with
ten style reloads. No builds, render tests or Instruments sessions ran during
the sweep. Desktop background activity and OS/driver caches were not controlled.
Raw logs remain in `benchmark/results/circle-metal-submission-fixed`; exported
metadata includes source revision, timestamp and binary hashes.

### Submission work: definite improvement

These counters were constant on every one of the **2,100 warm eight-layer frames
per variant**. “Before” is the original diagnostic recording, not a newly timed
old binary. Backing-storage updates and inline submissions must not be added
together as if they measured the same operation.

| Per eight-layer warm frame | Plugin before | Plugin after | Native after |
|---|---:|---:|---:|
| Draws | 32 | 32 | 32 |
| Vertex/index update bytes | 0 / 0 | 0 / 0 | 0 / 0 |
| Uniform backing-storage update bytes | 48 | 48 | 3,632 |
| Vertex inline calls | 65 | 49 | 49 |
| Fragment inline calls | 65 | 41 | 41 |
| Vertex inline bytes | 7,856 | 5,936 | 4,272 |
| Fragment inline bytes | 7,856 | 816 | 688 |
| **Total inline calls** | **130** | **90** | **90** |
| **Total inline bytes** | **15,712** | **6,752** | **4,960** |
| Vertex / fragment MTLBuffer binds | 64 / 0 | 64 / 0 | 64 / 0 |

Inline calls fall **30.8%** and bytes **57.0%**. The excess bytes over native
fall from 10,752 to 1,792 (**83.3% less excess**), but are not eliminated: the
plugin still has larger tile and paint blocks. The immutable paint snapshot
also removes the unconditional per-tile map assignments and comparisons by
construction, with ownership/unchanged-frame regression coverage. Shader-group
resolution no longer runs on the retained-layer update path.

### CPU and wall time: promising, but not established parity

“Before” below is the previously committed [`optimized`](../results/metal-circle/optimized/)
full run, not the noisy temporary-probe experiment. CPU values are means of
seven process means in microseconds. Ratios are paired within the new sweep,
with 95% Student-t intervals across processes; an interval containing 1 is
**not proof of equivalence**. No outliers were removed.

| Workload | Plugin CPU before µs | Plugin CPU after µs | Native CPU after µs | New plugin/native CPU ratio, 95% CI |
|---|---:|---:|---:|---|
| 1k constant | 22.42 | 21.83 | 22.76 | 0.984 [0.786, 1.183] |
| 10k constant | 23.88 | 20.45 | 20.63 | 0.998 [0.888, 1.108] |
| 100k constant | 32.17 | 32.34 | 26.84 | 1.223 [1.058, 1.389] |
| 10k camera | 23.41 | 21.72 | 21.67 | 1.003 [0.937, 1.069] |
| 10k feature | 23.22 | 22.03 | 20.99 | 1.050 [1.018, 1.082] |
| 10k composite | 24.54 | 22.45 | 22.25 | 1.010 [0.953, 1.067] |
| 10k state, unchanged | 23.65 | 21.65 | 20.38 | 1.062 [1.032, 1.093] |
| 10k dense | 41.93 | 42.58 | 37.42 | 1.114 [0.951, 1.277] |
| Eight layers, 10k features | 65.44 | 47.88 | 52.01 | 0.958 [0.691, 1.225] |

The eight-layer plugin mean drops **26.8%**, consistent with removing hot-path
work. However, native's mean also moves from 40.72 to 52.01 µs, and the new
process means range from 34.73–66.76 µs for native and 40.15–82.34 µs for the
plugin. The wide ratio interval does not establish a precise speedup against
native or explain the contribution of each commit. There was no separately
randomized before/after run per fix. Most single-layer means improve modestly;
100k constant and dense CPU means do not improve, and some residual CPU gaps
remain measurable.

Eight-layer wall time is 0.842 ms plugin versus 0.992 ms native, ratio
0.876 [0.711, 1.041], likewise inconclusive. Dense wall time remains
**3.754 versus 3.113 ms**, ratio **1.206 [1.195, 1.216]**. The previous plugin
mean was 3.759 ms: the submission fixes did not resolve the dense rendering gap.

Lifecycle and update costs also remain important:

- 100k startup: **199.81 / 187.74 ms** plugin/native; reload:
  **169.86 / 136.52 ms**, still about 24.4% slower on reload.
- Eight-layer reload: **25.74 / 34.55 ms**; this advantage already existed
  before these submission fixes.
- Change 1% of feature states: **0.710 / 0.590 ms**, ratio
  **1.204 [1.190, 1.219]**. Change all states: **9.77 / 11.17 ms**, ratio
  **0.874 [0.859, 0.890]**. These do not support a blanket claim that plugin
  state updates are faster.
- 100k peak RSS: **989.4 / 760.1 MiB**; eight layers: **165.1 / 171.8 MiB**.
  These are process-wide high-water means, not allocations attributed to one
  subsystem. This round did not target snapshot/layout memory.

**Conclusion:** the diagnosed unnecessary submission work is fixed and the
operation-count savings are reproducible. Overall Metal performance parity
is not established. Next priorities remain dense shader/GPU work, large-source
layout/storage, and a quieter controlled CPU experiment—not speculative dirty
flag changes unsupported by the measured warm-frame behavior.
