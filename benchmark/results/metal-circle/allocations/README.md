# Instruments allocation capture

Successful capture on 2026-09-14, macOS 26.6.2 / Apple M3 Max, Instruments 16.0
(17E192), optimized plugin revision `73c3b279e6bd`. The process exited normally
with status 0; recording duration was 7.139379 seconds. The 45-second cap was not
reached. This is a diagnostic capture, not a performance-comparison sample.

Workload: 10,000 features shared by eight layers, feature-driven paint, spread
distribution, 30 warmups, 30 measured requests per normal phase and ten reloads.
No internal `--profile` instrumentation. Full trace kept locally at
`/tmp/maplibre-circle-allocations-debuggable.trace`; committed
[`statistics.xml`](statistics.xml) contains the exported Allocations statistics.

The original benchmark could not be attached to, even after authorization.
Recording succeeded outside the sandbox after ad-hoc signing a **temporary copy**
with `com.apple.security.get-task-allow=true`. The original measured executable
and system security settings were not changed.

SHA-256:

| Artifact | Digest |
|---|---|
| Original benchmark | `36f136913e52685be2fefa4bde73768a3cb3dde7e80188a6b7febaf9df6862bd` |
| Debugger-enabled copy | `0e960970d2922e0c1190b8cef3dc064b442f985356aaf9a2152b1a786224dd94` |
| Circle dylib | `fc52ae84617290be677ecdba74f07de2ab79c0504617e47aba9e418a4d638d8f` |

Reproduce with your debugger-enabled executable and plugin paths:

```sh
xcrun xctrace record --template Allocations --time-limit 45s --no-prompt \
  --output circle.trace --target-stdout circle.jsonl --launch -- \
  /absolute/path/to/debuggable/mln-circle-benchmark \
  /absolute/path/to/libmln-circle-layer.dylib 10000 8 feature 30 30 spread
xcrun xctrace export --input circle.trace \
  --xpath '/trace-toc/run[@number="1"]/tracks/track[@name="Allocations"]/details/detail[@name="Statistics"]' \
  --output statistics.xml
```

The export reports **9,818,144 heap allocations totaling 1,581,442,688 bytes**
over the entire run. Of those bytes, 1,580,378,736 were transient and 1,063,952
were persistent at the selected interval's end. These are allocation totals,
not peak resident memory or live bytes during rendering. The benchmark's own
instrumented peak RSS was 237,780,992 bytes; do not substitute this for the
uninstrumented result.

Typed categories include 44 `PluginFeatureData` shared allocations and 352
`PluginPaintVertexVector` shared allocations across the complete run. These
counts are consistent with sharing snapshots across layers while retaining
per-layer paint buffers. Generic malloc categories still account for much of the
allocation volume; this export alone does not attribute those bytes to individual
call stacks. No matching pre-optimization allocation trace was captured, so it
does not establish a before/after allocation-count reduction or prove absence of
leaks.
