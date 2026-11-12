#include <mln/test/util.hpp>
#include <mln/test/fake_file_source.hpp>
#include <mln/tile/raster_dem_tile.hpp>
#include <mln/tile/raster_tile.hpp>
#include <mln/tile/tile_loader_impl.hpp>

#include <mln/style/style.hpp>
#include <mln/util/run_loop.hpp>
#include <mln/map/transform.hpp>
#include <mln/annotation/annotation_manager.hpp>
#include <mln/renderer/tile_parameters.hpp>
#include <mln/renderer/buckets/hillshade_bucket.hpp>
#include <mln/renderer/image_manager.hpp>
#include <mln/text/glyph_manager.hpp>
#include <mln/gfx/dynamic_texture_atlas.hpp>
#include <mln/util/tile_cover.hpp>

using namespace mln;

class RasterDEMTileTest {
public:
    util::SimpleIdentity uniqueID;
    std::shared_ptr<FileSource> fileSource = std::make_shared<FakeFileSource>();
    TransformState transformState;
    util::RunLoop loop;
    AnnotationManager annotationManager{style};
    std::shared_ptr<ImageManager> imageManager = ImageManager::create();
    std::shared_ptr<GlyphManager> glyphManager = std::make_shared<GlyphManager>();
    gfx::DynamicTextureAtlasPtr dynamicTextureAtlas;

    Tileset tileset{{"https://example.com"}, {0, 22}, "none"};
    TileParameters tileParameters;
    style::Style style;

    RasterDEMTileTest()
        : tileParameters{.pixelRatio = 1.0,
                         .debugOptions = MapDebugOptions(),
                         .transformState = transformState,
                         .fileSource = fileSource,
                         .mode = MapMode::Continuous,
                         .annotationManager = annotationManager.makeWeakPtr(),
                         .imageManager = imageManager,
                         .glyphManager = glyphManager,
                         .prefetchZoomDelta = 0,
                         .threadPool = {Scheduler::GetBackground(), uniqueID},
                         .dynamicTextureAtlas = dynamicTextureAtlas},
          style{fileSource, 1, tileParameters.threadPool} {}
};

// A file source with a cache, as the default one has: a tile asks it for a
// cache-only response first.
class CachingFakeFileSource : public FakeFileSource {
public:
    bool supportsCacheOnlyRequests() const override { return true; }
};

// Stands for the terrain: the tile only asks whether there is one.
class NoElevation : public util::TileElevationProvider {
public:
    std::optional<Range<double>> getTileElevationRange(const CanonicalTileID&) const override { return std::nullopt; }
};

// An elevation tile is requested high priority under terrain, and an
// imagery tile is not. A hillshade layer alone has nothing waiting for its
// elevation tile.
TEST(RasterDEMTile, RequestsAreHighPriorityUnderTerrain) {
    RasterDEMTileTest test;
    auto fileSource = std::static_pointer_cast<FakeFileSource>(test.fileSource);

    RasterDEMTile hillshade(OverscaledTileID(0, 0, 0), "hillshade", test.tileParameters, test.tileset);
    hillshade.setNecessity(TileNecessity::Required);
    ASSERT_EQ(fileSource->requests.size(), 1u);
    EXPECT_EQ(fileSource->requests.back()->resource.priority, Resource::Priority::Regular);

    NoElevation terrain;
    test.tileParameters.elevationProvider = &terrain;

    RasterDEMTile elevation(OverscaledTileID(0, 0, 0), "elevation", test.tileParameters, test.tileset);
    elevation.setNecessity(TileNecessity::Required);
    ASSERT_EQ(fileSource->requests.size(), 2u);
    EXPECT_EQ(fileSource->requests.back()->resource.priority, Resource::Priority::High);

    RasterTile imagery(OverscaledTileID(0, 0, 0), "imagery", test.tileParameters, test.tileset);
    imagery.setNecessity(TileNecessity::Required);
    ASSERT_EQ(fileSource->requests.size(), 3u);
    EXPECT_EQ(fileSource->requests.back()->resource.priority, Resource::Priority::Regular);
}

// Only an elevation tile with nothing to show is what the mesh waits for. Once
// the cache has answered with data, revalidating it is a regular request.
TEST(RasterDEMTile, RevalidationOfATileWithDataIsRegularPriority) {
    RasterDEMTileTest test;
    auto fileSource = std::make_shared<CachingFakeFileSource>();
    test.tileParameters.fileSource = fileSource;
    NoElevation terrain;
    test.tileParameters.elevationProvider = &terrain;

    {
        RasterDEMTile uncached(OverscaledTileID(0, 0, 0), "uncached", test.tileParameters, test.tileset);
        uncached.setNecessity(TileNecessity::Required);
        ASSERT_EQ(fileSource->requests.size(), 1u);
        ASSERT_EQ(fileSource->requests.back()->resource.loadingMethod, Resource::LoadingMethod::CacheOnly);

        Response miss;
        miss.noContent = true;
        miss.error = std::make_unique<Response::Error>(Response::Error::Reason::NotFound, "not cached");
        ASSERT_TRUE(fileSource->respond(Resource::Tile, miss));

        ASSERT_EQ(fileSource->requests.size(), 1u);
        EXPECT_EQ(fileSource->requests.back()->resource.loadingMethod, Resource::LoadingMethod::NetworkOnly);
        EXPECT_EQ(fileSource->requests.back()->resource.priority, Resource::Priority::High);

        // A failed answer leaves the tile with nothing, so asking again is High too.
        Response failure;
        failure.error = std::make_unique<Response::Error>(Response::Error::Reason::Connection, "offline");
        ASSERT_TRUE(fileSource->respond(Resource::Tile, failure));
        uncached.setNecessity(TileNecessity::Optional);
        uncached.setNecessity(TileNecessity::Required);
        ASSERT_EQ(fileSource->requests.size(), 1u);
        EXPECT_EQ(fileSource->requests.back()->resource.priority, Resource::Priority::High);
    }

    RasterDEMTile cached(OverscaledTileID(0, 0, 0), "cached", test.tileParameters, test.tileset);
    cached.setNecessity(TileNecessity::Required);
    ASSERT_EQ(fileSource->requests.size(), 1u);
    ASSERT_EQ(fileSource->requests.back()->resource.loadingMethod, Resource::LoadingMethod::CacheOnly);

    Response hit;
    hit.data = std::make_shared<std::string>("elevation");
    ASSERT_TRUE(fileSource->respond(Resource::Tile, hit));

    ASSERT_EQ(fileSource->requests.size(), 1u);
    EXPECT_EQ(fileSource->requests.back()->resource.loadingMethod, Resource::LoadingMethod::NetworkOnly);
    EXPECT_EQ(fileSource->requests.back()->resource.priority, Resource::Priority::Regular);
}

TEST(RasterDEMTile, setError) {
    RasterDEMTileTest test;
    RasterDEMTile tile(OverscaledTileID(0, 0, 0), "testSource", test.tileParameters, test.tileset);
    tile.setError(std::make_exception_ptr(std::runtime_error("test")));
    EXPECT_FALSE(tile.isRenderable());
    EXPECT_TRUE(tile.isLoaded());
    EXPECT_TRUE(tile.isComplete());
}

TEST(RasterDEMTile, onError) {
    RasterDEMTileTest test;
    RasterDEMTile tile(OverscaledTileID(0, 0, 0), "testSource", test.tileParameters, test.tileset);
    tile.onError(std::make_exception_ptr(std::runtime_error("test")), 0);
    EXPECT_FALSE(tile.isRenderable());
    EXPECT_TRUE(tile.isLoaded());
    EXPECT_TRUE(tile.isComplete());
}

TEST(RasterDEMTile, onParsed) {
    RasterDEMTileTest test;
    RasterDEMTile tile(OverscaledTileID(0, 0, 0), "testSource", test.tileParameters, test.tileset);
    tile.onParsed(std::make_unique<HillshadeBucket>(PremultipliedImage({16, 16}), Tileset::RasterEncoding::Mapbox), 0);
    EXPECT_TRUE(tile.isRenderable());
    EXPECT_TRUE(tile.isLoaded());
    EXPECT_TRUE(tile.isComplete());

    // Make sure that once we've had a renderable tile and then receive
    // erroneous data, we retain the previously rendered data and keep the tile
    // renderable.
    tile.setError(std::make_exception_ptr(std::runtime_error("Connection offline")));
    EXPECT_TRUE(tile.isRenderable());
    EXPECT_TRUE(tile.isLoaded());
    EXPECT_TRUE(tile.isComplete());

    // Then simulate a parsing failure and make sure that we keep it renderable
    // in this situation as well.
    tile.onError(std::make_exception_ptr(std::runtime_error("Parse error")), 0);
    ASSERT_TRUE(tile.isRenderable());
    EXPECT_TRUE(tile.isLoaded());
    EXPECT_TRUE(tile.isComplete());
}

TEST(RasterDEMTile, onParsedEmpty) {
    RasterDEMTileTest test;
    RasterDEMTile tile(OverscaledTileID(0, 0, 0), "testSource", test.tileParameters, test.tileset);
    tile.onParsed(nullptr, 0);
    EXPECT_FALSE(tile.isRenderable());
    EXPECT_TRUE(tile.isLoaded());
    EXPECT_TRUE(tile.isComplete());
}
