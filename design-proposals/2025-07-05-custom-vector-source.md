# Custom Vector Source

## Motivation

MapLibre Native currently supports two ways to provide vector tile data:

1. **URL-based sources** (`vector` source type) — tiles are fetched from a remote server via HTTP.
2. **CustomGeometrySource** — accepts GeoJSON features programmatically, but internally re-encodes them into vector
   tiles, adding overhead and limiting control over the tile encoding.

This proposal comes from the need for an efficient way to connect a proprietary data source.
That data source provides a gRPC service that has a simple API to request tiles by X/Y/Z (similarly to vector tiles) and the
format itself is pretty close to vector tiles.
CustomGeometrySource was considered but it has some obvious drawbacks:

1. Tile requests are in the format of a bounding box — since it's not a tile X/Y/Z it is pretty unclear what logic it
   conforms to.
    1. Is it just a tile X/Y/Z but converted to geo bounds?
    2. Can it ever request data in a bounding box that is bigger than just one X/Y/Z tile
    3. Additional boilerplate of mapping geo bounds into tile X/Y/Z
2. Internally CustomGeometrySource spawns a 4 thread threadpool and runs tasks one by one inside. It's a good old technique
   but Kotlin has things like coroutines which are a more ergonomic way to handle suspendable operations (like network
   requests).
3. CustomGeometrySource operates over GeoJSON. So in case of a vector tiles like format it needs to be converted to
   GeoJSON and then internally it will be cut down into vector tile format back.

## Proposed Change

Introduce a new source type, `CustomVectorSource`, that lets the application deliver raw binary tile data (starting with
MVT) directly to the rendering pipeline. The source participates in the standard tile loading lifecycle — the core
requests tiles by `CanonicalTileID`, and the application responds asynchronously with encoded data or an error.

### Architecture

```
┌─────────────────────────────────────────────────────────────┐
│  Platform layer (Kotlin)                                    │
│                                                             │
│  CustomVectorTileProvider.fetchTile(z, x, y) → TileData     │
│       ↕ (JNI / platform bridge)                             │
├─────────────────────────────────────────────────────────────┤
│  Core C++ layer                                             │
│                                                             │
│  CustomVectorSource                                         │
│    ├── Options { fetchTileFunction, cancelTileFunction,     │
│    │             zoomRange }                                │
│    ├── setTileData(CanonicalTileID, shared_ptr<string>,     │
│    │              TileDataFormat)                           │
│    ├── setTileError(CanonicalTileID, exception_ptr)         │
│    └── invalidateTile(CanonicalTileID)                      │
│                                                             │
│  CustomVectorTileLoader (Actor on Scheduler::GetBackground) │
│    └── Receives fetch/cancel requests from tiles            │
│                                                             │
│  CustomVectorTile (extends Tile)                            │
│    └── Parses MVT data via VectorTileData, delegates to     │
│        standard GeometryTileWorker for symbol/line/fill     │
│        layout                                               │
│                                                             │
│  RenderCustomVectorSource (extends RenderTileSetSource)     │
│    └── Standard tile pyramid rendering                      │
├─────────────────────────────────────────────────────────────┤
│  Existing rendering pipeline (unchanged)                    │
│    GeometryTileWorker → Buckets → Drawables → GPU           │
└─────────────────────────────────────────────────────────────┘
```

### Key Design Decisions

1. **Binary data, not GeoJSON.** For the Kotlin layer to send data to C++, it needs to be marshalled into some
   data format that can be passed across languages. One could come up with a new binary format, implement an encoder in Kotlin
   and a decoder in C++. But it turns out MVT is already such a format — just a binary blob that can be passed simply
   as a sized buffer. Existing infrastructure of `MVT` tile data is just reused as is.

2. **Actor-based concurrency.** `CustomVectorTileLoader` runs on a background thread via MapLibre's Actor system. The
   platform callback (fetch/cancel) is invoked on this thread; the platform layer is responsible for dispatching to its
   own preferred context (e.g., coroutines on Android).

3. **Cooperative cancellation.** When a tile is no longer needed (e.g., user panned away), the core calls
   `cancelTileFunction`. The platform layer can abort in-flight work.

4. **Standard tile pipeline reuse.** Once binary data arrives, it goes through the same `VectorTileData` →
   `GeometryTileWorker` path as HTTP-fetched vector tiles. No custom rendering code is needed.

5. **Extensible format enum.** `TileDataFormat` is an enum starting with `MVT`. Future formats (e.g., a hypothetical
   compact binary format) can be added without API changes. And obvious next candidate is `MLT` format support in
   CustomVectorSource.

### Threading Model

```
Main thread:     Source::update() → tile needs data
                         ↓
Background Actor: CustomVectorTileLoader::fetchTile()
                         ↓
Platform bridge:  fetchTileFunction callback → platform code
                         ↓ (async, any thread)
                  setTileData() → mailbox → CustomVectorTile::setData()
                         ↓
GeometryTileWorker: parse MVT, create buckets (background pool)
                         ↓
Main thread:     Tile ready → render
```

## API Modifications

### Core C++ (public header)

```cpp
namespace mbgl::style {

enum class TileDataFormat : uint8_t { MVT = 0 };

using TileFunction = std::function<void(const CanonicalTileID&)>;

class CustomVectorSource final : public Source {
public:
    struct Options {
        TileFunction fetchTileFunction;
        TileFunction cancelTileFunction;
        Range<uint8_t> zoomRange = {0, 18};
    };

    CustomVectorSource(std::string id, const Options& options);

    void setTileData(const CanonicalTileID&,
                     const std::shared_ptr<const std::string>& data,
                     TileDataFormat format = TileDataFormat::MVT);
    void setTileError(const CanonicalTileID&, std::exception_ptr error);
    void invalidateTile(const CanonicalTileID&);
};

} // namespace mbgl::style
```

### Android (Kotlin)

```kotlin
interface CustomVectorTileProvider {
    suspend fun fetchTile(z: Int, x: Int, y: Int): TileData
}

sealed class TileData {
    class Mvt(val data: ByteArray) : TileData()
}

class CustomVectorSource(
    id: String,
    provider: CustomVectorTileProvider,
    scope: CoroutineScope,
    minZoom: Int = 0,
    maxZoom: Int = 18
) : Source()
```

The Android API uses Kotlin coroutines for async tile fetching. The `CoroutineScope` is caller-provided. Meaning
`CustomVectorSource` can run on a threadpool but not limited to a threadpool. `CoroutineScope` abstracts it away.

### iOS / macOS

Not included in this initial implementation. The core C++ API is platform-agnostic and can be wrapped in Swift/Obj-C
following the same pattern.

## Migration Plan and Compatibility

All changes are **additive**. No existing APIs are modified or removed. The new source type is opt-in — applications
that don't use it are unaffected.

- A new `SourceType::CustomMVTVector` enum value is added to the core.
- Existing `CustomGeometrySource` remains unchanged and is not deprecated.

## Rejected Alternatives

### Extend CustomGeometrySource to accept binary data

`CustomGeometrySource` is designed around GeoJSON features and internally manages geometry encoding. Bolting binary tile
support onto it would conflate two different data models and complicate its already non-trivial implementation.

### Use a local file source with a custom URL scheme

This approach (e.g., `mbtiles://` or `custom://`) requires the application to write tile data to disk or implement a
custom `FileSource`. It adds I/O overhead, doesn't support cooperative cancellation, and is harder to integrate with
structured concurrency frameworks like Kotlin coroutines.
