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
