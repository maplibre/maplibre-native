# Extrusion optimization experiments

Measurements use the same Release build, lavapipe driver, four software-renderer
threads, 10,000 buildings, 512×512 view, and camera as the
[instancing baseline](README.md). Three trials alternate a frozen previous
plugin executable and the candidate. Each phase has 40 measured frames after
ten warmup frames. Builds and correctness tests are idle during measurement.
Figures below are medians of the trials' p50 values. These measurements include
image readback and do not establish physical GPU performance.

To reproduce an individual experiment, build its parent revision and save the
benchmark executable, then build the experiment and run:

```sh
LP_NUM_THREADS=4 python3 plugins/fill-extrusion/benchmarks/run.py \
  build-plugin-vulkan/plugins/mln-fill-extrusion-benchmark \
  --baseline-plugin /tmp/previous-plugin-benchmark \
  --side 100 --frames 40 --trials 3 --output /tmp/experiment.json
```

In the captures, `baseline` and `plugin` both run the plugin implementation;
the difference is the executable. The original built-in/plugin comparison
remains available by omitting `--baseline-plugin`.

## Share identical paint endpoints — kept

Baseline: `c10bc57069e9`. Source-only expressions cannot change with zoom, but
their paint buffers stored two identical values per outline record. Separate
minimum/maximum attributes now alias a single sample. Packed scalar/vector
attributes still retain both components required by their shader declarations.
The expression is evaluated once when it does not depend on zoom. Switching
to a composite expression reallocates two samples; feature-state and constant
transitions retain their existing behavior. No precision is lost.

| Data-driven scene | Before | After |
| --- | ---: | ---: |
| Active vertex buffers | 5.400 MB | 3.600 MB |
| Steady buffer allocations | 5.672 MB | 3.872 MB |
| Steady frame | 24.885 ms | 24.769 ms |
| Feature-state frame | 24.966 ms | 24.764 ms |
| Feature-state CPU encoding | 0.199 ms | 0.191 ms |

Keep for the deterministic 32% allocation reduction. Timing changes are small;
there is no demonstrated frame-time improvement. Other steady scenes vary by
−1.3% to +0.7%, consistent with run-to-run variation. Load timing is noisy.

Validation: 50 focused tests, 50 eligible solid renders, four queries, and all
13 n-gon renders pass. The pattern suite remains 7/8 with the same existing
`tile-buffer` failure. Tests cover source/composite/constant switches, retained
state, statistics, and the existing rounded/patterned direct comparisons.
[Raw captures](shared-endpoints.json).

## Skip color work in depth-only passes — kept

Baseline: `3a1c7e1e613b`. The host exposes `MLN_PLUGIN_COLOR_WRITE` to shaders,
and includes pass/resource switches in the shader cache key. Extrusion depth
variants stop after position calculation and omit fragment texture sampling.
Color variants retain the original lighting/pattern behavior. Cache keys also
separate undefined and defined patterns with otherwise identical attributes.

| Scene/phase | Before | After |
| --- | ---: | ---: |
| Pattern steady | 49.571 ms | 45.248 ms |
| Pattern paint updates | 50.119 ms | 45.674 ms |
| Translucent steady | 41.744 ms | 42.091 ms |
| Data-driven steady | 24.534 ms | 24.721 ms |
| Data-driven paint updates | 25.408 ms | 28.077 ms |
| Rounded steady | 94.947 ms | 96.040 ms |

Keep for the repeatable patterned-rendering improvement: approximately 9% in
both phases. Solid/translucent steady scenes show no improvement (0.8–1.3%
slower in these medians). Allocation sizes and draw counts are unchanged.
Additional shader variants can increase loading cost; solid/translucent load
medians were 4–5% higher, while patterned loading was 6% lower in this run.

Mixed-opacity paint p50 is especially sensitive to slow frames: each phase
contains 20 opaque and 20 translucent frames, so the lower empirical median
lands near the slowest opaque frame, rather than a typical opaque frame.
Data-driven paint p50 ranges were 25.27–28.03 ms before and 25.49–28.09 ms after;
its +10.5% median change is not evidence of a repeatable regression or gain.
Raw p95 and each individual trial are retained for review.

Validation: 51 focused tests and all 50 solid renders/four queries pass.
Separate patterns remain 7/8 with the existing baseline failure. A new direct
comparison switches pattern presence and opacity on a persistent plugin layer
to exercise both shader variants and their cache identity.
[Raw captures](depth-only.json).
