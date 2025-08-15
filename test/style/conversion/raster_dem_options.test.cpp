#include <mbgl/test/util.hpp>

#include <mbgl/style/conversion/json.hpp>
#include <mbgl/style/conversion/raster_dem_options.hpp>
#include <mbgl/util/tileset.hpp>

#include <mbgl/util/logging.hpp>

using namespace mbgl;
using namespace mbgl::style;
using namespace mbgl::style::conversion;

TEST(RasterDEMOptions, Basic) {
    Error error;
    std::optional<RasterDEMOptions> converted = convertJSON<RasterDEMOptions>("{}", error);
    ASSERT_TRUE((bool)converted);
}

TEST(RasterDEMOptions, ErrorHandling) {
    Error error;
    std::optional<RasterDEMOptions> converted = convertJSON<RasterDEMOptions>(
        R"JSON({
        "encoding": "this isn't a valid encoding"
    })JSON",
        error);
    ASSERT_FALSE(converted);
    ASSERT_EQ(error.message, "invalid raster-dem encoding type - valid types are 'mapbox' and 'terrarium'");
}

TEST(RasterDEMOptions, TerrariumEncodingParsed) {
    Error error;
    std::optional<RasterDEMOptions> converted = convertJSON<RasterDEMOptions>(
        R"JSON({
        "encoding": "terrarium"
    })JSON",
        error);
    ASSERT_EQ(converted.value().encoding, Tileset::Encoding::Terrarium);
}

TEST(RasterDEMOptions, MapboxEncodingParsed) {
    Error error;
    std::optional<RasterDEMOptions> converted = convertJSON<RasterDEMOptions>(
        R"JSON({
        "encoding": "mapbox"
    })JSON",
        error);
    ASSERT_EQ(converted.value().encoding, Tileset::Encoding::Mapbox);
}

TEST(RasterDEMOptions, MVTEncodingParsed) {
    Error error;
    std::optional<RasterDEMOptions> converted = convertJSON<RasterDEMOptions>(
        R"JSON({
        "encoding": "mvt"
    })JSON",
        error);
    ASSERT_EQ(converted.value().encoding, Tileset::Encoding::Mapbox);
}

TEST(RasterDEMOptions, MLTEncodingParsed) {
    Error error;
    std::optional<RasterDEMOptions> converted = convertJSON<RasterDEMOptions>(
        R"JSON({
        "encoding": "mlt"
    })JSON",
        error);
    ASSERT_EQ(converted.value().encoding, Tileset::Encoding::MLT);
}
