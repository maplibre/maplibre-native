#include <mln/test/util.hpp>
#include <mln/test/fake_file_source.hpp>
#include <mln/test/stub_tile_observer.hpp>
#include <mln/style/sources/custom_geometry_source.hpp>
#include <mln/tile/custom_geometry_tile.hpp>
#include <mln/style/custom_tile_loader.hpp>

#include <mln/util/run_loop.hpp>
#include <mln/map/transform.hpp>
#include <mln/renderer/tile_parameters.hpp>
#include <mln/renderer/query.hpp>
#include <mln/style/style.hpp>
#include <mln/style/layers/circle_layer.hpp>
#include <mln/style/layers/circle_layer_impl.hpp>
#include <mln/annotation/annotation_manager.hpp>
#include <mln/renderer/image_manager.hpp>
#include <mln/text/glyph_manager.hpp>
#include <mln/gfx/dynamic_texture_atlas.hpp>

#include <memory>

using namespace mln;
using namespace mln::style;

class CustomTileTest {
public:
    util::SimpleIdentity uniqueID;
    std::shared_ptr<FileSource> fileSource = std::make_shared<FakeFileSource>();
    TransformState transformState;
    util::RunLoop loop;
    AnnotationManager annotationManager{style};
    std::shared_ptr<ImageManager> imageManager = ImageManager::create();
    std::shared_ptr<GlyphManager> glyphManager = std::make_shared<GlyphManager>();
    gfx::DynamicTextureAtlasPtr dynamicTextureAtlas;

    TileParameters tileParameters;
    style::Style style;

    CustomTileTest()
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

TEST(CustomGeometryTile, InvokeFetchTile) {
    CustomTileTest test;

    CircleLayer layer("circle", "source");

    mapbox::feature::feature_collection<double> features;
    features.push_back(mapbox::feature::feature<double>{mapbox::geometry::point<double>(0, 0)});
    CustomTileLoader loader(
        [&](const CanonicalTileID& tileId) {
            EXPECT_EQ(tileId, CanonicalTileID(0, 0, 0));
            test.loop.stop();
        },
        [&](const CanonicalTileID&) {

        });
    auto mb = std::make_shared<Mailbox>(*Scheduler::GetCurrent());
    ActorRef<CustomTileLoader> loaderActor(loader, mb);

    CustomGeometryTile tile(OverscaledTileID(0, 0, 0),
                            "source",
                            test.tileParameters,
                            makeMutable<CustomGeometrySource::TileOptions>(),
                            loaderActor);

    tile.setNecessity(TileNecessity::Required);

    test.loop.run();
}

TEST(CustomGeometryTile, InvokeCancelTile) {
    CustomTileTest test;

    CircleLayer layer("circle", "source");

    mapbox::feature::feature_collection<double> features;
    features.push_back(mapbox::feature::feature<double>{mapbox::geometry::point<double>(0, 0)});

    CustomTileLoader loader([&](const CanonicalTileID&) {},
                            [&](const CanonicalTileID& tileId) {
                                EXPECT_EQ(tileId, CanonicalTileID(0, 0, 0));
                                test.loop.stop();
                            });
    auto mb = std::make_shared<Mailbox>(*Scheduler::GetCurrent());
    ActorRef<CustomTileLoader> loaderActor(loader, mb);

    CustomGeometryTile tile(OverscaledTileID(0, 0, 0),
                            "source",
                            test.tileParameters,
                            makeMutable<CustomGeometrySource::TileOptions>(),
                            loaderActor);

    tile.setNecessity(TileNecessity::Required);
    tile.setNecessity(TileNecessity::Optional);
    test.loop.run();
}

TEST(CustomGeometryTile, FetchCancelledTileAgain) {
    CustomTileTest test;

    int fetches = 0;
    CustomTileLoader loader([&](const CanonicalTileID&) { fetches++; }, nullptr);
    auto mb = std::make_shared<Mailbox>(*Scheduler::GetCurrent());
    ActorRef<CustomTileLoader> loaderActor(loader, mb);

    CustomGeometryTile tile(OverscaledTileID(0, 0, 0),
                            "source",
                            test.tileParameters,
                            makeMutable<CustomGeometrySource::TileOptions>(),
                            loaderActor);

    tile.setNecessity(TileNecessity::Required);
    tile.setNecessity(TileNecessity::Optional);
    test.loop.waitForEmpty();
    ASSERT_EQ(1, fetches);

    tile.setNecessity(TileNecessity::Required);
    test.loop.waitForEmpty();
    EXPECT_EQ(2, fetches);
}

TEST(CustomGeometryTile, InvokeTileChanged) {
    CustomTileTest test;

    CircleLayer layer("circle", "source");

    mapbox::feature::feature_collection<double> features;
    features.push_back(mapbox::feature::feature<double>{mapbox::geometry::point<double>(0, 0)});

    CustomTileLoader loader(nullptr, nullptr);
    auto mb = std::make_shared<Mailbox>(*Scheduler::GetCurrent());
    ActorRef<CustomTileLoader> loaderActor(loader, mb);

    CustomGeometryTile tile(OverscaledTileID(0, 0, 0),
                            "source",
                            test.tileParameters,
                            makeMutable<CustomGeometrySource::TileOptions>(),
                            loaderActor);

    Immutable<LayerProperties> layerProperties = makeMutable<CircleLayerProperties>(
        staticImmutableCast<CircleLayer::Impl>(layer.baseImpl));
    StubTileObserver observer;
    observer.tileChanged = [&](const Tile&) {
        // Once present, the bucket should never "disappear", which would cause
        // flickering.
        ASSERT_TRUE(tile.layerPropertiesUpdated(layerProperties));
    };

    std::vector<Immutable<LayerProperties>> layers{layerProperties};
    tile.setLayers(layers, test.tileParameters.globalState);
    tile.setObserver(&observer);
    std::vector<Feature> result;
    tile.querySourceFeatures(result, {}, nullptr);
    EXPECT_TRUE(result.empty());

    tile.setTileData(features);

    while (!tile.isComplete()) {
        test.loop.runOnce();
    }

    tile.querySourceFeatures(result, {}, nullptr);
    EXPECT_EQ(1u, result.size());
}

TEST(CustomGeometryTile, CancellationDiscardsQueuedInvalidation) {
    CustomTileTest test;
    StubTileObserver observer;
    unsigned changes = 0;
    observer.tileChanged = [&](Tile&) {
        ++changes;
    };
    bool requested = false;
    CustomTileLoader loader([&](const CanonicalTileID&) { requested = true; }, nullptr);
    auto mailbox = std::make_shared<Mailbox>(*Scheduler::GetCurrent());
    ActorRef<CustomTileLoader> loaderActor(loader, mailbox);
    const OverscaledTileID id(0, 0, 0);
    CustomGeometryTile tile(
        id, "source", test.tileParameters, makeMutable<CustomGeometrySource::TileOptions>(), loaderActor, &observer);
    tile.setNecessity(TileNecessity::Required);
    test.loop.runOnce();
    ASSERT_TRUE(requested);

    loader.invalidateTile(id.canonical);
    tile.cancel();
    test.loop.runOnce();

    EXPECT_EQ(changes, 0u);
}

TEST(CustomGeometryTile, CancellationDiscardsLateInvalidation) {
    CustomTileTest test;
    StubTileObserver observer;
    unsigned changes = 0;
    observer.tileChanged = [&](Tile&) {
        ++changes;
    };
    bool requested = false;
    CustomTileLoader loader([&](const CanonicalTileID&) { requested = true; }, nullptr);
    auto mailbox = std::make_shared<Mailbox>(*Scheduler::GetCurrent());
    ActorRef<CustomTileLoader> loaderActor(loader, mailbox);
    const OverscaledTileID id(0, 0, 0);
    CustomGeometryTile tile(
        id, "source", test.tileParameters, makeMutable<CustomGeometrySource::TileOptions>(), loaderActor, &observer);
    tile.setNecessity(TileNecessity::Required);
    test.loop.runOnce();
    ASSERT_TRUE(requested);

    tile.cancel();
    loader.invalidateTile(id.canonical);
    test.loop.runOnce();

    EXPECT_EQ(changes, 0u);
}

// Two tiles of one canonical tile (here the same tile in two world copies)
// share its loader entry. Releasing one must not drop the other's registration.
TEST(CustomGeometryTile, CancelOneInstanceKeepsOtherRegistered) {
    CustomTileTest test;

    int fetches = 0;
    int cancels = 0;
    CustomTileLoader loader([&](const CanonicalTileID&) { fetches++; }, [&](const CanonicalTileID&) { cancels++; });
    auto mb = std::make_shared<Mailbox>(*Scheduler::GetCurrent());
    ActorRef<CustomTileLoader> loaderActor(loader, mb);

    StubTileObserver observer;
    Immutable<CustomGeometrySource::TileOptions> options = makeMutable<CustomGeometrySource::TileOptions>();
    CustomGeometryTile first(OverscaledTileID(0, 0, 0), "source", test.tileParameters, options, loaderActor, &observer);
    CustomGeometryTile second(OverscaledTileID(0, 1, CanonicalTileID(0, 0, 0)),
                              "source",
                              test.tileParameters,
                              options,
                              loaderActor,
                              &observer);

    first.setNecessity(TileNecessity::Required);
    second.setNecessity(TileNecessity::Required);
    test.loop.waitForEmpty();
    const int initialFetches = fetches;
    ASSERT_GT(initialFetches, 0);

    first.setNecessity(TileNecessity::Optional);
    test.loop.waitForEmpty();
    // The other tile still needs the fetch.
    EXPECT_EQ(0, cancels);

    // The remaining tile must still be invalidated, and fetched again.
    loader.invalidateTile(CanonicalTileID(0, 0, 0));
    test.loop.waitForEmpty();
    second.setNecessity(TileNecessity::Required);
    test.loop.waitForEmpty();
    EXPECT_EQ(initialFetches + 1, fetches);
}

// Data handed back before a tile is parsed must not outlive the tile's last
// registration, or the next tile at that position is served stale data.
TEST(CustomGeometryTile, CancelDiscardsCachedData) {
    CustomTileTest test;

    int fetches = 0;
    CustomTileLoader loader([&](const CanonicalTileID&) { fetches++; }, nullptr);
    auto mb = std::make_shared<Mailbox>(*Scheduler::GetCurrent());
    ActorRef<CustomTileLoader> loaderActor(loader, mb);

    const OverscaledTileID id(0, 0, 0);
    Immutable<CustomGeometrySource::TileOptions> options = makeMutable<CustomGeometrySource::TileOptions>();
    {
        CustomGeometryTile tile(id, "source", test.tileParameters, options, loaderActor);
        tile.setNecessity(TileNecessity::Required);
        test.loop.waitForEmpty();
        ASSERT_EQ(1, fetches);

        // Cached, but the tile is not renderable until it has been parsed.
        loader.setTileData(id.canonical, GeoJSON{FeatureCollection{}});
        ASSERT_FALSE(tile.isRenderable());
        tile.setNecessity(TileNecessity::Optional);
        test.loop.waitForEmpty();
    }
    test.loop.waitForEmpty();

    CustomGeometryTile next(id, "source", test.tileParameters, options, loaderActor);
    next.setNecessity(TileNecessity::Required);
    test.loop.waitForEmpty();
    EXPECT_EQ(2, fetches);
}
