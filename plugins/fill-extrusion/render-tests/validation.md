# Solid milestone validation

Environment: fresh Release build, Clang 21, Nixpkgs
`a32edd7654519351e48e80372a928df336394670`, Mesa software Vulkan (lavapipe).
GLFW, X11, and Wayland disabled. The local Nix flake and lock stay uncommitted.
The workspace's Vulkan-Headers checkout predates the core's expected API;
testing uses the repository-pinned `015e25c3c91b70eb1a754d36fb14c4ba6ad9b0b9`
headers extracted into `/tmp`, preserving the user's checkout.

| Milestone | Built-in render passes | Plugin render passes | Query passes |
| --- | ---: | ---: | ---: |
| Initial, empty cache | 44/50 | 43/50 | 4/4 each |
| Depth-only color mask corrected | 44/50 | 44/50 | 4/4 each |
| Existing offline cache used | 50/50 | 50/50 | 4/4 each |

The initial cache omitted sprite/raster resources: four image failures and two
resource errors affected both executables. All six disappear using the existing
repository cache. Final baseline failures: zero.

The 72 render candidates comprise 50 eligible solids, 13 existing ignored solids,
one existing JS-only skip, and eight deferred pattern fixtures. The manifest also
covers 15 footprint-query fixtures: four eligible and 11 existing ignores.
One ignored render fixture passes with the plugin; its ignore remains unchanged.

The fresh plugins-disabled Vulkan render runner also builds successfully.
All 40 focused API/geometry/parity tests pass.

The n-gon unit executable and all 13 n-gon render fixtures pass. Focused checks
cover replacement conflicts/identity, boolean parsing and expressions, malformed
descriptors, callback suppression/recovery, polygon holes, segmentation past
65,535 vertices, opacity changes through zero/partial/one, translation anchors,
light changes, vertical gradient, and feature-state updates. Direct comparison
scenes retain an existing built-in object across plugin registration.

## Rounded corners

Separate direct comparisons cover radii 0, 1, 12, 1000 (limited by edge length),
and a zoom expression, each at opacity 0.5 and 1. The scene includes holes and
multiple overlapping features across tile boundaries. All ten comparisons pass.
The plugin reproduces the circular arcs, three intermediate arc samples,
five-degree straight-edge cutoff, 20% edge-length limit, float tessellation
coordinates, and 1/128 tile-unit GPU quantization used by the native layer.
Layout scope/serialization/invalidation and invalid descriptors are checked
separately. All 42 focused tests pass, as do all 50 eligible solids, four eligible queries,
and all 13 n-gon fixtures. The original solid suite remains unchanged.

## Patterns

Separate eight-fixture baseline: built-in 7/8, plugin 7/8, with no ignores.
The existing `fill-extrusion-pattern/tile-buffer` expectation fails for both;
their actual PNG files are byte-identical (SHA-256
`a4c6cc019cb2dac903c52124d9df5e52c925948a279582eeb3b894a83b597b7d`).
No expectation, threshold, or ignore was changed.

Direct comparisons cover seven pattern cases at four zooms and two corner radii
(56 comparisons), changing opacity and gradient at runtime. Cases include
literal sprites at image pixel ratio 2, explicit empty and undefined patterns,
feature/composite expressions, and image/coalesce fallback. Additional checks
cover image serialization, availability, discrete transitions, and malformed
atlas descriptors. Composite sampling follows the native next-zoom `to` value.

The native layer has an additional runtime limitation: replacing a constant
pattern with a sprite absent from the tile atlas can make the extrusion disappear
because it does not request a new layout. The plugin requests new dependencies
for this change. Direct parity scenes start with each pattern to avoid treating
that existing defect as required behavior.

Pattern milestone regression totals: 45/45 focused tests, 50/50 eligible solid
renders, 4/4 eligible queries, and 13/13 n-gon renders. Both implementations pass
all seven applicable pattern expectations; the eighth has the identical existing
baseline failure described above.

## Performance changes

After packing geometry to 12 bytes per vertex, sharing roof vertices within
16-bit segments, and skipping state-independent paint uploads, the same 45
focused tests, 50 eligible solid renders, four queries, and 13 n-gon renders
pass. Pattern results remain 7/8 with the byte-identical existing baseline
failure. The plugins-disabled Vulkan runner builds and the public API header
passes a C11 syntax check. See [benchmark results](../benchmarks/README.md).

## Instanced walls

Walls now use one generated quad per outline edge. Indexed roofs and instanced
walls share eight-byte outline records and identical feature paint buffers.
The generic Vulkan binding path now honors starting-record offsets when two
attributes read different records from the same GPU allocation.

All 49 focused tests pass. Added coverage checks Vulkan-only instancing
registration, copied descriptor ownership, shifted record/instance bounds,
sharing paint only for identical feature mappings, and a 70,000-point outline
with instanced walls and safely segmented roof indices. Existing runtime
comparisons cover opacity, translation, light, gradient, feature-state, ten
rounded cases, and 56 patterned cases using the new instance path.

The built-in and plugin executables were rerun separately: 50/50 eligible solid
renders and 4/4 queries each. The n-gon unit executable and 13/13 renders pass.
Pattern results remain 7/8 each; the actual `tile-buffer` PNG hashes remain
identical to the SHA-256 recorded above. Expectations, tolerances, and ignores
remain unchanged. The four direct host tests also pass with
`VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`, with no validation errors
reported. The plugins-disabled Vulkan runner builds and the public header
passes C11 syntax checking. See the fresh
[benchmark results](../benchmarks/README.md).


## Subsequent optimization experiments

Identical paint endpoint sharing, depth-only shader specialization, normalized
byte colors, and separate scalar endpoint attributes were measured independently
and retained in separate commits. All 55 focused tests pass. The final direct
comparisons update base/height/color through feature state together and switch
them to composite expressions at fractional zooms. All six host comparison tests
also pass with the Khronos Vulkan validation layer and no validation errors.

Final fixture totals remain 50/50 eligible solids, 4/4 queries, 13/13 n-gon renders,
and 7/8 patterns. The known pattern failure retains the same built-in actual PNG
hash recorded above. The n-gon unit executable, plugins-disabled Vulkan build,
and C11 header check pass. See the [individual experiments and raw captures](../benchmarks/experiments.md),
including the longer investigation of an apparent translucent timing regression.
