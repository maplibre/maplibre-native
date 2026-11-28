#include <mln/test/stub_file_source.hpp>
#include <mln/test/stub_render_source_observer.hpp>
#include <mln/test/util.hpp>

#include <mln/annotation/annotation_manager.hpp>
#include <mln/gfx/dynamic_texture_atlas.hpp>
#include <mln/map/transform.hpp>
#include <mln/renderer/dem_elevation_provider.hpp>
#include <mln/renderer/image_manager.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/renderer/render_source.hpp>
#include <mln/renderer/tile_parameters.hpp>
#include <mln/style/layers/hillshade_layer.hpp>
#include <mln/style/layers/hillshade_layer_impl.hpp>
#include <mln/style/layers/line_layer.hpp>
#include <mln/style/layers/line_layer_impl.hpp>
#include <mln/style/sources/raster_dem_source.hpp>
#include <mln/style/sources/vector_source.hpp>
#include <mln/style/style.hpp>
#include <mln/text/glyph_manager.hpp>
#include <mln/util/image.hpp>
#include <mln/util/logging.hpp>
#include <mln/util/projection.hpp>
#include <mln/util/run_loop.hpp>
#include <mln/util/tile_coordinate.hpp>
#include <mln/util/tile_cover.hpp>
#include <mln/util/timer.hpp>

#include <algorithm>
#include <map>

using namespace mln;
using namespace mln::style;

namespace mln {
template <class T>
void PrintTo(const Range<T>& range, std::ostream* os) {
    *os << "[" << range.min << ", " << range.max << "]";
}
} // namespace mln

namespace {

constexpr auto demURL = "dem/{z}/{x}/{y}";
constexpr auto vectorURL = "vector/{z}/{x}/{y}";
constexpr uint8_t maxZoom = 15;

// A Terrain-RGB tile (-10000 + (R * 65536 + G * 256 + B) * 0.1 meters) whose lowest point is
// `low` and highest `high`.
std::string demTile(int32_t low, int32_t high) {
    constexpr uint32_t size = 16;
    PremultipliedImage image({size, size});
    for (uint32_t i = 0; i < size * size; ++i) {
        const auto value = static_cast<uint32_t>(((i == 0 ? high : low) + 10000) * 10);
        uint8_t* pixel = image.data.get() + i * 4;
        pixel[0] = static_cast<uint8_t>(value >> 16);
        pixel[1] = static_cast<uint8_t>(value >> 8);
        pixel[2] = static_cast<uint8_t>(value);
        pixel[3] = 255;
    }
    return encodePNG(image);
}

bool covers(const CanonicalTileID& tile, const CanonicalTileID& other) {
    return tile == other || other.isChildOf(tile);
}

bool overlaps(const CanonicalTileID& a, const CanonicalTileID& b) {
    return covers(a, b) || covers(b, a);
}

// The range spanning `low` and `high`, as DEMElevationProvider reports it.
Range<double> span(const Range<int32_t>& low, const Range<int32_t>& high) {
    return {static_cast<double>(low.min), static_cast<double>(high.max)};
}

// The same elevation range everywhere.
class ConstantElevation : public util::TileElevationProvider {
public:
    explicit ConstantElevation(Range<double> range_)
        : range(range_) {}
    std::optional<Range<double>> getTileElevationRange(const CanonicalTileID&) const override { return range; }

private:
    Range<double> range;
};

// A raster-dem source with terrain, and a vector source draped over it, updated frame by frame
// the way RenderOrchestrator::createRenderTree does.
class TerrainSourcesTest {
public:
    util::SimpleIdentity uniqueID;
    util::RunLoop loop;
    // Synchronous, so that each call of tileResponse is one request: the asynchronous stub asks
    // again every millisecond for as long as a response is withheld.
    std::shared_ptr<StubFileSource> fileSource = std::make_shared<StubFileSource>(
        StubFileSource::ResponseType::Synchronous);
    StubRenderSourceObserver observer;
    Transform transform;
    TransformState state;
    AnnotationManager annotationManager{style};
    std::shared_ptr<ImageManager> imageManager = ImageManager::create();
    std::shared_ptr<GlyphManager> glyphManager = std::make_shared<GlyphManager>();
    gfx::DynamicTextureAtlasPtr dynamicTextureAtlas;
    TaggedScheduler threadPool;
    style::Style style;

    RasterDEMSource demSource{"dem", Tileset{{demURL}, {0, maxZoom}}, util::tileSize_I};
    VectorSource vectorSource{"vector", Tileset{{vectorURL}, {0, maxZoom}}};
    std::vector<Immutable<LayerProperties>> demLayers;
    std::vector<Immutable<LayerProperties>> vectorLayers;
    /// What the DEM render source is updated with, as the style's source changes.
    Immutable<style::Source::Impl> demImpl = demSource.baseImpl;
    std::unique_ptr<RenderSource> demRenderSource;
    std::unique_ptr<RenderSource> vectorRenderSource;

    /// The DEM served for a tile, as {lowest, highest} meters.
    std::function<Range<int32_t>(const CanonicalTileID&)> elevationOf = [](const CanonicalTileID&) {
        return Range<int32_t>{0, 0};
    };
    /// A DEM tile whose request is never answered, as if it were still on its way.
    std::optional<CanonicalTileID> withheldDEMTile;
    /// When set, only DEM tiles of this zoom are answered, so that no tile of another zoom has a
    /// range of its own.
    std::optional<uint8_t> servedDEMZoom;
    /// Requests by tile URL template and tile.
    std::map<std::pair<std::string, CanonicalTileID>, int> requests;

    TerrainSourcesTest()
        : threadPool(Scheduler::GetBackground(), uniqueID),
          style{fileSource, 1, threadPool} {
        Log::setObserver(std::make_unique<Log::NullObserver>());
        transform.resize({512, 512});
        transform.setMaxPitch(85);

        // Vector tiles never answer: one that leaves the cover before it loads is cancelled, so
        // a second request for it means the cover dropped it and took it back.
        fileSource->tileResponse = [&](const Resource& resource) -> std::optional<Response> {
            const auto& tile = *resource.tileData;
            const CanonicalTileID id(tile.z, tile.x, tile.y);
            ++requests[{tile.urlTemplate, id}];
            if (tile.urlTemplate != demURL || id == withheldDEMTile || (servedDEMZoom && id.z != *servedDEMZoom)) {
                return std::nullopt;
            }
            ++served;
            const auto elevation = elevationOf(id);
            Response response;
            response.data = std::make_shared<std::string>(demTile(elevation.min, elevation.max));
            return response;
        };

        HillshadeLayer hillshade("hillshade", "dem");
        demLayers.push_back(
            makeMutable<HillshadeLayerProperties>(staticImmutableCast<HillshadeLayer::Impl>(hillshade.baseImpl)));
        LineLayer line("line", "vector");
        vectorLayers.push_back(makeMutable<LineLayerProperties>(staticImmutableCast<LineLayer::Impl>(line.baseImpl)));

        demSource.loadDescription(*fileSource);
        vectorSource.loadDescription(*fileSource);
        demImpl = demSource.baseImpl;
        demRenderSource = RenderSource::create(demImpl, threadPool);
        vectorRenderSource = RenderSource::create(vectorSource.baseImpl, threadPool);
        demRenderSource->setObserver(&observer);
        vectorRenderSource->setObserver(&observer);
    }

    ~TerrainSourcesTest() { threadPool.waitForEmpty(); }

    void jumpTo(const CameraOptions& camera) {
        transform.jumpTo(camera);
        state = transform.getState();
    }

    /// Looks straight down at `tile`'s zoom, centred a quarter tile into it, so that the view
    /// shows `tile` and its neighbours to the west, north and north-west, until their DEM has
    /// loaded.
    void loadAround(const CanonicalTileID& tile) {
        jumpTo(CameraOptions()
                   .withCenter(Projection::unproject(
                       {(tile.x + 0.25) * util::tileSize_D, (tile.y + 0.25) * util::tileSize_D}, std::exp2(tile.z)))
                   .withZoom(tile.z));
        frame(false);
        frame(false);
    }

    /// The elevation the next frame's covers will see.
    std::optional<Range<double>> elevation(const CanonicalTileID& id) const {
        return DEMElevationProvider{demRenderSource.get(), 1.0}.getTileElevationRange(id);
    }

    /// One frame: the elevation is taken before any source updates, every source's cover uses
    /// it when `terrainCover` is set, then the sources prepare their render tiles. Waits for the
    /// DEM tiles the frame received to parse, as they would before the next frame.
    void frame(bool terrainCover = true) {
        const DEMElevationProvider elevationProvider{demRenderSource.get(), 1.0};
        const TileParameters parameters{.pixelRatio = 1.0,
                                        .debugOptions = MapDebugOptions(),
                                        .transformState = state,
                                        .fileSource = fileSource,
                                        .mode = MapMode::Continuous,
                                        .annotationManager = annotationManager.makeWeakPtr(),
                                        .imageManager = imageManager,
                                        .glyphManager = glyphManager,
                                        .prefetchZoomDelta = 1,
                                        .threadPool = threadPool,
                                        .dynamicTextureAtlas = dynamicTextureAtlas,
                                        .elevationProvider = terrainCover ? &elevationProvider : nullptr};
        const bool relayout = frames++ == 0;
        demRenderSource->update(demImpl, demLayers, true, relayout, parameters);
        vectorRenderSource->update(vectorSource.baseImpl, vectorLayers, true, relayout, parameters);

        const TransformParameters transformParameters{state};
        const MapDebugOptions debugOptions = MapDebugOptions::NoDebug;
        for (auto* source : {demRenderSource.get(), vectorRenderSource.get()}) {
            source->prepare({.transform = transformParameters,
                             .debugOptions = debugOptions,
                             .imageManager = *imageManager,
                             .sourceName = source->baseImpl->id});
        }

        waitForParsing();
    }

private:
    void waitForParsing() {
        if (parsed == served) {
            return;
        }
        util::Timer timeout;
        timeout.start(Seconds(10), Duration::zero(), [&] {
            ADD_FAILURE() << "DEM tiles did not parse";
            loop.stop();
        });
        observer.tileChanged = [&](RenderSource& source, const OverscaledTileID&) {
            if (source.baseImpl->type == SourceType::RasterDEM && ++parsed == served) {
                timeout.stop();
                loop.stop();
            }
        };
        loop.run();
        observer.tileChanged = nullptr;
    }

    int frames = 0;
    int served = 0;
    int parsed = 0;
};

} // namespace

// The elevation of an area must come from the DEM data the source has, not from which of its
// tiles happen to be drawn: the cover decides what gets drawn, so an answer that follows the
// drawn tiles feeds back into the cover. Here a tile's parent is loaded but not drawn while the
// tile is absent, then loading (the parent drawn in its place), then absent again; the tile's
// range is its parent's throughout.
TEST(DEMElevationProvider, UnloadedTileTakesItsParentsRangeWhetherOrNotTheParentIsDrawn) {
    TerrainSourcesTest test;

    // z12, looking straight down, so that the cover is exactly the tiles the 512 px view
    // overlaps. Centred a quarter tile into (x, y), it shows (x-1..x, y-1..y), and its prefetch
    // cover (one zoom up) holds their parents. `parent` is the parent of (x, y), which is drawn,
    // and of `child`, its neighbour to the east, which is just out of view.
    const uint32_t x = 2 * 1100;
    const uint32_t y = 2 * 1400;
    const auto centredAt = [&](double tileX, double tileY) {
        return CameraOptions()
            .withCenter(Projection::unproject({tileX * util::tileSize_D, tileY * util::tileSize_D}, std::exp2(12)))
            .withZoom(12.0);
    };
    const CanonicalTileID parent(11, x / 2, y / 2);
    const CanonicalTileID child(12, x + 1, y);
    // One drawn tile holds much higher ground, so that the range of everything drawn is not the
    // parent's.
    const CanonicalTileID high(12, x - 1, y - 1);
    test.elevationOf = [&](const CanonicalTileID& id) {
        return covers(id, high) ? Range<int32_t>{100, 1500} : Range<int32_t>{100, 100};
    };
    test.withheldDEMTile = child;
    const Range<double> parentRange{100, 100};

    // The child absent; the parent loaded by the prefetch cover, not drawn.
    test.jumpTo(centredAt(x + 0.25, y + 0.25));
    test.frame(false);
    test.frame(false);
    EXPECT_EQ(parentRange, test.elevation(parent));
    EXPECT_EQ(parentRange, test.elevation(child));

    // The child in view and loading; the parent drawn in its place.
    test.jumpTo(centredAt(x + 1.25, y + 0.25));
    test.frame(false);
    test.frame(false);
    ASSERT_EQ(1, (test.requests[{demURL, child}]));
    EXPECT_EQ(parentRange, test.elevation(child));

    // The child out of view again and cancelled; the parent no longer drawn.
    test.jumpTo(centredAt(x + 0.25, y + 0.25));
    test.frame(false);
    EXPECT_EQ(parentRange, test.elevation(child));
}

// A still camera must settle on one tile cover. Low and steep over a hill, a DEM tile at the near
// edge of the view counts as visible when its height is taken from the highest ground in view,
// and as not visible when taken from its parent, which the DEM source holds for prefetch. While
// the tile loads its parent is drawn in its place, and if the elevation follows the drawn DEM
// tiles, the cover alternates between the two answers: every source requests the tile's area,
// drops it (which cancels the request) and requests it again, frame after frame.
TEST(DEMElevationProvider, StillCameraSettlesOnOneCover) {
    TerrainSourcesTest test;
    const uint8_t z = 15;

    // Looking north from 2.1 tiles south of the centre (z15.4, 1024 px, pitch 68), the camera
    // stands 0.85 into row 11587, and the bottom of the view meets the ground just short of
    // that row. Raised ground there comes into view, and the row's parents also cover row
    // 11586, which is in view flat.
    test.transform.resize({1024, 1024});
    test.jumpTo(
        CameraOptions()
            .withCenter(Projection::unproject({17112.5 * util::tileSize_D, 11585.74 * util::tileSize_D}, std::exp2(z)))
            .withZoom(15.4)
            .withPitch(68.0));
    const auto& state = test.state;

    // A hill under the map centre, rising to most of the camera's height; sea level elsewhere.
    const double metersPerPixel = Projection::getMetersPerPixelAtLatitude(state.getLatLng().latitude(),
                                                                          state.getZoom());
    const double cameraHeight = state.getCameraToCenterDistance() * metersPerPixel * std::cos(state.getPitch());
    const auto peak = static_cast<int32_t>(cameraHeight * 0.8);
    const auto centre = TileCoordinate::fromLatLng(z, state.getLatLng()).p;
    const CanonicalTileID hill(z, static_cast<uint32_t>(centre.x), static_cast<uint32_t>(centre.y));
    test.elevationOf = [&](const CanonicalTileID& id) {
        return covers(id, hill) ? Range<int32_t>{0, peak} : Range<int32_t>{0, 0};
    };

    // A tile at the edge of the view that the cover takes with the hill's height but not flat,
    // whose parent is not the hill's and also covers part of the flat view, so the prefetch
    // cover holds it. Its DEM never arrives.
    const Range<uint8_t> zoomRange{0, maxZoom};
    const ConstantElevation hillHeight({0, static_cast<double>(peak)});
    const auto flat = util::tileCover({.transformState = state}, z, zoomRange, z);
    const auto raised = util::tileCover({.transformState = state, .elevationProvider = &hillHeight}, z, zoomRange, z);
    const auto inFlatView = [&](const CanonicalTileID& tile) {
        return std::ranges::any_of(flat, [&](const OverscaledTileID& id) { return overlaps(tile, id.canonical); });
    };
    ASSERT_TRUE(inFlatView(hill));
    std::optional<CanonicalTileID> edge;
    for (const auto& id : raised) {
        const auto& tile = id.canonical;
        if (tile.z != z || inFlatView(tile)) {
            continue;
        }
        const auto parent = tile.scaledTo(static_cast<uint8_t>(z - 1));
        if (!covers(parent, hill) && inFlatView(parent)) {
            edge = tile;
            break;
        }
    }
    ASSERT_TRUE(edge) << "no tile at the edge of the view changes with the hill's height";
    test.withheldDEMTile = *edge;

    for (int frame = 0; frame < 10; ++frame) {
        test.frame();
    }
    ASSERT_EQ(1, (test.requests[{demURL, hill}]));

    // Each tile is requested once at most: none was dropped while loading and taken back.
    for (const auto& [key, count] : test.requests) {
        const auto& [url, tile] = key;
        EXPECT_EQ(1, count) << url << " " << util::toString(tile);
    }
}

// The ranges belong to the data they were decoded from: a new tileset starts over, as the tile
// pyramid does.
TEST(DEMElevationProvider, ForgetsTheRangesOfAReplacedTileset) {
    TerrainSourcesTest test;
    test.elevationOf = [](const CanonicalTileID&) {
        return Range<int32_t>{100, 200};
    };
    test.jumpTo(CameraOptions().withCenter(LatLng{46.55, 8.0}).withZoom(12.0));
    test.frame();
    test.frame();
    const auto centre = TileCoordinate::fromLatLng(12, test.state.getLatLng()).p;
    const CanonicalTileID tile(12, static_cast<uint32_t>(centre.x), static_cast<uint32_t>(centre.y));
    ASSERT_EQ((Range<double>{100, 200}), test.elevation(tile));

    // Another DEM, whose tiles have not arrived yet.
    RasterDEMSource replacement{"dem", Tileset{{"elsewhere/{z}/{x}/{y}"}, {0, maxZoom}}, util::tileSize_I};
    replacement.loadDescription(*test.fileSource);
    test.demImpl = replacement.baseImpl;
    test.frame();
    EXPECT_EQ(std::nullopt, test.elevation(tile));
}

// A tile that no loaded DEM covers takes the range of the loaded ground nearest to it, not of all
// the ground the source has loaded. After looking at high mountains, the camera moves on to low
// ground far away; once some of the DEM there has loaded, the tile next to it is as low as that
// ground. Taken as high as the mountains, it and the tiles beyond it would be requested by every
// source's cover, only to drop out again, cancelled, as soon as their own DEM arrived.
TEST(DEMElevationProvider, UnloadedTileTakesTheRangeOfTheGroundLoadedNearIt) {
    TerrainSourcesTest test;
    test.servedDEMZoom = 12;
    // The Bernese Alps and, some 600 km away, the Netherlands: their nearest common ancestor is a
    // tile of zoom 4.
    const CanonicalTileID mountains(12, 2139, 1448);
    const CanonicalTileID lowland(12, 2108, 1350);
    const Range<int32_t> mountainRange{400, 3000};
    const Range<int32_t> lowlandRange{0, 30};
    test.elevationOf = [&](const CanonicalTileID& id) {
        return covers(mountains.scaledTo(8), id) ? mountainRange : lowlandRange;
    };

    test.loadAround(mountains);
    test.loadAround(lowland);
    ASSERT_EQ(span(mountainRange, mountainRange), test.elevation(mountains));

    // Just east of the loaded lowland, out of view: neither it nor any tile covering it loaded.
    const CanonicalTileID next(12, lowland.x + 1, lowland.y);
    ASSERT_EQ(0, (test.requests[{demURL, next}]));
    EXPECT_EQ(span(lowlandRange, lowlandRange), test.elevation(next));
}

// A tile above the DEM's zoom levels, which the cover's traversal tests before it descends, has to
// be at least as tall as every loaded tile inside it, or the traversal culls it while one of those
// is in view. That is all it needs: ground loaded outside it does not raise it.
TEST(DEMElevationProvider, LargeTileTakesTheRangeOfTheTilesLoadedInIt) {
    TerrainSourcesTest test;
    test.servedDEMZoom = 12;
    // Two areas of different heights loaded inside one tile of zoom 8, and higher ground loaded
    // outside it.
    const CanonicalTileID large(8, 138, 88);
    const CanonicalTileID west(12, (138 * 16) + 2, (88 * 16) + 2);
    const CanonicalTileID east(12, (138 * 16) + 10, (88 * 16) + 10);
    const CanonicalTileID elsewhere(12, 1000, 1000);
    const Range<int32_t> westRange{100, 500};
    const Range<int32_t> eastRange{1000, 2000};
    test.elevationOf = [&](const CanonicalTileID& id) {
        if (covers(west.scaledTo(10), id)) {
            return westRange;
        }
        return covers(east.scaledTo(10), id) ? eastRange : Range<int32_t>{0, 4000};
    };

    test.loadAround(elsewhere);
    test.loadAround(west);
    test.loadAround(east);
    ASSERT_EQ((Range<double>{0, 4000}), test.elevation(elsewhere));

    EXPECT_EQ(span(westRange, eastRange), test.elevation(large));
    // A tile with only one of the areas inside takes that one's range.
    EXPECT_EQ(span(westRange, westRange), test.elevation(west.scaledTo(10)));

    // Every tile above a loaded one, up to z0, contains its range.
    for (const auto& tile : {west, east, elsewhere}) {
        const auto inner = test.elevation(tile);
        ASSERT_TRUE(inner);
        for (int z = tile.z - 1; z >= 0; --z) {
            const auto ancestor = tile.scaledTo(static_cast<uint8_t>(z));
            const auto outer = test.elevation(ancestor);
            ASSERT_TRUE(outer) << util::toString(ancestor);
            EXPECT_LE(outer->min, inner->min) << util::toString(ancestor);
            EXPECT_GE(outer->max, inner->max) << util::toString(ancestor);
        }
    }
}

// With nothing loaded inside a tile or near it, the tile takes the range of the deepest tile above
// it that has loaded ground inside, and so of all the ground loaded only when it shares no closer
// ancestor with any of it. Before anything has loaded, there is no answer.
TEST(DEMElevationProvider, TileWithNothingLoadedNearItTakesTheRangeOfTheNearestLoadedArea) {
    TerrainSourcesTest test;
    test.servedDEMZoom = 12;
    // Low ground in the north-west quarter of the world, high ground in the north-east one.
    const CanonicalTileID low(12, 1000, 1000);
    const CanonicalTileID high(12, 3000, 1000);
    const Range<int32_t> lowRange{0, 100};
    const Range<int32_t> highRange{0, 2000};
    test.elevationOf = [&](const CanonicalTileID& id) {
        return covers(CanonicalTileID(1, 0, 0), id) ? lowRange : highRange;
    };
    // Far from the low ground, in its quarter; and in a quarter of its own.
    const CanonicalTileID inLowQuarter(12, 1500, 1900);
    const CanonicalTileID inOwnQuarter(12, 1000, 3000);
    EXPECT_EQ(std::nullopt, test.elevation(inLowQuarter));

    test.loadAround(low);
    test.loadAround(high);
    ASSERT_EQ(span(highRange, highRange), test.elevation(high));

    EXPECT_EQ(span(lowRange, lowRange), test.elevation(inLowQuarter));
    EXPECT_EQ(span(lowRange, highRange), test.elevation(inOwnQuarter));
}
