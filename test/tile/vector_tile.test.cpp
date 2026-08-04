#include <mbgl/test/util.hpp>
#include <mbgl/test/fake_file_source.hpp>
#include <mbgl/tile/vector_mvt_tile.hpp>
#include <mbgl/tile/vector_mvt_tile_data.hpp>
#include <mbgl/tile/tile_loader_impl.hpp>
#include <mbgl/storage/resource_options.hpp>

#include <mbgl/util/io.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/map/transform.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/style/layers/symbol_layer.hpp>
#include <mbgl/renderer/tile_parameters.hpp>
#include <mbgl/renderer/buckets/symbol_bucket.hpp>
#include <mbgl/renderer/query.hpp>
#include <mbgl/geometry/feature_index.hpp>
#include <mbgl/annotation/annotation_manager.hpp>
#include <mbgl/renderer/image_manager.hpp>
#include <mbgl/test/vector_tile_test.hpp>
#include <mbgl/text/glyph_manager.hpp>

#include <memory>
#include "mbgl/tile/vector_mlt_tile_data.hpp"

using namespace mbgl;

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
    tile.querySourceFeatures(result, {{{"layer"}}, {}});
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
    ASSERT_EQ(feature->getType(), mbgl::FeatureType::LineString);
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
        ASSERT_EQ(feature->getType(), mbgl::FeatureType::LineString);
        ASSERT_TRUE(feature->getID().is<uint64_t>());
        ASSERT_EQ(feature->getID().get<uint64_t>(), 1u);

        const std::unordered_map<std::string, Value>& properties = feature->getProperties();
        ASSERT_EQ(properties.size(), 3u);
        ASSERT_EQ(properties.at("disputed"), *feature->getValue("disputed"));

        ASSERT_EQ(feature->getValue("invalid"), std::nullopt);
    }
}
