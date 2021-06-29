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
const TileServerOptions mapboxTileServerOptions = TileServerOptions::MapboxConfiguration();
const TileServerOptions mapLibreTileServerOptions = TileServerOptions::MapLibreConfiguration();
const TileServerOptions mapTilerTileServerOptions = TileServerOptions::MapTilerConfiguration();
} // namespace fixture

TEST(Mapbox, SourceURL) {
    EXPECT_EQ(
        "https://api.mapbox.com/v4/user.map.json?access_token=key&secure",
        mbgl::util::mapbox::normalizeSourceURL(mapboxFixture::mapboxTileServerOptions, "mapbox://user.map", "key"));
    EXPECT_EQ(
        "https://api.example.com/v4/user.map.json?access_token=key&secure",
        mbgl::util::mapbox::normalizeSourceURL(TileServerOptions(mapboxFixture::mapboxTileServerOptions).withBaseURL("https://api.example.com"), "mapbox://user.map", "key"));
    EXPECT_EQ(
        "https://api.mapbox.com/v4/user.map.json?access_token=key&secure&style=mapbox://styles/mapbox/streets-v9@0",
        mbgl::util::mapbox::normalizeSourceURL(mapboxFixture::mapboxTileServerOptions, "mapbox://user.map?style=mapbox://styles/mapbox/streets-v9@0", "key"));
    EXPECT_EQ(
        "https://api.mapbox.com/v4/user.map.json?access_token=key&secure",
        mbgl::util::mapbox::normalizeSourceURL(mapboxFixture::mapboxTileServerOptions, "mapbox://user.map?", "key"));
    EXPECT_EQ(
        "http://path",
        mbgl::util::mapbox::normalizeSourceURL(mapboxFixture::mapboxTileServerOptions, "http://path", "key"));
    EXPECT_THROW(
        mbgl::util::mapbox::normalizeSourceURL(mapboxFixture::mapboxTileServerOptions, "mapbox://user.map", ""),
        std::runtime_error);
}

TEST(Mapbox, GlyphsURL) {
    EXPECT_EQ(
        "https://api.mapbox.com/fonts/v1/boxmap/Comic%20Sans/0-255.pbf?access_token=key",
        mbgl::util::mapbox::normalizeGlyphsURL(mapboxFixture::mapboxTileServerOptions, "mapbox://fonts/boxmap/Comic%20Sans/0-255.pbf", "key"));
    EXPECT_EQ(
        "https://api.example.com/fonts/v1/boxmap/Comic%20Sans/0-255.pbf?access_token=key",
              mbgl::util::mapbox::normalizeGlyphsURL(TileServerOptions(mapboxFixture::mapboxTileServerOptions).withBaseURL("https://api.example.com"), "mapbox://fonts/boxmap/Comic%20Sans/0-255.pbf", "key"));
    EXPECT_EQ(
        "https://api.mapbox.com/fonts/v1/boxmap/{fontstack}/{range}.pbf?access_token=key",
        mbgl::util::mapbox::normalizeGlyphsURL(mapboxFixture::mapboxTileServerOptions, "mapbox://fonts/boxmap/{fontstack}/{range}.pbf", "key"));
    EXPECT_EQ(
        "http://path",
        mbgl::util::mapbox::normalizeGlyphsURL(mapboxFixture::mapboxTileServerOptions, "http://path", "key"));
    EXPECT_EQ(
        "mapbox://path",
        mbgl::util::mapbox::normalizeGlyphsURL(mapboxFixture::mapboxTileServerOptions, "mapbox://path", "key"));
}

TEST(Mapbox, StyleURL) {
    EXPECT_EQ(
        "mapbox://foo",
        mbgl::util::mapbox::normalizeStyleURL(mapboxFixture::mapboxTileServerOptions, "mapbox://foo", "key"));
    EXPECT_EQ(
        "https://api.mapbox.com/styles/v1/user/style?access_token=key",
        mbgl::util::mapbox::normalizeStyleURL(mapboxFixture::mapboxTileServerOptions, "mapbox://styles/user/style", "key"));
    EXPECT_EQ(
        "https://api.example.com/styles/v1/user/style?access_token=key",
              mbgl::util::mapbox::normalizeStyleURL(TileServerOptions(mapboxFixture::mapboxTileServerOptions).withBaseURL("https://api.example.com"), "mapbox://styles/user/style", "key"));
    EXPECT_EQ(
        "https://api.mapbox.com/styles/v1/user/style/draft?access_token=key",
        mbgl::util::mapbox::normalizeStyleURL(mapboxFixture::mapboxTileServerOptions, "mapbox://styles/user/style/draft", "key"));
    EXPECT_EQ(
        "https://api.mapbox.com/styles/v1/user/style?access_token=key&shave=true",
        mbgl::util::mapbox::normalizeStyleURL(mapboxFixture::mapboxTileServerOptions, "mapbox://styles/user/style?shave=true", "key"));
    EXPECT_EQ(
        "https://api.mapbox.com/styles/v1/user/style?access_token=key",
        mbgl::util::mapbox::normalizeStyleURL(mapboxFixture::mapboxTileServerOptions, "mapbox://styles/user/style?", "key"));
    EXPECT_EQ(
        "http://path",
        mbgl::util::mapbox::normalizeStyleURL(mapboxFixture::mapboxTileServerOptions, "http://path", "key"));
}

TEST(Mapbox, SpriteURL) {
    EXPECT_EQ(
        "map/box/sprites@2x.json",
        mbgl::util::mapbox::normalizeSpriteURL(mapboxFixture::mapboxTileServerOptions, "map/box/sprites@2x.json", "key"));
    EXPECT_EQ(
        "mapbox://foo",
        mbgl::util::mapbox::normalizeSpriteURL(mapboxFixture::mapboxTileServerOptions, "mapbox://foo", "key"));
    EXPECT_EQ(
        "https://api.mapbox.com/styles/v1/mapbox/streets-v8/sprite.json?access_token=key",
        mbgl::util::mapbox::normalizeSpriteURL(mapboxFixture::mapboxTileServerOptions, "mapbox://sprites/mapbox/streets-v8.json", "key"));
    EXPECT_EQ(
        "https://api.example.com/styles/v1/mapbox/streets-v8/sprite.json?access_token=key",
              mbgl::util::mapbox::normalizeSpriteURL(TileServerOptions(mapboxFixture::mapboxTileServerOptions).withBaseURL("https://api.example.com"), "mapbox://sprites/mapbox/streets-v8.json", "key"));
    EXPECT_EQ(
        "https://api.mapbox.com/styles/v1/mapbox/streets-v8/sprite@2x.png?access_token=key",
        mbgl::util::mapbox::normalizeSpriteURL(mapboxFixture::mapboxTileServerOptions, "mapbox://sprites/mapbox/streets-v8@2x.png", "key"));
    EXPECT_EQ(
        "https://api.mapbox.com/styles/v1/mapbox/streets-v8/draft/sprite@2x.png?access_token=key",
        mbgl::util::mapbox::normalizeSpriteURL(mapboxFixture::mapboxTileServerOptions, "mapbox://sprites/mapbox/streets-v8/draft@2x.png", "key"));
    EXPECT_EQ(
        "https://api.mapbox.com/styles/v1/mapbox/streets-v11/sprite?access_token=key&fresh=true.png",
        mbgl::util::mapbox::normalizeSpriteURL(mapboxFixture::mapboxTileServerOptions,
            "mapbox://sprites/mapbox/streets-v11?fresh=true.png",
            "key"));
    EXPECT_EQ("mapbox://////", mbgl::util::mapbox::normalizeSpriteURL(mapboxFixture::mapboxTileServerOptions, "mapbox://////", "key"));
}

TEST(Mapbox, TileURL) {
    EXPECT_EQ(
        "https://api.mapbox.com/v4/a.b/0/0/0.pbf?access_token=key",
        mbgl::util::mapbox::normalizeTileURL(mapboxFixture::mapboxTileServerOptions, "mapbox://tiles/a.b/0/0/0.pbf", "key"));
    EXPECT_EQ(
        "https://api.mapbox.com/v4/a.b/0/0/0.pbf?access_token=key&style=mapbox://styles/mapbox/streets-v9@0",
        mbgl::util::mapbox::normalizeTileURL(mapboxFixture::mapboxTileServerOptions, "mapbox://tiles/a.b/0/0/0.pbf?style=mapbox://styles/mapbox/streets-v9@0", "key"));
    EXPECT_EQ(
        "https://api.mapbox.com/v4/a.b/0/0/0.pbf?access_token=key",
        mbgl::util::mapbox::normalizeTileURL(mapboxFixture::mapboxTileServerOptions, "mapbox://tiles/a.b/0/0/0.pbf?", "key"));
    EXPECT_EQ(
        "https://api.mapbox.com/v4/a.b/0/0/0.png?access_token=key",
        mbgl::util::mapbox::normalizeTileURL(mapboxFixture::mapboxTileServerOptions, "mapbox://tiles/a.b/0/0/0.png", "key"));
    EXPECT_EQ(
        "https://api.example.com/v4/a.b/0/0/0.png?access_token=key",
              mbgl::util::mapbox::normalizeTileURL(TileServerOptions(mapboxFixture::mapboxTileServerOptions).withBaseURL("https://api.example.com"), "mapbox://tiles/a.b/0/0/0.png", "key"));
    EXPECT_EQ(
        "https://api.mapbox.com/v4/a.b/0/0/0@2x.png?access_token=key",
        mbgl::util::mapbox::normalizeTileURL(mapboxFixture::mapboxTileServerOptions, "mapbox://tiles/a.b/0/0/0@2x.png", "key"));
    EXPECT_EQ(
        "https://api.mapbox.com/v4/a.b,c.d/0/0/0.pbf?access_token=key",
        mbgl::util::mapbox::normalizeTileURL(mapboxFixture::mapboxTileServerOptions, "mapbox://tiles/a.b,c.d/0/0/0.pbf", "key"));
    EXPECT_EQ(
        "http://path",
        mbgl::util::mapbox::normalizeSpriteURL(mapboxFixture::mapboxTileServerOptions, "http://path", "key"));
}

TEST(Mapbox, CanonicalURL) {
    EXPECT_EQ(
        "mapbox://tiles/a.b/{z}/{x}/{y}.vector.pbf",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions, "http://a.tiles.mapbox.com/v4/a.b/{z}/{x}/{y}.vector.pbf", SourceType::Vector, 512));
    EXPECT_EQ(
        "mapbox://tiles/a.b/{z}/{x}/{y}.vector.pbf",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions, "http://b.tiles.mapbox.com/v4/a.b/{z}/{x}/{y}.vector.pbf", SourceType::Vector, 512));
    EXPECT_EQ(
        "mapbox://tiles/a.b/{z}/{x}/{y}.vector.pbf",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions, "http://api.mapbox.com/v4/a.b/{z}/{x}/{y}.vector.pbf", SourceType::Vector, 512));
    EXPECT_EQ(
        "mapbox://tiles/a.b/{z}/{x}/{y}.vector.pbf",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions,"http://api.mapbox.com/v4/a.b/{z}/{x}/{y}.vector.pbf?access_token=key", SourceType::Vector, 512));
    EXPECT_EQ(
        "mapbox://tiles/a.b/{z}/{x}/{y}.vector.pbf",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions,"https://api.mapbox.cn/v4/a.b/{z}/{x}/{y}.vector.pbf?access_token=key", SourceType::Vector, 512));
    EXPECT_EQ(
        "mapbox://tiles/a.b,c.d/{z}/{x}/{y}.vector.pbf",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions,"http://api.mapbox.com/v4/a.b,c.d/{z}/{x}/{y}.vector.pbf?access_token=key", SourceType::Vector, 512));
    EXPECT_EQ(
        "mapbox://tiles/a.b/{z}/{x}/{y}.vector.pbf?custom=parameter",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions,"http://a.tiles.mapbox.com/v4/a.b/{z}/{x}/{y}.vector.pbf?access_token=key&custom=parameter", SourceType::Vector, 512));
    EXPECT_EQ(
        "mapbox://tiles/a.b/{z}/{x}/{y}.vector.pbf?custom=parameter",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions, "http://a.tiles.mapbox.com/v4/a.b/{z}/{x}/{y}.vector.pbf?custom=parameter&access_token=key", SourceType::Vector, 512));
    EXPECT_EQ(
        "mapbox://tiles/a.b/{z}/{x}/{y}.vector.pbf?custom=parameter&second=param",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions, "http://a.tiles.mapbox.com/v4/a.b/{z}/{x}/{y}.vector.pbf?custom=parameter&access_token=key&second=param", SourceType::Vector, 512));
    EXPECT_EQ(
        "mapbox://tiles/a.b/{z}/{x}/{y}{ratio}.jpg",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions, "http://api.mapbox.com/v4/a.b/{z}/{x}/{y}.jpg?access_token=key", SourceType::Raster, 256));
    EXPECT_EQ(
        "mapbox://tiles/a.b/{z}/{x}/{y}{ratio}.jpg70",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions, "http://api.mapbox.com/v4/a.b/{z}/{x}/{y}.jpg70?access_token=key", SourceType::Raster, 256));
    EXPECT_EQ(
        "mapbox://tiles/a.b/{z}/{x}/{y}@2x.jpg",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions, "http://api.mapbox.com/v4/a.b/{z}/{x}/{y}.jpg?access_token=key", SourceType::Raster, 512));
    EXPECT_EQ(
        "mapbox://tiles/a.b/{z}/{x}/{y}@2x.jpg70",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions, "http://api.mapbox.com/v4/a.b/{z}/{x}/{y}.jpg70?access_token=key", SourceType::Raster, 512));
    EXPECT_EQ(
        "mapbox://tiles/a.b/{z}/{x}/{y}{ratio}.png",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions, "http://api.mapbox.com/v4/a.b/{z}/{x}/{y}.png", SourceType::Raster, 256));
    EXPECT_EQ(
        "mapbox://tiles/a.b/{z}/{x}/{y}{ratio}.png",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions, "http://api.mapbox.com/v4/a.b/{z}/{x}/{y}.png?access_token=key", SourceType::Raster, 256));
    EXPECT_EQ(
        "mapbox://tiles/a.b/{z}/{x}/{y}@2x.png",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions, "http://api.mapbox.com/v4/a.b/{z}/{x}/{y}.png", SourceType::Raster, 512));
    EXPECT_EQ(
        "mapbox://tiles/a.b/{z}/{x}/{y}@2x.png",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions, "http://api.mapbox.com/v4/a.b/{z}/{x}/{y}.png?access_token=key", SourceType::Raster, 512));

    // We don't ever expect to see these inputs, but be safe anyway.
    EXPECT_EQ(
        "",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions, "", SourceType::Raster, 256));
    EXPECT_EQ(
        "http://path",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions, "http://path", SourceType::Raster, 256));
    EXPECT_EQ(
        "http://api.mapbox.com/v4/",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions, "http://api.mapbox.com/v4/", SourceType::Raster, 256));
    EXPECT_EQ(
        "http://api.mapbox.com/v4/a.b/{z}/{x}/{y}.",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions, "http://api.mapbox.com/v4/a.b/{z}/{x}/{y}.", SourceType::Raster, 256));
    EXPECT_EQ(
        "http://api.mapbox.com/v4/a.b/{z}/{x}/{y}/.",
        mbgl::util::mapbox::canonicalizeTileURL(mapboxFixture::mapboxTileServerOptions, "http://api.mapbox.com/v4/a.b/{z}/{x}/{y}/.", SourceType::Raster, 256));
}

TEST(Mapbox, CanonicalizeRasterTileset) {
    mbgl::Tileset tileset;
    tileset.tiles = {
        "http://a.tiles.mapbox.com/v4/mapbox.satellite/{z}/{x}/{y}.png?access_token=key"
    };

    mbgl::util::mapbox::canonicalizeTileset(mapboxFixture::mapboxTileServerOptions, tileset, "mapbox://mapbox.satellite", SourceType::Raster, 256);

    EXPECT_EQ("mapbox://tiles/mapbox.satellite/{z}/{x}/{y}{ratio}.png", tileset.tiles[0]);
}

TEST(Mapbox, CanonicalizeVectorTileset) {
    mbgl::Tileset tileset;
    tileset.tiles = {
        "http://a.tiles.mapbox.com/v4/mapbox.streets/{z}/{x}/{y}.vector.pbf?access_token=key"
    };

    mbgl::util::mapbox::canonicalizeTileset(mapboxFixture::mapboxTileServerOptions, tileset, "mapbox://mapbox.streets", SourceType::Vector, 512);

    EXPECT_EQ("mapbox://tiles/mapbox.streets/{z}/{x}/{y}.vector.pbf", tileset.tiles[0]);
}

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
