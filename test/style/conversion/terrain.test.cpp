#include <mln/test/util.hpp>

#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/terrain.hpp>
#include <mln/style/conversion_impl.hpp>
#include <mln/style/terrain.hpp>

#include <string>

using namespace mln;
using namespace mln::style;
using namespace mln::style::conversion;

namespace {

// Converts `json`; on failure returns no value and puts the converter's message in `message`.
std::optional<Terrain> parseTerrain(const std::string& json, std::string& message) {
    Error error;
    auto terrain = convertJSON<Terrain>(json, error);
    message = error.message;
    return terrain;
}

} // namespace

TEST(StyleConversion, TerrainSourceAndExaggeration) {
    std::string message;
    const auto terrain = parseTerrain(R"({"source": "dem", "exaggeration": 2.5})", message);
    ASSERT_TRUE(terrain);
    EXPECT_EQ("dem", terrain->getSource());
    EXPECT_FLOAT_EQ(2.5f, terrain->getExaggeration());
}

TEST(StyleConversion, TerrainExaggerationDefaultsToOne) {
    std::string message;
    const auto terrain = parseTerrain(R"({"source": "dem"})", message);
    ASSERT_TRUE(terrain);
    EXPECT_EQ("dem", terrain->getSource());
    EXPECT_FLOAT_EQ(1.0f, terrain->getExaggeration());
}

TEST(StyleConversion, TerrainMustBeAnObject) {
    std::string message;
    EXPECT_FALSE(parseTerrain(R"("dem")", message));
    EXPECT_EQ("terrain must be an object", message);

    EXPECT_FALSE(parseTerrain(R"([{"source": "dem"}])", message));
    EXPECT_EQ("terrain must be an object", message);
}

TEST(StyleConversion, TerrainNeedsASource) {
    std::string message;
    EXPECT_FALSE(parseTerrain(R"({"exaggeration": 2})", message));
    EXPECT_EQ("terrain must have a source", message);
}

TEST(StyleConversion, TerrainSourceMustBeAString) {
    std::string message;
    EXPECT_FALSE(parseTerrain(R"({"source": 7})", message));
    EXPECT_EQ("terrain source must be a string", message);
}

TEST(StyleConversion, TerrainExaggerationMustBeANumber) {
    std::string message;
    EXPECT_FALSE(parseTerrain(R"({"source": "dem", "exaggeration": "high"})", message));
    EXPECT_EQ("terrain exaggeration must be a number", message);
}
