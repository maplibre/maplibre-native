#include <mbgl/test/util.hpp>

#include <mbgl/util/logging.hpp>
#include <mbgl/util/mapbox.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/tileset.hpp>
#include <mbgl/util/tile_server_options.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <stdexcept>

using namespace mbgl;
using SourceType = mbgl::style::SourceType;

namespace mapboxFixture {
const TileServerOptions mapLibreTileServerOptions = TileServerOptions::MapLibreConfiguration();
const TileServerOptions mapTilerTileServerOptions = TileServerOptions::MapTilerConfiguration();
} // namespace fixture

// MapLibre tests
TEST(MapLibre, CanonicalURL) {
    EXPECT_EQ(
        "https://demotiles.maplibre.org/style.json",
        mbgl::util::mapbox::normalizeStyleURL(mapboxFixture::mapLibreTileServerOptions, "maplibre://maps/style", ""));
    EXPECT_EQ(
        "https://demotiles.maplibre.org/tiles/tiles.json",
        mbgl::util::mapbox::normalizeSourceURL(mapboxFixture::mapLibreTileServerOptions, "maplibre://tiles/tiles", ""));
    EXPECT_EQ(
        "https://demotiles.maplibre.org/font/{fontstack}/{start}-{end}.pbf",
        mbgl::util::mapbox::normalizeGlyphsURL(mapboxFixture::mapLibreTileServerOptions, "maplibre://fonts/{fontstack}/{start}-{end}.pbf", ""));

    EXPECT_EQ(
        "maplibre://tiles/tiles/{z}/{x}/{y}.pbf",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapLibreTileServerOptions, "https://demotiles.maplibre.org/tiles/{z}/{x}/{y}.pbf", SourceType::Vector, 512));
}

// MapTiler tests
TEST(MapTiler, StyleURL) {
    //Styles
    EXPECT_EQ(
        "https://api.maptiler.com/maps/basic/style.json",
        mbgl::util::mapbox::normalizeStyleURL(mapboxFixture::mapTilerTileServerOptions, "maptiler://maps/basic", ""));
    EXPECT_EQ(
        "https://api.maptiler.com/maps/basic/style.json?key=abcdef",
        mbgl::util::mapbox::normalizeStyleURL(mapboxFixture::mapTilerTileServerOptions, "maptiler://maps/basic", "abcdef"));
    EXPECT_EQ(
        "",
        mbgl::util::mapbox::normalizeStyleURL(mapboxFixture::mapTilerTileServerOptions, "", ""));
    EXPECT_EQ(
        "https://dummy",
        mbgl::util::mapbox::normalizeSourceURL(mapboxFixture::mapTilerTileServerOptions, "https://dummy", ""));
}
        
// Sources
TEST(MapTiler, SourceURL) {
    EXPECT_EQ(
        "https://api.maptiler.com/tiles/v3/tiles.json?key=abcdef",
        mbgl::util::mapbox::normalizeSourceURL(mapboxFixture::mapTilerTileServerOptions, "maptiler://sources/v3", "abcdef"));
    EXPECT_EQ(
        "https://api.maptiler.com/tiles/outdoor/tiles.json?key=abcdef",
        mbgl::util::mapbox::normalizeSourceURL(mapboxFixture::mapTilerTileServerOptions, "maptiler://sources/outdoor", "abcdef"));
    EXPECT_EQ(
        "https://api.maptiler.com/tiles/7ac429c7-c96e-46dd-8c3e-13d48988986a/tiles.json?key=abcdef",
        mbgl::util::mapbox::normalizeSourceURL(mapboxFixture::mapTilerTileServerOptions, "maptiler://sources/7ac429c7-c96e-46dd-8c3e-13d48988986a", "abcdef"));

    EXPECT_EQ(
        "maptiler://sources/v3",
        mbgl::util::mapbox::canonicalizeSourceURL(mapboxFixture::mapTilerTileServerOptions, "https://api.maptiler.com/tiles/v3/tiles.json?key=abcdef"));
    EXPECT_EQ(
        "maptiler://sources/outdoor",
        mbgl::util::mapbox::canonicalizeSourceURL(mapboxFixture::mapTilerTileServerOptions, "https://api.maptiler.com/tiles/outdoor/tiles.json?key=abcdef"));
    EXPECT_EQ(
        "maptiler://sources/7ac429c7-c96e-46dd-8c3e-13d48988986a",
        mbgl::util::mapbox::canonicalizeSourceURL(mapboxFixture::mapTilerTileServerOptions, "https://api.maptiler.com/tiles/7ac429c7-c96e-46dd-8c3e-13d48988986a/tiles.json?key=abcdef"));
}

TEST(MapTiler, SourceURLPassThrough) {
    EXPECT_EQ(
        "",
        mbgl::util::mapbox::normalizeSourceURL(mapboxFixture::mapTilerTileServerOptions, "", ""));
    EXPECT_EQ(
        "http://dummy",
        mbgl::util::mapbox::normalizeSourceURL(mapboxFixture::mapTilerTileServerOptions, "http://dummy", ""));
    EXPECT_EQ(
        "https://api.tileserver.com/map?key=1234",
        mbgl::util::mapbox::normalizeSourceURL(mapboxFixture::mapTilerTileServerOptions, "https://api.tileserver.com/map?key=1234", ""));
    EXPECT_EQ(
        "http://dummy",
        mbgl::util::mapbox::canonicalizeSourceURL(mapboxFixture::mapTilerTileServerOptions, "http://dummy"));
    EXPECT_EQ(
        "https://api.tileserver.com/map?key=1234",
        mbgl::util::mapbox::canonicalizeSourceURL(mapboxFixture::mapTilerTileServerOptions, "https://api.tileserver.com/map?key=1234"));
    EXPECT_EQ(
        "",
        mbgl::util::mapbox::canonicalizeSourceURL(mapboxFixture::mapTilerTileServerOptions, ""));
}

TEST(MapTiler, GlyphsURL) {
    // Glyphs
    EXPECT_EQ(
        "https://api.maptiler.com/fonts/{fontstack}/{range}.pbf",
        mbgl::util::mapbox::normalizeGlyphsURL(mapboxFixture::mapTilerTileServerOptions, "maptiler://fonts/{fontstack}/{range}.pbf", "")); 
    EXPECT_EQ(
        "https://api.maptiler.com/fonts/{fontstack}/{range}.pbf?key=abcdef",
        mbgl::util::mapbox::normalizeGlyphsURL(mapboxFixture::mapTilerTileServerOptions, "maptiler://fonts/{fontstack}/{range}.pbf", "abcdef"));
    
    EXPECT_EQ(
        "maptiler://fonts/{fontstack}/{range}.pbf",
        mbgl::util::mapbox::canonicalizeGlyphURL(mapboxFixture::mapTilerTileServerOptions, "https://api.maptiler.com/fonts/{fontstack}/{range}.pbf"));
    EXPECT_EQ(
        "maptiler://fonts/{fontstack}/{range}.pbf",
        mbgl::util::mapbox::canonicalizeGlyphURL(mapboxFixture::mapTilerTileServerOptions, "https://api.maptiler.com/fonts/{fontstack}/{range}.pbf?key=abcdef"));
}

TEST(MapTiler, GlyphsURLPassThrough) {
    EXPECT_EQ(
        "",
        mbgl::util::mapbox::normalizeGlyphsURL(mapboxFixture::mapTilerTileServerOptions, "", ""));
    EXPECT_EQ(
        "http://dummy",
        mbgl::util::mapbox::normalizeGlyphsURL(mapboxFixture::mapTilerTileServerOptions, "http://dummy", ""));
    EXPECT_EQ(
        "https://api.tileserver.com/map?key=1234",
        mbgl::util::mapbox::normalizeGlyphsURL(mapboxFixture::mapTilerTileServerOptions, "https://api.tileserver.com/map?key=1234", ""));
    EXPECT_EQ(
        "http://dummy",
        mbgl::util::mapbox::canonicalizeGlyphURL(mapboxFixture::mapTilerTileServerOptions, "http://dummy"));
    EXPECT_EQ(
        "https://api.tileserver.com/map?key=1234",
        mbgl::util::mapbox::canonicalizeGlyphURL(mapboxFixture::mapTilerTileServerOptions, "https://api.tileserver.com/map?key=1234"));
    EXPECT_EQ(
        "",
        mbgl::util::mapbox::canonicalizeGlyphURL(mapboxFixture::mapTilerTileServerOptions, ""));
}

TEST(MapTiler, Sprites) {
    // Sprites
    EXPECT_EQ(
        "https://api.maptiler.com/maps/streets/sprite",
        mbgl::util::mapbox::normalizeSpriteURL(mapboxFixture::mapTilerTileServerOptions, "maptiler://sprites/streets/sprite", ""));
    // Sprites
    EXPECT_EQ(
        "maptiler://sprites/streets/sprite",
        mbgl::util::mapbox::canonicalizeSpriteURL(mapboxFixture::mapTilerTileServerOptions, "https://api.maptiler.com/maps/streets/sprite"));
}

TEST(MapTiler, SpritesURLPassThrough) {
    EXPECT_EQ(
        "",
        mbgl::util::mapbox::normalizeSpriteURL(mapboxFixture::mapTilerTileServerOptions, "", ""));
    EXPECT_EQ(
        "http://dummy",
        mbgl::util::mapbox::normalizeSpriteURL(mapboxFixture::mapTilerTileServerOptions, "http://dummy", ""));
    EXPECT_EQ(
        "https://api.tileserver.com/map?key=1234",
        mbgl::util::mapbox::normalizeSpriteURL(mapboxFixture::mapTilerTileServerOptions, "https://api.tileserver.com/map?key=1234", ""));
    EXPECT_EQ(
        "http://dummy",
        mbgl::util::mapbox::canonicalizeSpriteURL(mapboxFixture::mapTilerTileServerOptions, "http://dummy"));
    EXPECT_EQ(
        "https://api.tileserver.com/map?key=1234",
        mbgl::util::mapbox::canonicalizeSpriteURL(mapboxFixture::mapTilerTileServerOptions, "https://api.tileserver.com/map?key=1234"));
    EXPECT_EQ(
        "",
        mbgl::util::mapbox::canonicalizeSpriteURL(mapboxFixture::mapTilerTileServerOptions, ""));
}
    
TEST(MapTiler, Tiles) {
    // Tiles
    EXPECT_EQ(
        "https://api.maptiler.com/tiles/contours/{z}/{x}/{y}.pbf?key=abcdef",
        mbgl::util::mapbox::normalizeTileURL(mapboxFixture::mapTilerTileServerOptions, "maptiler://tiles/tiles/contours/{z}/{x}/{y}.pbf", "abcdef"));
    EXPECT_EQ(
        "https://api.maptiler.com/tiles/v3/{z}/{x}/{y}.pbf?key=abcdef",
        mbgl::util::mapbox::normalizeTileURL(mapboxFixture::mapTilerTileServerOptions, "maptiler://tiles/tiles/v3/{z}/{x}/{y}.pbf", "abcdef"));
    EXPECT_EQ(
        "maptiler://tiles/tiles/contours/{z}/{x}/{y}.pbf",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapTilerTileServerOptions, "https://api.maptiler.com/tiles/contours/{z}/{x}/{y}.pbf?key=abcdef", SourceType::Vector, 512));
}

TEST(MapTiler, TilesURLPassThrough) {
    EXPECT_EQ(
        "",
        mbgl::util::mapbox::normalizeTileURL(mapboxFixture::mapTilerTileServerOptions, "", ""));
    EXPECT_EQ(
        "http://dummy",
        mbgl::util::mapbox::normalizeTileURL(mapboxFixture::mapTilerTileServerOptions, "http://dummy", ""));
    EXPECT_EQ(
        "https://api.tileserver.com/map?key=1234",
        mbgl::util::mapbox::normalizeTileURL(mapboxFixture::mapTilerTileServerOptions, "https://api.tileserver.com/map?key=1234", ""));
    EXPECT_EQ(
        "http://dummy",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapTilerTileServerOptions, "http://dummy", SourceType::Vector, 512));
    EXPECT_EQ(
        "https://api.tileserver.com/map?key=1234",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapTilerTileServerOptions, "https://api.tileserver.com/map?key=1234", SourceType::Vector, 512));
    EXPECT_EQ(
        "",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapTilerTileServerOptions, "", SourceType::Vector, 512));
}

TEST(MapTiler, CanonicalizeRasterTileset) {
    mbgl::Tileset tileset;
    tileset.tiles = {
        "https://api.maptiler.com/tiles/satellite/{z}/{x}/{y}.jpg?key=abcdef"
    };

    mbgl::util::mapbox::canonicalizeTileset(mapboxFixture::mapTilerTileServerOptions, tileset, "maptiler://tiles/satellite", SourceType::Raster, 256);

    EXPECT_EQ("maptiler://tiles/tiles/satellite/{z}/{x}/{y}{ratio}.jpg", tileset.tiles[0]);
}

TEST(MapTiler, CanonicalizeVectorTileset) {
    mbgl::Tileset tileset;
    tileset.tiles = {
        "https://api.maptiler.com/tiles/v3/{z}/{x}/{y}.pbf?key=abcdef"
    };

    mbgl::util::mapbox::canonicalizeTileset(mapboxFixture::mapTilerTileServerOptions, tileset, "maptiler://tiles/streets", SourceType::Vector, 512);

    EXPECT_EQ("maptiler://tiles/tiles/v3/{z}/{x}/{y}.pbf", tileset.tiles[0]);
}
