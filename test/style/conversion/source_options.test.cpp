#include <mln/test/util.hpp>

#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/source_options.hpp>
#include <mln/util/tileset.hpp>

#include <mln/util/logging.hpp>

using namespace mln;
using namespace mln::style;
using namespace mln::style::conversion;

TEST(SourceOptions, Basic) {
    Error error;
    auto converted = convertJSON<SourceOptions>("{}", error);
    ASSERT_FALSE(converted);
}

TEST(SourceOptions, ErrorHandling) {
    Error error;
    auto converted = convertJSON<SourceOptions>(
        R"JSON({
        "encoding": "this isn't a valid encoding"
    })JSON",
        error);
    ASSERT_FALSE(converted);
    ASSERT_EQ(0, error.message.find("invalid encoding"));
}

TEST(SourceOptions, TerrariumEncodingParsed) {
    Error error;
    auto converted = convertJSON<SourceOptions>(
        R"JSON({
        "encoding": "terrarium"
    })JSON",
        error);
    ASSERT_EQ(converted.value().rasterEncoding, Tileset::RasterEncoding::Terrarium);
    ASSERT_FALSE(converted.value().vectorEncoding);
}

TEST(SourceOptions, MapboxEncodingParsed) {
    Error error;
    auto converted = convertJSON<SourceOptions>(
        R"JSON({
        "encoding": "mapbox"
    })JSON",
        error);
    ASSERT_EQ(converted.value().rasterEncoding, Tileset::RasterEncoding::Mapbox);
    ASSERT_FALSE(converted.value().vectorEncoding);
}

TEST(SourceOptions, MVTEncodingParsed) {
    Error error;
    auto converted = convertJSON<SourceOptions>(
        R"JSON({
        "encoding": "mvt"
    })JSON",
        error);
    ASSERT_EQ(converted.value().vectorEncoding, Tileset::VectorEncoding::Mapbox);
    ASSERT_FALSE(converted.value().rasterEncoding);
}

TEST(SourceOptions, MLTEncodingParsed) {
    Error error;
    auto converted = convertJSON<SourceOptions>(
        R"JSON({
        "encoding": "mlt"
    })JSON",
        error);
    ASSERT_EQ(converted.value().vectorEncoding, Tileset::VectorEncoding::MLT);
    ASSERT_FALSE(converted.value().rasterEncoding);
}
