# Native `contour` source for maplibre-native

**Status:** Draft
**Author:** Dave Brophy ([@dave](https://github.com/dave))
**Tracks:** [maplibre-style-spec#583](https://github.com/maplibre/maplibre-style-spec/issues/583)

## Motivation

MapLibre style authors who want elevation contour lines on a hillshade
basemap currently have to either:

1. Bake contours server-side into vector tiles and serve them as a vector
   source (extra storage + traffic; the DEM is already loaded for hillshade),
   or
2. Use [`maplibre-contour`](https://github.com/onthegomap/maplibre-contour),
   the existing TS prototype, which runs in `maplibre-gl-js` as a custom
   `addProtocol` callback. That works around the fact that gl-js's
   `source_cache.ts` doesn't support multiple consumers reading the same
   DEM tiles, by maintaining its own parallel DEM fetch path.

Neither option lands well in maplibre-native:

- Server-side baking duplicates the DEM data on disk and on the wire.
- The plugin pattern doesn't translate to the C++ renderer; there's no
  `addProtocol` equivalent for in-process tile generation.

In maplibre-native we control the tile pyramid directly, so a true
source-on-source contour generator is achievable: the DEM tiles loaded
for an existing `raster-dem` source can drive contour-line generation
in-process, with no extra fetches, by registering a tile-load listener.

This proposal implements the `contour` source type from
[maplibre-style-spec#583](https://github.com/maplibre/maplibre-style-spec/issues/583)
natively in maplibre-native's renderer, with a shared-pyramid
architecture matching the issue's stated rationale:

> One of the reasons for moving out of the plugin into maplibre would be to make use of the shared DEM tile cache between sources… terrain/hillshading/contours reading from the same tiles should be able share a source definition.
>
> — msbarry, in [#583](https://github.com/maplibre/maplibre-style-spec/issues/583)

## Proposed Change

### Style spec

A new source type `contour` with this shape:

```json
{
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["https://example.com/dem/{z}/{x}/{y}.png"],
      "encoding": "terrarium",
      "tileSize": 512,
      "maxzoom": 12
    },
    "contours": {
      "type": "contour",
      "source": "dem",
      "intervals": [200, 9, 100, 10, 50, 11, 20, 12, 10],
      "unit": "meters",
      "majorMultiplier": [5, 14, 4, 15, 5],
      "overzoom": 0
    }
  }
}
```

| Field | Type | Required | Description |
|---|---|---|---|
| `type` | string | yes | Always `"contour"`. |
| `source` | string | yes | ID of the upstream `raster-dem` source whose tile pyramid drives generation. |
| `intervals` | step-by-zoom number array | yes | Per-zoom contour interval in display units. Odd-length: `[output0, stop1, output1, stop2, …, outputN]`. |
| `unit` | string \| number | no, default `"meters"` | `"meters"`, `"feet"`, or a positive number used as a metres→display multiplier. |
| `majorMultiplier` | step-by-zoom positive-integer array | no, default `[5]` | Same odd-length step-by-zoom shape as `intervals`. Tags every Nth contour level as `major: true` at each zoom band. |
| `overzoom` | non-negative integer | no, default 0 | Levels of bilinear upsample to apply over the upstream DEM before running marching-squares (closes tile-edge seams when the contour source is rendered above the DEM's `maxzoom`). |

The source emits standard vector-tile features that the existing
`LineLayer` and `SymbolLayer` consume:

| Property | Type | Description |
|---|---|---|
| `ele` | number | Elevation of the contour line in display units. |
| `interval` | number | The contour interval used at this tile's zoom (display units). |
| `major` | bool | True if this line is a multiple of `majorMultiplier × interval`. |

(Implementations may emit additional engine-specific properties; the
above are the canonical spec.)

### Architecture

The render-side contour source borrows the upstream raster-dem source's
tile pyramid via a tile-load listener API. Concretely:

```
RenderRasterDEMSource (existing)
   ├── TilePyramid<RasterDEMTile>   ← single fetch path, single cache
   ├── consumer: HillshadeLayer (existing)
   └── consumer: RenderContourSource (new) — subscribes via addTileLoadListener

RenderContourSource (new, non-fetching)
   ├── TilePyramid<ContourTile>    ← derived geometry tiles, no network I/O
   ├── on DEM-tile-loaded (z,x,y) → run marching-squares → vector features
   └── exposed to LineLayer / SymbolLayer via the standard render-tile interface
```

Two new bits of plumbing make this work:

1. **`RenderRasterDEMSource::addTileLoadListener`** publishes per-tile-load
   events to cross-source consumers. Implemented as a `ListenerSet<>`
   (handle-based fan-out container — separate utility, also useful for
   future inter-source dependencies). The listener fires after the
   existing border-stitching logic for a renderable tile and synchronously
   replays already-loaded tiles when a new listener registers, to close
   the race between consumer-pyramid update and DEM tile arrivals.
2. **`TileParameters::getRenderSource`** is a `std::function<RenderSource*(const std::string&)>`
   populated by `RenderOrchestrator` and threaded into the per-frame
   `update()` calls. This is the first inter-source lookup mechanism in
   the renderer; it's deliberately minimal (just a pointer-by-id callback)
   and reusable for future source-on-source patterns.

`RenderContourSource::update()` queries the orchestrator for its
configured upstream raster-dem on the first call where the upstream has
been constructed, registers a listener, and unregisters in its
destructor.

### Algorithm

The marching-squares core lives at `src/mbgl/algorithm/contour/` as a
self-contained module with no MapLibre dependencies. Highlights:

- **Single-pass cell traversal** (from `maplibre-contour`'s `isolines.ts`):
  one sweep over the height grid emits all threshold levels for each
  cell, with incremental polyline stitching during traversal. ~5× fewer
  cell touches vs the per-level pass our prior server-side implementation
  used.
- **Saddle handling (cases 5 and 10)**: emit both segments of one fixed
  pairing rather than disambiguating per-cell via the corner mean. The
  fixed-pairing output is sufficient for typical DEM data where saddle
  points are sparse and visually subtle. Center-average disambiguation
  is a future improvement; the algorithm's interface accommodates it
  without breaking existing callers.
- **Tile-local int32 coordinates with configurable extent** (default 4096
  via `ContourThresholds::extent`).
- **Smoothing and simplification are a separate stage** applied by the
  contour-tile builder: Douglas-Peucker (half-tile-pixel tolerance) then
  Chaikin (2 iterations). Keeping these out of the algorithm core makes
  it easy to test against `maplibre-contour`'s raw output and to skip a
  stage entirely when a consumer wants un-smoothed output.

### Threading

Each contour tile snapshots its DEM heights synchronously on the render
thread (avoids racing with DEMData::backfillBorder, which mutates the
shared image as neighbour tiles arrive), then dispatches marching-squares
to the background scheduler. The algorithm reply lands back on the
render thread and calls `setData()` on the tile. A `WeakPtrFactory`
guards against the contour tile being destroyed (panned away) before
the reply arrives.

Empirically: <30 ms per 256×256 grid on Apple Silicon at 10 thresholds
with smoothing, well within the budget for live panning at 60 fps.

## API Modifications

### Public

- New `style::ContourSource` class at `include/mbgl/style/sources/contour_source.hpp`,
  matching the existing `Source` subclass pattern.
- New `style::ContourSourceOptions` struct on the same header.
- Style-spec parser recognises `"type": "contour"` and dispatches to
  `convertContourSource` in `src/mbgl/style/conversion/source.cpp`.

### Internal

- New `SourceType::Contour` enum value in `include/mbgl/style/types.hpp`.
- New `RenderContourSource` at `src/mbgl/renderer/sources/`.
- New `ContourTile` at `src/mbgl/tile/`.
- `RenderRasterDEMSource` gains the listener API.
- `TileParameters` gains the `getRenderSource` callback.
- `ListenerSet<>` utility added at `include/mbgl/util/listener_set.hpp`.

No existing public API breaks. The `getRenderSource` callback in
`TileParameters` is internal (not part of any SDK public surface).

## Migration Plan and Compatibility

This is a strictly additive change at the style-spec level:

- Existing styles continue to work unchanged.
- A style author opting in to the new source type adds it as a sibling
  source pointing at any existing `raster-dem` source. The new source
  contributes vector features that style filters on `["get", "ele"]` /
  `["get", "major"]` consume.
- The TS prototype (`maplibre-contour`) remains the gl-js path until
  gl-js gains shared-pyramid support; both implementations agree on the
  style-spec emit shape so the same style authoring works against
  either.

## Rejected Alternatives

### Loose-coupling via custom-protocol callback

Mirror the TS prototype: register a tile-URL pattern, fetch DEM tiles
ourselves under a separate cache, run the algorithm, return MVT bytes.

Rejected: defeats the main reason for going native. The whole point of
this work is to share the DEM tile pyramid that's already loaded for
hillshade. Loose coupling reproduces gl-js's known-broken-but-not-fixable
double-fetch architecture in a renderer where we can do better.

### Constructor-injection cross-source lookup

Pass `std::function<RenderSource*(const std::string&)>` to every
`RenderSource` subclass at construction time, stored as a member.

Rejected: changes 8+ existing constructor signatures with no clear
benefit over per-frame `TileParameters` injection. The lookup is only
needed during `update()` anyway, so threading it through that single
call site is the smaller blast radius.

### Multi-observer `TilePyramid`

Replace `TilePyramid::observer` with `std::vector<TileObserver*>` so
multiple consumers can subscribe directly to tile-load events at the
pyramid level.

Rejected: more invasive than needed. The `RenderRasterDEMSource` is
already the single observer of its pyramid (it forwards to its render
parent and does border-stitching); adding a fan-out side-channel on
the source rather than the pyramid keeps the change local to the one
type that has cross-source consumers, with no churn in `Tile`,
`TilePyramid`, or any other observer site.

## Testing

### Unit

- `algorithm/contour/isolines.test.cpp` — marching-squares cases,
  saddle disambiguation, multi-threshold single-pass emission, perf
  smoke check.
- `algorithm/contour/intervals.test.cpp` — per-zoom interval lookup,
  step-expression boundary semantics.
- `algorithm/contour/units.test.cpp` — meters / feet / custom
  multiplier round-trips.
- `algorithm/contour/smoothing.test.cpp` — Douglas-Peucker invariants
  (endpoint preservation, collinear drop, threshold strictness),
  Chaikin endpoint pinning + monotonicity preservation.
- `style/sources/contour_source.test.cpp` — class construction +
  getters, JSON parser happy + rejection paths, idempotent round-trip
  (parse → reconstruct via getters → re-parse → equal).
- `util/listener_set.test.cpp` — handle-based fan-out container.

### Integration

- Manual: deploy to iOS device with a representative DEM source +
  contour source + line/symbol layers, pan over real terrain, confirm:
  hard-coded line in the spike layer, then real contours, then
  smoothed contours, all rendering as expected.

### Cross-backend

(Not yet covered in this proposal — the implementation has been
verified on Metal only. Render-test fixtures across GL / Vulkan /
WebGPU are required before merge.)

## Disclosure

Significant portions of this proposal and the underlying implementation
were drafted with assistance from an AI coding agent (Anthropic's
Claude Opus 4.7) per the
[MapLibre AI Policy](https://github.com/maplibre/maplibre/blob/main/AI_POLICY.md).
All design decisions, code, and prose were reviewed and edited by the
human author; the algorithm's correctness was verified by the test
suite cited above plus on-device visual inspection.
