#include <mln/test/util.hpp>
#include <mln/test/fake_file_source.hpp>
#include <mln/test/stub_tile_observer.hpp>
#include <mln/style/sources/custom_geometry_source.hpp>
#include <mln/tile/custom_geometry_tile.hpp>
#include <mln/style/custom_tile_loader.hpp>

#include <mln/util/run_loop.hpp>
#include <mln/map/transform.hpp>
#include <mln/renderer/tile_parameters.hpp>
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
    tile.setLayers(layers);
    tile.setObserver(&observer);
    tile.setTileData(features);

    while (!tile.isComplete()) {
        test.loop.runOnce();
    }
}
