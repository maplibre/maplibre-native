# Circle plugin: Metal benchmark

The same source revision builds three configurations: plugins disabled with
native circles; plugins enabled with native circles; and plugins enabled with
the circle override loaded from a separate dynamic library. The library links
only the system C/C++ runtimes, not MapLibre C++ symbols. LTO is disabled.

Build from the repository root:

```sh
cmake --preset macos-metal -B build-circle-metal-off -DCMAKE_BUILD_TYPE=Release -DMLN_WITH_PLUGINS=OFF -DMLN_CREATE_AUTORELEASEPOOL=ON -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=OFF
cmake --build build-circle-metal-off --target mln-circle-benchmark -j6
cmake --preset macos-metal -B build-circle-metal-on -DCMAKE_BUILD_TYPE=Release -DMLN_WITH_PLUGINS=ON -DMLN_CREATE_AUTORELEASEPOOL=ON -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=OFF
cmake --build build-circle-metal-on --target mln-circle-benchmark mln-circle-layer mln-circle-render-tests -j6
build-circle-metal-on/plugins/mln-circle-render-tests --manifestPath plugins/circle-layer/render-tests/manifest.json
node benchmark/renderer/run-circle-metal.mjs
```

Use `--smoke` for a quick three-variant execution check. Full runs default to
30 warmup frames, 300 measured frames per phase and seven fresh process repeats.
Run without concurrent builds or other GPU workloads. Raw JSONL, machine
metadata, process means, paired ratios and 95% Student-t confidence intervals
are saved under `benchmark/results/circle-metal`. Intervals use process repeats,
not correlated individual frames. With fewer than two repeats no interval is
reported. The default core background pool has four workers in all variants.
Fresh processes do not clear macOS file caches or Metal's shader/pipeline cache;
startup results are not guaranteed cold-cache measurements. Keep the power mode
and other machine workloads stable. No performance threshold gates correctness.
The executable also wraps complete frame requests in an outer autorelease pool,
matching an application's event loop: shader and drawable creation can happen
before the renderer's internal pool. This is important for meaningful headless
memory measurements.

The fixture is deterministic, inline GeoJSON; no network, sprites or fonts.
All variants use a 1024×768 headless surface at pixel ratio 1, zoom 3. Workloads:
1k/10k/100k constant circles; 10k camera, feature, composite and feature-state
circles; dense overdraw; and eight layers sharing a 10k-feature source.

Measurements:

- `registration`: dynamic library loading and C descriptor registration, before
  creating the map. This is a one-off cost, not frame overhead.
- `startup_readback`: map creation, style parsing, source processing, layout,
  binding, shader/pipeline setup, first render and readback. It excludes fixture
  generation and registration. It is **not an isolated layout timer**.
- `warm_no_readback`: static-map request through rendering completion callback,
  without explicit image readback. Includes host scheduling, CPU submission and
  Metal's headless `waitUntilCompleted`. **Not isolated CPU or GPU elapsed time**.
  `encoding_ms` separately reports the host CPU encoding counter; `submit_wait_ms`
  reports backend commit/wait time. Neither is a Metal GPU timestamp measurement.
- `warm_readback`: the same request with completed image readback.
- `zoom` and `paint_update`: update plus submission, excluding readback.
- `state_1pct` / `state_100pct`: change feature radius on 1% or all features,
  including state API calls, binder updates and submission (state workload only).
- `style_reload`: reload the unchanged style/source and submit the first complete
  frame, up to ten samples per process. Includes source/layout/binding work;
  not a measure of incremental GeoJSON updates.

Each frame also records draw calls, buffer memory and upload counters. Counters
are backend-reported: creation-time allocations need not appear as update
bytes. In particular, `uniform_upload_bytes` counts backing-storage updates,
**not** all bytes supplied to Metal. The `metal_vertex_inline_bytes` and
`metal_fragment_inline_bytes` counters measure actual `setVertexBytes` and
`setFragmentBytes` submissions after binding deduplication, including small
vertex arrays and the per-drawable index. Corresponding `*_bytes_calls` count
these calls; `*_buffer_binds` count `setVertexBuffer`/`setFragmentBuffer` calls
(not offset-only changes). These are CPU encoder submissions, not GPU/PCIe
bandwidth measurements. Peak RSS is process-wide and uses `getrusage` native
units (bytes on macOS).
Metal GPU timestamps are not collected. For a separate diagnostic run, add
`--profile` to the script. It records cumulative worker time separately for
geometry layout, immutable snapshots, initial binding and state updates, plus
snapshot and affected-range counts. Worker sums can overlap and are not wall
time. Instrumented runs must not be used as headline performance comparisons.
Profiling is disabled by default and does not change the public plugin ABI.

To publish compact results, preserving every process mean and confidence interval
without the large per-frame logs:

```sh
node benchmark/renderer/export-circle-results.mjs benchmark/results/circle-metal benchmark/results/metal-circle/optimized
```

The exported `processes.json` retains sample counts, mean/p50/p95 for each metric
and phase, and peak RSS for each process. Registration-time rendering counters
reflect the later first frame, not work done by registration; use only `wall_ms`
for that phase. Machine metadata and binary hashes are retained unchanged.

Correctness uses the original core circle fixtures and expected images unchanged:
58 supported fixtures, excluding only the unimplemented `circle-sort-key` test.
The `--builtin` option on the circle render runner supplies a native reference.
All eleven circle paint properties are supported; layout sort keys and animated
transition parity are outside this comparison.

## Focused before/after comparisons

Preserve the pre-change benchmark executable and plugin dylib before rebuilding.
The focused runner interleaves before-native, before-plugin, after-native and
after-plugin processes, rotating/reversing order across seven repeats. It refuses
to overwrite an existing output directory and records hashes for all four input
artifacts. For example:

```sh
node benchmark/renderer/compare-circle-metal.mjs \
  --before-bin /path/to/saved/mln-circle-benchmark \
  --before-plugin /path/to/saved/libmln-circle-layer.dylib \
  --after-bin build-circle-metal-on/benchmark/mln-circle-benchmark \
  --after-plugin build-circle-metal-on/plugins/libmln-circle-layer.dylib \
  --output benchmark/results/circle-metal-followup
```

Defaults: dense, eight-layer, 100k-constant and state-update workloads; 100
warmups, 300 measured frames, seven repeats (112 processes). Use `--cases dense`
for the isolated dense-shader comparison. `processes.json` preserves per-process
phase means, counts and peak RSS; `comparisons.json` includes paired before/after
and native-control ratios with process-level 95% Student-t intervals. Copy those
two files and `metadata.json` to publish compact results; retain raw JSONL locally.
Do not run builds, render tests or profilers concurrently with timing runs.

Opt-in `--profile` executable output also includes `plugin_bounds_vertices`:
the actual number of vertices revisited while refreshing dirty bounds blocks.
The existing `plugin_snapshot_ms` now times owned feature-view setup, not deep
cloning. Diagnostic timings/counts must be collected separately from headline
timing runs, and neither field measures hardware GPU time.
