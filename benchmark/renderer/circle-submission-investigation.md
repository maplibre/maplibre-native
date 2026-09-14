# Circle plugin submission investigation

Investigated `perf/circle-plugin-metal` at `c5f90bafc72d`, using the Metal
benchmark on Apple M3 Max. No implementation fixes were applied or pushed.
Temporary probes and diagnostic switches were removed, the normal benchmark
rebuilt, and a restored-binary smoke run passed.

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
