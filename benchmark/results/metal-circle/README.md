# Metal circle benchmark evidence

See the [analysis](../../renderer/circle-metal-results.md) and
[reproduction instructions](../../renderer/circle-metal.md).

These are measurements, not render-test expected images or performance gates.

| Directory | Purpose | Fresh processes | Frames / warmups / repeats |
|---|---|---:|---|
| `baseline` | Original circle plugin, before optimizations | 189 | 300 / 30 / 7 |
| `optimized` | All five optimizations; internal profiling disabled | 189 | 300 / 30 / 7 |
| `shared-snapshots` | Incremental smoke measurement after snapshot sharing | 27 | 3 / 2 / 1 |
| `specialized-shader` | Incremental smoke measurement after Metal specialization | 27 | 3 / 2 / 1 |
| `no-stencil` | Incremental smoke measurement after stencil removal | 27 | 3 / 2 / 1 |
| `cached-uniforms` | Incremental smoke measurement after uniform caching | 27 | 3 / 2 / 1 |
| `indexed-state` | Incremental smoke measurement after sparse-state indexing | 27 | 3 / 2 / 1 |
| `profile` | Separate opt-in worker timers and operation counters | 27 | 3 / 2 / 1 |
| `allocations` | Separate Instruments Allocations capture, optimized eight-layer plugin | 1 | 30 / 30 / 1 |

Each normal run preserves:

- `metadata.json`: original revision, tracked diff digest, artifact SHA-256 hashes,
  hardware, OS and sampling configuration. Paths identify the local artifacts at
  measurement time; no local files are required to read these results. Some
  incremental runs preceded their commits; the final report maps changes to commits.
- `summary.json`: all process means for wall/encoding time, paired variant ratios,
  and 95% Student-t intervals across process repeats, not individual frames.
- `processes.json`: per-process peak RSS and per-phase sample counts and
  mean/p50/p95 for timings and counters. Percentiles are nearest-rank within each
  process, not aggregate population percentiles. Native RSS units are bytes on macOS.

The exporter retains registration-time rendering counters for fidelity, but those
counters describe the later first frame. Only registration `wall_ms` is meaningful.
Single-repeat smoke/profile runs have **no statistical confidence intervals** and
cannot establish a timing improvement. They do show deterministic draw/upload
changes and provide diagnostic evidence. Instrumented runs are not comparable to
uninstrumented wall times or RSS.

Large raw per-frame logs and the approximately 1 GiB Instruments trace remain
local. Interrupted benchmark runs are excluded. No expected images or tolerances
were modified for these measurements.
