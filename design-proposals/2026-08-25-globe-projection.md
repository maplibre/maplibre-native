# Globe Projection Design Proposal

Related issue: [#3161](https://github.com/maplibre/maplibre-native/issues/3161).

## Motivation

MapLibre GL JS has had a globe since 2024: a `vertical-perspective` projection, `mercator`, and a `globe` preset that blends between them by zoom. The style spec defines it as a root-level `projection` property of type `projectionDefinition`, and that definition is already in this repo's `scripts/style-spec-reference/v8.json`. Native ignores it. A style that renders as a globe on the web renders flat on iOS, Android and Qt, with no error and no way to ask for anything else.

#3161 has been open since January 2025 with the `js-parity` label. Nobody has picked it up because it is big. Steve's estimate was at least six figures, and his point about mobile stands: phone users are more interactive than web users, so corner cases GL JS could ship and fix later will get Native an earful. In April 2026 a sponsor surfaced and the org started looking for a second. What the thread still lacks is a plan: which files change, in what order, what each step costs, and what "done" means on a phone.

This is that plan. The first step is mergeable with no funding and no behavior change. Each later step is a small PR against a seam the previous one created. Sponsors can see where the money goes before committing it. It extends the Drawable/tweaker architecture from the 2022 rendering-modularization proposal; nothing here replaces it.

The reference implementation is GL JS. Its projection code is portable by construction (the `Projection` / `Transform` split, a shader prelude with identical function signatures for both projections, the subdivision pass), and its commit history is a checklist of what breaks after the first "final" PR. This proposal maps that design onto Native's files and adds what Native needs that GL JS did not: four render backends, a C++ tile worker, and platform SDKs.

## Proposed Change

### What GL JS built, and what Native has today

GL JS solved the globe with five seams:

1. `Projection`: per-map, state-free, owns GPU resources and the shader variant (`geo/projection/projection.ts`).
2. `ITransform`: project, unproject, tile matrix, covering-tile bounding volumes. Three implementations: `mercator_transform.ts`, `vertical_perspective_transform.ts`, and `globe_transform.ts`, which holds one of each plus a `_globeness` scalar and blends.
3. `ProjectionData`: the per-tile uniform contract (`mainMatrix`, `tileMercatorCoords`, `clippingPlane`, `projectionTransition`, `fallbackMatrix`, `clipAntimeridian`). The only thing a layer shader knows about the projection.
4. `ICameraHelper`: pan, zoom, pitch, `easeTo`, `flyTo` on a sphere versus a plane.
5. `CoveringTilesDetailsProvider`: distance-to-tile, wrap, bounding volume, per projection.

Plus a shader prelude (`_projection_globe.vertex.glsl` / `_projection_mercator.vertex.glsl`) exposing `projectTile()` with the same signature in both, so no layer shader knows which projection is active, and a subdivision pass (`render/subdivision.ts`) that splits long edges so the vertex shader can bend them onto the sphere.

Native, at `1d0908bd` (2026-08-25), has none of the five. `include/mln/util/projection.hpp:42` is a static Mercator utility with 72 statically bound call sites across 13 files. `TransformState::matrixFor` (`src/mln/map/transform_state.cpp:115`) is a 2D affine. `screenCoordinateToTileCoordinate` (`:778`) solves a ray against the plane `z = 0`. `Transform::moveBy` (`src/mln/map/transform.cpp:406`) pans by unprojecting a screen point. `tileCover` (`src/mln/util/tile_cover.cpp:158`) culls a quadtree of flat `AABB`s against a `Frustum`. `LayerTweaker::getTileMatrix` (`src/mln/renderer/layer_tweaker.cpp:28`) hands each drawable one `mat4`, and that `mat4` is the entire projection contract to the shaders. `CollisionIndex::projectTileBoundaries` (`src/mln/text/collision_index.cpp:89`) projects four tile corners and treats the result as an axis-aligned rectangle. There is no subdivision (`fill_generator.cpp` hands raw rings to earcut). The style parser (`src/mln/style/parser.cpp`) reads `light`, `roll` and `transition` at the root, not `projection`.

What Native does have: the Drawable/tweaker split puts the projection contract on exactly one boundary (`LayerTweaker` → per-drawable UBO). All four backends already assemble every shader from a shared prelude (`shaders/_prelude.vertex.glsl`, `include/mln/shaders/mtl/common.hpp`, the Vulkan and WebGPU `common.hpp`). Tiles are parsed off the render thread in `GeometryTileWorker`. Fill and line geometry flow through two shared generators (`src/mln/gfx/fill_generator.cpp`, `polyline_generator.cpp`) rather than nine separate buckets. Platform gesture code passes deltas into `Transform::moveBy` / `rotateBy` / `easeTo` and does no projection math of its own. `roll` and `FreeCameraOptions` already exist on the transform.

### Design

The GL JS design, with the seams where Native's structure puts them.

**Projection strategy.** A new abstract class, `ProjectionBase`, owned by `TransformState`, one per map, holding no camera state. It folds GL JS's `Projection` and the projection half of its `Transform` into one object; `TransformState` keeps the camera state. Concrete names follow GL JS's files. `mln::Projection` (the static Mercator math) and `ProjectionMode` (the axonometric switch) are untouched. It answers: project / unproject a `LatLng`, tile matrix for an `UnwrappedTileID`, ray direction from a pixel, is-point-on-surface, `projectTileCoordinates` (point, signed camera distance, occlusion), covering-tile bounding volume and wrap rules, subdivision granularity, and `getProjectionData(tileID, aligned)`. Three implementations: `MercatorProjection` (today's behavior, moved), `VerticalPerspectiveProjection` (the port of `vertical_perspective_transform.ts` and `globe_utils.ts`), and `GlobeProjection`, which owns one of each and a transition scalar and delegates or lerps, as `globe_transform.ts` does. The static `mln::Projection` utility stays; the Mercator implementation calls it.

**`ProjectionData` through the tweakers.** A per-tile struct with GL JS's fields: main matrix, tile Mercator coordinates, clipping plane, transition, fallback matrix, antimeridian clip flag. `LayerTweaker::getTileMatrix` becomes `getProjectionData`, and each `*DrawableUBO` grows a projection block (under `MLN_UBO_CONSOLIDATION`, the per-drawable array element does). Under Mercator the main matrix is today's matrix and every other field is inert. That is what makes the first PR a no-op for the render tests.

**Shader prelude per backend.** Each backend's shared prelude gains `projectTile(vec2)`, `projectTile(vec2, vec2 rawPos)`, `projectTileWithElevation`, `projectTileFor3D`, `projectLineThickness` and `circumferenceRatioAtTileY`, taking a projection block; the Mercator variant reduces to `matrix * vec4(pos, 0, 1)`. The globe variant ports `_projection_globe.vertex.glsl`: mercator-to-sphere via the half-angle substitution (no `atan`/`sin`/`cos`, for float32 precision), pole detection by the ±32767 sentinel, a custom clip-space Z from the clipping plane so back-of-planet geometry is discarded, and `interpolateProjection` mixing globe and fallback by the transition scalar. Which variant a program gets is a shader-manifest key, the way GL JS keys its program cache by `shaderVariantName`. Layer shaders change one line each, `matrix * pos` to `projectTile(pos)`, on all four backends in the same PR, one layer family at a time. The prelude is shared code, so it lands on all four backends together; the Mercator variant is behavior-identical, which makes that cheap to do first.

**Subdivision in the tile worker.** Long edges must be split before the vertex shader can bend them. GL JS hooked four buckets and symbol layout. Native's fill and line geometry already pass through `fill_generator.cpp` and `polyline_generator.cpp`, so the subdivision pass goes at the generator boundary (`subdividePolygon`, `subdivideVertexLine`, `fixWindingOrder`, `scanlineTriangulateVertexRing`, ported from `render/subdivision.ts`), with `FillExtrusionBucket` (its own earcut) and `CircleBucket` (`circle-pitch-alignment: map`, heatmap) hooked separately. Granularity is owned by the projection (GL JS globe values: fill 128, line 512, tile 128, stencil 128, circle 3, halved per zoom; Mercator: none) and travels into `GeometryTileWorker` with the layer properties it already receives, so a projection change reloads tiles the way a layout-property change does. Tile-covering meshes with pole caps for raster, hillshade, color-relief and the stencil clip masks port from `util/create_tile_mesh.ts` and replace the fixed quad.

**Transform and gestures.** `screenCoordinateToTileCoordinate` and `screenCoordinateToLatLng` route through the projection: ray-sphere intersection with the horizon clamp from `vertical_perspective_transform.ts:908` on the globe, the existing `z = 0` solve on Mercator. `Transform::moveBy` and `TransformState::moveLatLng` get a spherical drag (rotate about the earth's center, GL JS `computeGlobePanCenter`) behind the same seam. `easeTo` and `flyTo` interpolate in the projection's space. `constrain` stops applying Mercator bounds when the globe is active (GL JS #5186). The iOS and Android gesture handlers don't change; they already pass deltas.

**Tile cover.** `tileCover` keeps its quadtree DFS and its `Frustum`, and asks the projection for the node's bounding volume: today's `AABB` on Mercator, a convex hull of sphere-projected corners on the globe, double-buffered per frame as GL JS does, plus a horizon plane test. Wrap logic (nearest of three copies across the antimeridian, mirror across the poles) and `allowWorldCopies = false` come from `globe_covering_tiles_details_provider.ts`. `update_tile_masks.hpp` keeps working in ID space.

**Symbols and collision.** `CollisionIndex::projectTileBoundaries` and the `posMatrix`/`labelPlaneMatrix` path in `symbol_projection.cpp` and `placement.cpp` switch to `projectTileCoordinates`, which returns occlusion. Occluded symbols are hidden even under `overlap: always`, boxes get the pitched-text correction, and the Mercator path keeps the fast single-matrix route. GL JS needed a dedicated symbols PR (#4071) while the globe branch was still on its way in, months before the "final" PR merged. It is its own phase here.

**The blend.** `globe` is a style expression, `["interpolate", ["linear"], ["zoom"], 11, "vertical-perspective", 12, "mercator"]`, evaluated like any other zoom expression; the transition scalar feeds `GlobeProjection` every frame. The fallback matrix and the antimeridian fragment clip make the crossfade seamless. Style code plus one scalar, so it comes late in the order.

**Custom layers.** `CustomLayerRenderParameters` gains the projection block (transition, tile-to-Mercator, clipping plane, fallback), the equivalent of GL JS `getProjectionDataForCustomLayer`; the Metal and Vulkan subclasses inherit it. Until that lands, a custom layer on a globe style draws in Mercator space over a sphere: wrong, but not a crash, and the release notes will say so.

**Style and API.** A `projection` root property parsed next to `light`, generated by `scripts/generate-style-code.mjs` from the existing spec entry the way `light.hpp.ejs` is, with a `ProjectionDefinition` expression type (string, `[from, to, transition]` tuple, or expression) added to `include/mln/style/expression/type.hpp`. `Style::getProjection()` / `setProjection()` mirror `getLight()` / `setLight()`.

### Phased plan

Every phase is a PR (or a short series) that leaves `main` shippable, with the render tests as the oracle.

| Phase | Scope | Mergeable alone | Oracle |
|---|---|---|---|
| 1 | Seam: `ProjectionBase` + `ProjectionData` through `LayerTweaker` into the UBOs, Mercator only, all four backends | Yes, no behavior change | Every existing render test, every manifest, pixel-identical (`allowed` unchanged) |
| 2 | Prelude functions on all four backends (Mercator variant only); layer shaders switched to `projectTile` one family at a time | Yes, no behavior change | Same |
| 3 | `projection` root property: parser, codegen, expression type, `Style` API, transition scalar plumbed to the (still Mercator) transform | Yes, parsed and ignored by the renderer | Unit tests on parser and expression; render tests unchanged |
| 4 | Globe prelude variant + `VerticalPerspectiveProjection` project/tile-matrix on **Metal**, fill and background only, no subdivision | Yes, behind the style property | First ported GL JS globe render tests (`fill-planet-solid`, `background`) on the macOS Metal manifest; the visible tile seams are the argument for phase 5 |
| 5 | Subdivision at the generators, tile meshes with pole caps, granularity through the worker | Yes | `fill-planet-tiles`, `fill-seams`, `fill-planet-pole`, `raster-planet`, `raster-pole` |
| 6 | Unproject, gestures, `easeTo`/`flyTo`, constrain | Yes | Unit tests on `TransformState` (ray-sphere, horizon clamp, pan), the GLFW app by hand, video in the PR |
| 7 | Tile cover: convex volumes, horizon plane, wrap, no world copies | Yes | `antimeridian-lod`, `antimeridian-overdraw`, tile-count assertions in `tile_cover.test.cpp` |
| 8 | Symbols and collision | Yes | The 12 `collision-*` / `text-*` / `icon-text-*` globe tests |
| 9 | Remaining layer families on Metal (line, circle, heatmap, fill-extrusion, hillshade, color-relief, raster), the blend, custom-layer parameters | One PR per family | The remaining globe tests, `zoom-transition`, `custom` |
| 10 | OpenGL, Vulkan, WebGPU globe prelude variants | One PR per backend | Same tests on the Linux GL, Android Vulkan and WebGPU manifests |
| 11 | Platform SDK: `MLNStyle.projection`, Android `Style.setProjection`, GLFW and test-app demos, Qt tracking issue | Per platform | Platform tests, docs with real-code includes |
| 12 | The corrective tail | Many small PRs | GL JS's list, below |

Metal first for the globe itself: the macOS Metal preset is the one that builds and runs the render tests on a developer machine and in CI (`metrics/macos-xcode11-release-style.json`), iOS is the sponsor's platform, and OpenGL doesn't build on macOS. The Linux OpenGL CI leg is the GL witness from phase 1 onward, since phases 1 to 3 touch every backend. GL first (simplest backend) would lose the local dev loop on the machines the maintainers use.

### Test strategy

Phases 1 to 3 are held to pixel-identical output on every manifest. The diff to `expected.png` files is zero; any nonzero diff is a bug in the seam, not a new expectation. From phase 4 the 45 GL JS globe render tests under `test/integration/render/tests/projection/globe/` are ported one directory at a time into `metrics/integration/render-tests/projection/` (which already exists for axonometric and perspective), with GL JS assets attributed in `metrics/integration/tiles/README.md`, platform expectations under `metrics/expectations/platform-*` where backends legitimately differ, and ignores only with a linked reason. Transform math gets unit tests in `test/map/transform.test.cpp` and `test/util/tile_cover.test.cpp` that fail before and pass after. Gestures get the GLFW app and a video; there is no automated oracle for how a drag feels.

### Scope and cost

What the "final" globe PR leaves undone, from GL JS's own history (`git log --reverse -- src/geo/projection`): around and after the globe PR (#3963, merged 2024-09-30) came symbol translation and collision fixes (#4071), symbol and covering-tile optimizations (#4778), a white-flash fix (#4845), camera roll (#4780), pitch beyond 90° (#4851), terrain-on-globe (#4825), LOD at high pitch (#4779), covering-tiles optimization (#4937), the Mercator/vertical-perspective split that made `globe` a blend (#5023), tile border fixes (#4868), not constraining with the Mercator transform on the globe (#5186), LOD control (#5719), clamping unproject to the horizon (#5771), improved frustum culling (#5865), constrain-on-projection-change (#6917), a distance-to-tile fix (#7234), and max zoom for vertical perspective (#7372, 2026-04-02). Eighteen PRs across roughly two years, several by people other than the original author. This proposal front-loads the ones that are design decisions (the VP/globe split, occlusion in collision, horizon clamp, no constrain on globe) and budgets the rest as phase 12. Some of that tail already exists on Native for other reasons (`roll` is parsed and stored on the transform); most of it does not.

Backend multiplier: the globe-specific shader code is written four times (one prelude variant per backend, a few hundred lines each), and the per-layer one-line switch is done 4 × ~20 times as mechanical work. That is the price of no `#ifdef`s in shared headers and every backend compiling on CI, and it is far below 4 × 20 shader rewrites. WebGPU is the least mature backend with the flakiest render-test leg; it goes last.

Where a web-quality globe will get Native an earful: gestures (a drag that doesn't track the finger on a sphere is obvious on a phone and merely odd with a mouse), tile loading at the horizon and poles (memory on a low-end Android device is the constraint, not GPU), and symbol placement at the limb. Phases 6, 7 and 8 are sized for that. The plan also budgets a bug-squishing pass several months after the first release ships in a real app, as the Metal proposal recommended.

Not included: a WGS84 ellipsoid, geodesic line rendering, and a camera that works at satellite altitudes. GL JS's globe is a unit sphere in Mercator space and this port keeps that. Those three are what "a properly curved earth" for aviation or drone work means beyond parity. The `ProjectionBase` seam is where an ellipsoid model would plug in; they are a second engagement.

Terrain doesn't exist on Native and this proposal doesn't depend on it. `ProjectionData` carries the elevation entry points (`projectTileWithElevation`) so a terrain implementation wouldn't have to reopen the seam.

## API Modifications

Additive only; a minor release.

- Style: the spec's root-level `projection` property (`"mercator"` default, `"vertical-perspective"`, `"globe"`, a `[from, to, transition]` tuple, or a zoom expression) is parsed and honored. The spec already defines it; no style-spec repo change is needed.
- Core: `style::Style::getProjection()` / `setProjection(ProjectionDefinition)` alongside `getLight()` / `setLight()`. `CustomLayerRenderParameters` gains projection fields, appended after the existing ones. Source-compatible, ABI-breaking for custom-layer hosts built against older headers. Phase 9 either rides a major release or puts the fields behind a pointer member; that PR says which.
- Binary size: one extra prelude string per backend and one extra compiled variant per program on the backends that compile at build time. Expected in the low tens of KB per backend; each shader-touching phase reports the `pr-bloaty-ios.yml` delta in its PR body.
- iOS/macOS: `MLNStyle.projection`, mirroring `MLNStyle.light`, plus an `MLNProjectionDefinition` value type.
- Android: `Style.getProjection()` / `setProjection(ProjectionDefinition)`, mirroring `getLight()` / `setLight()`.
- Qt and Node: tracking issues; the core work is the same, the bindings are small.
- No map-view-level projection setter. GL JS drives it from the style; a second setter would be a second source of truth.

## Migration Plan and Compatibility

No existing behavior changes. A style without `projection` renders as before; phases 1 to 3 are verified pixel-identical. Styles that already carry `projection` for GL JS start rendering as globes on Native at the release that ships phase 4, which is the point of the `js-parity` label; an app that must keep a flat map for such a style sets `projection` to `"mercator"` at load via the new setter. Custom layers on globe styles draw in Mercator space until phase 9; the phase 4 release notes say so. `ProjectionMode` (axonometric skew) is untouched, composes with Mercator only, and is a documented no-op on the globe.

## Rejected Alternatives

**Globe as a custom layer or plugin.** It can't own gestures, tile cover, unprojection, or symbol collision; it would be a picture of a globe the map doesn't know about. The 2025 plugin-layers proposal covers representations of existing geometry, not a different transform.

**A "fake globe" at low zoom only.** Warp the Mercator world onto a sphere below zoom 3 and switch to flat above. Skips subdivision, gestures on the sphere, and symbols, and each of those is the bug report the day it ships. GL JS's adaptive blend exists for this case and still needed the vertical-perspective transform underneath it.

**Skip subdivision and rely on high zoom.** Tile edges chord across the sphere at zoom 0 to 4, the only range where the globe is visible. Phase 4 ships without it on purpose so the render test shows why phase 5 exists.

**All four backends at once for the globe variant.** The seam and the Mercator prelude go to all four together because they are behavior-identical and cheap. The globe variant is a few hundred lines of shader math per backend and is better landed and eyeballed one backend at a time with the render tests green in between. No backend-specific `#ifdef`s in shared code at any point; variant selection is a manifest key.

**Terrain first.** GL JS did terrain first and then spent #4825 and #4868 reconciling it with the globe. Native has no terrain, so the globe can take the cleaner order; the elevation entry points in the prelude are the concession to a future terrain implementation.

**Extending `mln::Projection` (the static utility) into the strategy.** It is a namespace of Mercator math used in 13 files and by platform code; making it virtual would touch every call site for no gain. The strategy is a new type; the utility stays as the Mercator implementation's math.
