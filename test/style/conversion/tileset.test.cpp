#include <mln/test/util.hpp>

#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/tileset.hpp>

#include <mln/util/logging.hpp>

using namespace mln;
using namespace mln::style::conversion;

TEST(Tileset, Empty) {
    Error error;
    std::optional<Tileset> converted = convertJSON<Tileset>("{}", error);
    EXPECT_FALSE((bool)converted);
}

TEST(Tileset, ErrorHandling) {
    Error error;
    std::optional<Tileset> converted = convertJSON<Tileset>(
        R"JSON({
        "tiles": "should not be a string"
    })JSON",
        error);
    EXPECT_FALSE((bool)converted);
}

TEST(Tileset, InvalidBounds) {
    {
        Error error;
        std::optional<Tileset> converted = convertJSON<Tileset>(
            R"JSON({
            "tiles": ["http://mytiles"],
            "bounds": [73, -180, -73, -120]
        })JSON",
            error);

        EXPECT_FALSE((bool)converted);
    }
    {
        Error error;
        std::optional<Tileset> converted = convertJSON<Tileset>(
            R"JSON({
            "tiles": ["http://mytiles"],
            "bounds": [-120]
        })JSON",
            error);

        EXPECT_FALSE((bool)converted);
    }
    {
        Error error;
        std::optional<Tileset> converted = convertJSON<Tileset>(
            R"JSON({
            "tiles": ["http://mytiles"],
            "bounds": "should not be a string"
        })JSON",
            error);

        EXPECT_FALSE((bool)converted);
    }
}

TEST(Tileset, ValidWorldBounds) {
    Error error;
    std::optional<Tileset> converted = convertJSON<Tileset>(
        R"JSON({
        "tiles": ["http://mytiles"],
        "bounds": [-180, -90, 180, 90]
    })JSON",
        error);
    EXPECT_TRUE((bool)converted);
    EXPECT_EQ(converted->bounds, LatLngBounds::hull({90, -180}, {-90, 180}));
}

TEST(Tileset, PointBounds) {
    Error error;
    std::optional<Tileset> converted = convertJSON<Tileset>(
        R"JSON({
        "tiles": ["http://mytiles"],
        "bounds": [0, 0, 0, 0]
    })JSON",
        error);
    EXPECT_TRUE((bool)converted);
    EXPECT_EQ(converted->bounds, LatLngBounds::hull({0, 0}, {0, 0}));
}

TEST(Tileset, BoundsAreClamped) {
    Error error;
    std::optional<Tileset> converted = convertJSON<Tileset>(
        R"JSON({
        "tiles": ["http://mytiles"],
        "bounds": [-181.0000005,-90.000000006,180.00000000000006,91]
    })JSON",
        error);
    EXPECT_TRUE((bool)converted);
    EXPECT_EQ(converted->bounds, LatLngBounds::hull({90, -180}, {-90, 180}));
}

TEST(Tileset, FullConversion) {
    Error error;
    Tileset converted = *convertJSON<Tileset>(
        R"JSON({
        "tiles": ["http://mytiles"],
        "scheme": "xyz",
        "minzoom": 1,
        "maxzoom": 2,
        "attribution": "mapbox",
        "bounds": [-180, -73, -120, 73]
    })JSON",
        error);

    EXPECT_EQ(converted.tiles[0], "http://mytiles");
    EXPECT_EQ(converted.scheme, Tileset::Scheme::XYZ);
    EXPECT_EQ(converted.zoomRange.min, 1);
    EXPECT_EQ(converted.zoomRange.max, 2);
    EXPECT_EQ(converted.attribution, "mapbox");
    EXPECT_EQ(converted.bounds, LatLngBounds::hull({73, -180}, {-73, -120}));
}
