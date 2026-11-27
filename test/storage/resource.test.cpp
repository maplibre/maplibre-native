#include <mln/storage/resource.hpp>

#include <gtest/gtest.h>

#include <array>
#include <sstream>
#include <string>

TEST(Resource, Style) {
    using namespace mln;
    Resource resource = Resource::style("http://example.com");
    EXPECT_EQ(Resource::Kind::Style, resource.kind);
    EXPECT_EQ("http://example.com", resource.url);
}

TEST(Resource, Source) {
    using namespace mln;
    Resource resource = Resource::source("http://example.com");
    EXPECT_EQ(Resource::Kind::Source, resource.kind);
    EXPECT_EQ("http://example.com", resource.url);
}

TEST(Resource, Tile) {
    using namespace mln;

    Resource rasterTile = Resource::tile(
        "http://example.com/{z}/{x}/{y}{ratio}.png", 2.0, 1, 2, 3, Tileset::Scheme::XYZ);
    EXPECT_EQ(Resource::Kind::Tile, rasterTile.kind);
    EXPECT_EQ("http://example.com/3/1/2@2x.png", rasterTile.url);
    EXPECT_EQ("http://example.com/{z}/{x}/{y}{ratio}.png", rasterTile.tileData->urlTemplate);
    EXPECT_EQ(2, rasterTile.tileData->pixelRatio);
    EXPECT_EQ(1, rasterTile.tileData->x);
    EXPECT_EQ(2, rasterTile.tileData->y);
    EXPECT_EQ(3, rasterTile.tileData->z);

    Resource vectorTile = Resource::tile(
        "http://example.com/{prefix}/{z}/{x}/{y}.mvt", 2.0, 1, 2, 3, Tileset::Scheme::XYZ);
    EXPECT_EQ(Resource::Kind::Tile, vectorTile.kind);
    EXPECT_EQ("http://example.com/12/3/1/2.mvt", vectorTile.url);
    EXPECT_EQ("http://example.com/{prefix}/{z}/{x}/{y}.mvt", vectorTile.tileData->urlTemplate);
    EXPECT_EQ(1, vectorTile.tileData->pixelRatio);
    EXPECT_EQ(1, vectorTile.tileData->x);
    EXPECT_EQ(2, vectorTile.tileData->y);
    EXPECT_EQ(3, vectorTile.tileData->z);

    Resource quadTile = Resource::tile("http://example.com/{quadkey}.png", 2.0, 0, 0, 1, Tileset::Scheme::XYZ);
    EXPECT_EQ(Resource::Kind::Tile, quadTile.kind);
    EXPECT_EQ("http://example.com/0.png", quadTile.url);
    EXPECT_EQ("http://example.com/{quadkey}.png", quadTile.tileData->urlTemplate);
    EXPECT_EQ(1, quadTile.tileData->pixelRatio);
    EXPECT_EQ(0, quadTile.tileData->x);
    EXPECT_EQ(0, quadTile.tileData->y);
    EXPECT_EQ(1, quadTile.tileData->z);

    quadTile = Resource::tile("http://example.com/{quadkey}.png", 2.0, 0, 0, 2, Tileset::Scheme::XYZ);
    EXPECT_EQ(Resource::Kind::Tile, quadTile.kind);
    EXPECT_EQ("http://example.com/00.png", quadTile.url);
    EXPECT_EQ("http://example.com/{quadkey}.png", quadTile.tileData->urlTemplate);
    EXPECT_EQ(1, quadTile.tileData->pixelRatio);
    EXPECT_EQ(0, quadTile.tileData->x);
    EXPECT_EQ(0, quadTile.tileData->y);
    EXPECT_EQ(2, quadTile.tileData->z);

    quadTile = Resource::tile("http://example.com/{quadkey}.png", 2.0, 1, 1, 2, Tileset::Scheme::XYZ);
    EXPECT_EQ(Resource::Kind::Tile, quadTile.kind);
    EXPECT_EQ("http://example.com/03.png", quadTile.url);
    EXPECT_EQ("http://example.com/{quadkey}.png", quadTile.tileData->urlTemplate);
    EXPECT_EQ(1, quadTile.tileData->pixelRatio);
    EXPECT_EQ(1, quadTile.tileData->x);
    EXPECT_EQ(1, quadTile.tileData->y);
    EXPECT_EQ(2, quadTile.tileData->z);

    quadTile = Resource::tile("http://example.com/{quadkey}.png", 2.0, 22914, 52870, 17, Tileset::Scheme::XYZ);
    EXPECT_EQ(Resource::Kind::Tile, quadTile.kind);
    EXPECT_EQ("http://example.com/02301322130000230.png", quadTile.url);
    EXPECT_EQ("http://example.com/{quadkey}.png", quadTile.tileData->urlTemplate);
    EXPECT_EQ(1, quadTile.tileData->pixelRatio);
    EXPECT_EQ(22914, quadTile.tileData->x);
    EXPECT_EQ(52870, quadTile.tileData->y);
    EXPECT_EQ(17, quadTile.tileData->z);

    // Test case confirmed by quadkeytools package
    // https://bitbucket.org/steele/quadkeytools/src/master/test/quadkey.js?fileviewer=file-view-default#quadkey.js-57
    quadTile = Resource::tile("http://example.com/{quadkey}.png", 2.0, 29, 3, 6, Tileset::Scheme::XYZ);
    EXPECT_EQ(Resource::Kind::Tile, quadTile.kind);
    EXPECT_EQ("http://example.com/011123.png", quadTile.url);
    EXPECT_EQ("http://example.com/{quadkey}.png", quadTile.tileData->urlTemplate);
    EXPECT_EQ(1, quadTile.tileData->pixelRatio);
    EXPECT_EQ(29, quadTile.tileData->x);
    EXPECT_EQ(3, quadTile.tileData->y);
    EXPECT_EQ(6, quadTile.tileData->z);

    Resource wmsTile = Resource::tile("http://example.com/?bbox={bbox-epsg-3857}", 2.0, 0, 0, 1, Tileset::Scheme::XYZ);
    EXPECT_EQ(Resource::Kind::Tile, wmsTile.kind);
    EXPECT_EQ("http://example.com/?bbox=-20037508.342789244,0,0,20037508.342789244", wmsTile.url);
    EXPECT_EQ("http://example.com/?bbox={bbox-epsg-3857}", wmsTile.tileData->urlTemplate);
    EXPECT_EQ(1, wmsTile.tileData->pixelRatio);
    EXPECT_EQ(0, wmsTile.tileData->x);
    EXPECT_EQ(0, wmsTile.tileData->y);
    EXPECT_EQ(1, wmsTile.tileData->z);

    Resource tmsTile = Resource::tile("http://example.com/{z}/{x}/{y}{ratio}.png", 1.0, 1, 2, 3, Tileset::Scheme::TMS);
    EXPECT_EQ(Resource::Kind::Tile, tmsTile.kind);
    EXPECT_EQ("http://example.com/3/1/5.png", tmsTile.url);
    EXPECT_EQ("http://example.com/{z}/{x}/{y}{ratio}.png", tmsTile.tileData->urlTemplate);
    EXPECT_EQ(1, tmsTile.tileData->pixelRatio);
    EXPECT_EQ(1, tmsTile.tileData->x);
    EXPECT_EQ(5, tmsTile.tileData->y);
    EXPECT_EQ(3, tmsTile.tileData->z);
}

TEST(Resource, TileBBox) {
    using namespace mln;
    // `{bbox}` is the tile's box in the map projection's tile matrix set, from the tile's own row in either scheme.
    // Each expected box is the one GL JS's `CanonicalTileID.url` makes for the same tile, scheme and tile matrix set.
    const auto url = [](const TileMatrixSet& tileMatrixSet, int32_t x, int32_t y, int8_t z, Tileset::Scheme scheme) {
        return Resource::tile("http://example.com/{z}/{x}/{y}?bbox={bbox}",
                              1.0,
                              x,
                              y,
                              z,
                              scheme,
                              Resource::LoadingMethod::All,
                              tileMatrixSet)
            .url;
    };
    // Apple clang on arm64 fuses each origin plus index times tile extent into one multiply-add, which skips the
    // product's rounding, so a number can move by half an ulp of a product as large as the extent. Next to an origin
    // that cancels most of the product that is many ulps of the result, so the bound scales with the extent.
    constexpr double relativeToExtent = 1e-15;
    const auto expectBBox = [&](const TileMatrixSet& tileMatrixSet,
                                int32_t x,
                                int32_t y,
                                int8_t z,
                                Tileset::Scheme scheme,
                                const std::string& path,
                                const std::array<double, 4>& box) {
        const std::string actual = url(tileMatrixSet, x, y, z, scheme);
        const std::string prefix = "http://example.com/" + path + "?bbox=";
        ASSERT_TRUE(actual.starts_with(prefix)) << actual;
        std::istringstream numbers(actual.substr(prefix.size()));
        for (const double expected : box) {
            std::string number;
            std::getline(numbers, number, ',');
            EXPECT_NEAR(expected, std::stod(number), tileMatrixSet.extentAtZoom0 * relativeToExtent) << actual;
        }
        EXPECT_TRUE(numbers.eof()) << actual;
    };
    const auto xyz = Tileset::Scheme::XYZ;
    const auto tms = Tileset::Scheme::TMS;

    expectBBox(mercatorTileMatrixSet, 0, 0, 1, xyz, "1/0/0", {-20037508.342789244, 0, 0, 20037508.342789244});
    expectBBox(mercatorTileMatrixSet,
               2,
               5,
               3,
               tms,
               "3/2/2",
               {-10018754.171394622, -10018754.171394622, -5009377.085697312, -5009377.085697312});
    expectBBox(mercatorTileMatrixSet,
               2047,
               1360,
               12,
               xyz,
               "12/2047/1360",
               {-9783.939620502293, 6721566.519285258, 0, 6731350.45890576});

    // NZTM2000Quad's corners are not whole meters.
    const TileMatrixSet nztm2000Quad{.origin = {-3260586.7284, 10438190.1652}, .extentAtZoom0 = 10018754.1714};
    const std::array<double, 4> nztmBox{1122618.2215875, 4176468.808075001, 1748790.3572999998, 4802640.943787501};
    expectBBox(nztm2000Quad, 7, 9, 4, xyz, "4/7/9", nztmBox);
    expectBBox(nztm2000Quad, 7, 9, 4, tms, "4/7/6", nztmBox);

    // Every number here is exact in binary, so a multiply-add gives the same answer and the strings must match GL JS's.
    const TileMatrixSet identity{.origin = {-90, 90}, .extentAtZoom0 = 180};
    EXPECT_EQ("http://example.com/1/1/0?bbox=0,0,90,90", url(identity, 1, 0, 1, xyz));
    EXPECT_EQ("http://example.com/1/1/1?bbox=0,0,90,90", url(identity, 1, 0, 1, tms));
    const TileMatrixSet epsg3413{.origin = {-4194304, 4194304}, .extentAtZoom0 = 8388608};
    EXPECT_EQ("http://example.com/4/7/6?bbox=-524288,-1048576,0,-524288", url(epsg3413, 7, 9, 4, tms));
}

TEST(Resource, Glyphs) {
    using namespace mln;
    Resource resource = Resource::glyphs("http://example.com/{fontstack}/{range}", {{"stack"}}, {0, 255});
    EXPECT_EQ(Resource::Kind::Glyphs, resource.kind);
    EXPECT_EQ("http://example.com/stack/0-255", resource.url);
}

TEST(Resource, SpriteImage) {
    using namespace mln;
    Resource resource = Resource::spriteImage("http://example.com/sprite", 2.0);
    EXPECT_EQ(Resource::Kind::SpriteImage, resource.kind);
    EXPECT_EQ("http://example.com/sprite@2x.png", resource.url);

    Resource paramResource = Resource::spriteImage("http://example.com/sprite?query=true", 2.0);
    EXPECT_EQ(Resource::Kind::SpriteImage, paramResource.kind);
    EXPECT_EQ("http://example.com/sprite@2x.png?query=true", paramResource.url);
}

TEST(Resource, Image) {
    using namespace mln;
    Resource resource = Resource::image("http://example.com/sprite.jpg");
    EXPECT_EQ(Resource::Kind::Image, resource.kind);
    EXPECT_EQ("http://example.com/sprite.jpg", resource.url);
}

TEST(Resource, SpriteJSON) {
    using namespace mln;
    Resource resource = Resource::spriteJSON("http://example.com/sprite", 2.0);
    EXPECT_EQ(Resource::Kind::SpriteJSON, resource.kind);
    EXPECT_EQ("http://example.com/sprite@2x.json", resource.url);

    Resource paramResource = Resource::spriteJSON("http://example.com/sprite?query=true", 2.0);
    EXPECT_EQ(Resource::Kind::SpriteJSON, paramResource.kind);
    EXPECT_EQ("http://example.com/sprite@2x.json?query=true", paramResource.url);
}
