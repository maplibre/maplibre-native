#include <mln/test/util.hpp>
#include <mln/test/fake_file_source.hpp>
#include <mln/tile/vector_mvt_tile.hpp>
#include <mln/tile/vector_mvt_tile_data.hpp>
#include <mln/tile/tile_loader_impl.hpp>
#include <mln/storage/resource_options.hpp>

#include <mln/util/io.hpp>
#include <mln/util/run_loop.hpp>
#include <mln/map/transform.hpp>
#include <mln/style/style.hpp>
#include <mln/style/layers/symbol_layer.hpp>
#include <mln/renderer/tile_parameters.hpp>
#include <mln/renderer/buckets/symbol_bucket.hpp>
#include <mln/renderer/query.hpp>
#include <mln/geometry/feature_index.hpp>
#include <mln/annotation/annotation_manager.hpp>
#include <mln/renderer/image_manager.hpp>
#include <mln/test/vector_tile_test.hpp>
#include <mln/text/glyph_manager.hpp>

#include <memory>
#include "mln/tile/vector_mlt_tile_data.hpp"

using namespace mln;

TEST(VectorTile, setError) {
    VectorTileTest test;
    VectorMVTTile tile(OverscaledTileID(0, 0, 0), "source", test.tileParameters, test.tileset);
    tile.setError(std::make_exception_ptr(std::runtime_error("test")));
    EXPECT_FALSE(tile.isRenderable());
    EXPECT_TRUE(tile.isLoaded());
    EXPECT_TRUE(tile.isComplete());
}

TEST(VectorTile, onError) {
    VectorTileTest test;
    VectorMVTTile tile(OverscaledTileID(0, 0, 0), "source", test.tileParameters, test.tileset);
    tile.onError(std::make_exception_ptr(std::runtime_error("test")), 0);

    EXPECT_FALSE(tile.isRenderable());
    EXPECT_TRUE(tile.isLoaded());
    EXPECT_TRUE(tile.isComplete());
}

TEST(VectorTile, Issue8542) {
    VectorTileTest test;
    VectorMVTTile tile(OverscaledTileID(0, 0, 0), "source", test.tileParameters, test.tileset);

    // Query before data is set
    std::vector<Feature> result;
    tile.querySourceFeatures(result, {{{"layer"}}, {}}, nullptr);
}

TEST(VectorTile, HoldForFadeLastsTheFadeDuration) {
    VectorTileTest test;
    VectorMVTTile tile(OverscaledTileID(0, 0, 0), "source", test.tileParameters, test.tileset);
    const Duration fadeDuration = Milliseconds(300);
    const TimePoint start = TimePoint() + Seconds(10);

    EXPECT_FALSE(tile.holdForFade());

    // The tile leaves the ideal set and is held for its symbols to fade out.
    tile.markRenderedPreviously();
    EXPECT_TRUE(tile.holdForFade());

    // While new tiles arrive, placement runs every 30 ms. The fade starts with the first placement and the tile
    // must be held until it has run its whole course, not just for two placements.
    for (int ms = 0; ms < 300; ms += 30) {
        tile.performedFadePlacement(start + Milliseconds(ms), fadeDuration);
        EXPECT_TRUE(tile.holdForFade()) << "placement at " << ms << " ms";
    }

    tile.performedFadePlacement(start + Milliseconds(300), fadeDuration);
    EXPECT_FALSE(tile.holdForFade());
}

TEST(VectorTile, HoldForFadeTwoPlacementsAtTheUsualPeriod) {
    VectorTileTest test;
    VectorMVTTile tile(OverscaledTileID(0, 0, 0), "source", test.tileParameters, test.tileset);
    const Duration fadeDuration = Milliseconds(300);
    const TimePoint start = TimePoint() + Seconds(10);

    // When placements are a fade duration apart, the tile goes after the second, as before.
    tile.markRenderedPreviously();
    tile.performedFadePlacement(start, fadeDuration);
    EXPECT_TRUE(tile.holdForFade());
    tile.performedFadePlacement(start + fadeDuration, fadeDuration);
    EXPECT_FALSE(tile.holdForFade());
}

TEST(VectorTile, HoldForFadeWithoutFade) {
    VectorTileTest test;
    VectorMVTTile tile(OverscaledTileID(0, 0, 0), "source", test.tileParameters, test.tileset);
    const TimePoint start = TimePoint() + Seconds(10);

    // Without a fade the tile is still kept for two placements.
    tile.markRenderedPreviously();
    tile.performedFadePlacement(start, Duration::zero());
    EXPECT_TRUE(tile.holdForFade());
    tile.performedFadePlacement(start, Duration::zero());
    EXPECT_FALSE(tile.holdForFade());
}

TEST(VectorTile, HoldForFadeRestartsWhenIdealAgain) {
    VectorTileTest test;
    VectorMVTTile tile(OverscaledTileID(0, 0, 0), "source", test.tileParameters, test.tileset);
    const Duration fadeDuration = Milliseconds(300);
    const TimePoint start = TimePoint() + Seconds(10);

    tile.markRenderedPreviously();
    tile.performedFadePlacement(start, fadeDuration);

    // Back in the ideal set: no longer held.
    tile.markRenderedIdeal();
    EXPECT_FALSE(tile.holdForFade());

    // Leaving again starts a new fade, timed from the next placement.
    tile.markRenderedPreviously();
    tile.performedFadePlacement(start + Milliseconds(400), fadeDuration);
    tile.performedFadePlacement(start + Milliseconds(600), fadeDuration);
    EXPECT_TRUE(tile.holdForFade());
    tile.performedFadePlacement(start + Milliseconds(700), fadeDuration);
    EXPECT_FALSE(tile.holdForFade());
}

TEST(VectorTileData, ParseResults) {
    VectorMVTTileData data(std::make_shared<std::string>(util::read_file("test/fixtures/map/issue12432/0-0-0.mvt")));

    std::vector<std::string> layerNames = data.layerNames();
    ASSERT_EQ(layerNames.size(), 2u);
    ASSERT_EQ(layerNames.at(0), "admin");
    ASSERT_EQ(layerNames.at(1), "water");

    ASSERT_FALSE(data.getLayer("invalid"));

    std::unique_ptr<GeometryTileLayer> layer = data.getLayer("admin");
    ASSERT_EQ(layer->getName(), "admin");
    ASSERT_EQ(layer->featureCount(), 17154u);

    ASSERT_THROW(layer->getFeature(17154u), std::out_of_range);

    std::unique_ptr<GeometryTileFeature> feature = layer->getFeature(0u);
    ASSERT_EQ(feature->getType(), mln::FeatureType::LineString);
    ASSERT_TRUE(feature->getID().is<uint64_t>());
    ASSERT_EQ(feature->getID().get<uint64_t>(), 1u);

    const std::unordered_map<std::string, Value>& properties = feature->getProperties();
    ASSERT_EQ(properties.size(), 3u);
    ASSERT_EQ(properties.at("disputed"), *feature->getValue("disputed"));

    ASSERT_EQ(feature->getValue("invalid"), std::nullopt);
}

TEST(VectorTileData, MLTParseResults) {
    struct SubCase {
        bool useFastPFOR;
        bool fastPFOREnabled;
    };
    for (const auto& testCase : std::vector{
             SubCase{.useFastPFOR = false, .fastPFOREnabled = false},
             SubCase{.useFastPFOR = false, .fastPFOREnabled = true},
             SubCase{.useFastPFOR = true, .fastPFOREnabled = true},
             SubCase{.useFastPFOR = true, .fastPFOREnabled = false},
         }) {
        const auto path = "test/fixtures/map/issue12432/0-0-0" +
                          std::string(testCase.useFastPFOR ? "-fastpfor.mlt" : ".mlt");
        VectorMLTTileData data(std::make_shared<std::string>(util::read_file(path)), testCase.fastPFOREnabled);

        std::vector<std::string> layerNames = data.layerNames();

        if (!testCase.fastPFOREnabled && testCase.useFastPFOR) {
            // If fast PFOR is not enabled, the fast PFOR tile should fail to parse
            ASSERT_EQ(layerNames.size(), 0u);
            continue;
        }

        // MLT layers are presented in the order they're encountered, not sorted alphabetically like MVT layers
        ASSERT_EQ(layerNames.size(), 2u);
        ASSERT_EQ(layerNames.at(0), "water");
        ASSERT_EQ(layerNames.at(1), "admin");

        ASSERT_FALSE(data.getLayer("invalid"));

        std::unique_ptr<GeometryTileLayer> layer = data.getLayer("admin");
        ASSERT_EQ(layer->getName(), "admin");
        ASSERT_EQ(layer->featureCount(), 17154u);

        std::unique_ptr<GeometryTileFeature> feature = layer->getFeature(0u);
        ASSERT_EQ(feature->getType(), mln::FeatureType::LineString);
        ASSERT_TRUE(feature->getID().is<uint64_t>());
        ASSERT_EQ(feature->getID().get<uint64_t>(), 1u);

        const std::unordered_map<std::string, Value>& properties = feature->getProperties();
        ASSERT_EQ(properties.size(), 3u);
        ASSERT_EQ(properties.at("disputed"), *feature->getValue("disputed"));

        ASSERT_EQ(feature->getValue("invalid"), std::nullopt);
    }
}
