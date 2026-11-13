#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/style/image.hpp>
#include <mln/style/sources/raster_dem_source.hpp>
#include <mln/style/style.hpp>
#include <mln/style/terrain.hpp>
#include <mln/util/image.hpp>
#include <mln/util/projection.hpp>
#include <mln/util/run_loop.hpp>
#include <mln/util/tileset.hpp>

#include <cmath>
#include <cstdlib>
#include <memory>
#include <string>
#include <utility>

using namespace mln;

namespace {

// A plain at sea level with a 3000 m ridge across it 2-3 km north of the centre, and a band draped
// over the ridge. Seen from the south at pitch 60, the band stands higher on the screen the more the
// terrain is exaggerated, and at full height the ridge hides a point 5 km north of the centre.
constexpr double centreLat = 46.0;
constexpr double centreLon = 10.0;
constexpr double metresPerDegreeLat = 111195.0;
constexpr double ridgeSouthLat = centreLat + 2000.0 / metresPerDegreeLat;
constexpr double ridgeNorthLat = centreLat + 3000.0 / metresPerDegreeLat;
constexpr double behindLat = centreLat + 5000.0 / metresPerDegreeLat;
constexpr uint32_t demTileSize = 256;

// Terrain-RGB: elevation = -10000 + (R * 65536 + G * 256 + B) * 0.1
// Terrarium: elevation = R * 256 + G + B / 256 - 32768
std::string makeRidgeTile(const CanonicalTileID& id, bool terrarium) {
    PremultipliedImage image({demTileSize, demTileSize});
    const double tiles = std::pow(2.0, id.z);
    for (uint32_t y = 0; y < demTileSize; ++y) {
        const double worldY = (id.y + (y + 0.5) / demTileSize) / tiles;
        const double lat = util::rad2deg(std::atan(std::sinh(M_PI * (1.0 - 2.0 * worldY))));
        const double metres = lat >= ridgeSouthLat && lat <= ridgeNorthLat ? 3000.0 : 0.0;
        const auto value = terrarium ? static_cast<uint32_t>(std::lround((metres + 32768.0) * 256.0))
                                     : static_cast<uint32_t>(std::lround((metres + 10000.0) * 10.0));
        for (uint32_t x = 0; x < demTileSize; ++x) {
            uint8_t* px = image.data.get() + (y * demTileSize + x) * 4;
            px[0] = static_cast<uint8_t>(value >> 16);
            px[1] = static_cast<uint8_t>((value >> 8) & 0xff);
            px[2] = static_cast<uint8_t>(value & 0xff);
            px[3] = 255;
        }
    }
    return encodePNG(image);
}

std::string ridgeStyle(float exaggeration, const std::string& encoding, bool icons) {
    return R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/)STYLE" +
           encoding + R"STYLE(/{z}-{x}-{y}.png"],
      "encoding": ")STYLE" +
           encoding + R"STYLE(",
      "maxzoom": 12,
      "tileSize": 256
    },
    "icon": {"type": "geojson", "data": {"type": "Point", "coordinates": [10.0, )STYLE" +
           std::to_string(behindLat) + R"STYLE(]}},
    "band": {
      "type": "geojson",
      "data": {"type": "Polygon", "coordinates": [[[9.9, )STYLE" +
           std::to_string(ridgeSouthLat) + "], [10.1, " + std::to_string(ridgeSouthLat) + "], [10.1, " +
           std::to_string(ridgeNorthLat) + "], [9.9, " + std::to_string(ridgeNorthLat) + "], [9.9, " +
           std::to_string(ridgeSouthLat) + R"STYLE(]]]}
    }
  },
  "terrain": {"source": "dem", "exaggeration": )STYLE" +
           std::to_string(exaggeration) + R"STYLE(},
  "layers": [
    {"id": "background", "type": "background", "paint": {"background-color": "white"}},
    {"id": "band", "type": "fill", "source": "band", "paint": {"fill-color": ")STYLE" +
           (icons ? "blue" : "red") + R"STYLE("}})STYLE" +
           (icons ? R"STYLE(,
    {"id": "icon", "type": "symbol", "source": "icon",
     "layout": {"icon-image": "dot", "icon-allow-overlap": true, "icon-ignore-placement": true}})STYLE"
                  : "") +
           R"STYLE(
  ]
})STYLE";
}

struct RidgeMap {
    explicit RidgeMap(float exaggeration, const std::string& encoding = "mapbox", bool icons = false)
        : fileSource(std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions())),
          map(frontend,
              MapObserver::nullObserver(),
              fileSource,
              MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize())) {
        fileSource->tileResponse = [](const Resource& resource) {
            Response res;
            res.data = std::make_shared<std::string>(
                makeRidgeTile({static_cast<uint8_t>(resource.tileData->z),
                               static_cast<uint32_t>(resource.tileData->x),
                               static_cast<uint32_t>(resource.tileData->y)},
                              resource.url.find("terrarium") != std::string::npos));
            return res;
        };
        map.getStyle().loadJSON(ridgeStyle(exaggeration, encoding, icons));
        if (icons) {
            PremultipliedImage dot({12, 12});
            for (uint32_t i = 0; i < 12 * 12; ++i) {
                dot.data[i * 4 + 0] = 255;
                dot.data[i * 4 + 1] = 0;
                dot.data[i * 4 + 2] = 0;
                dot.data[i * 4 + 3] = 255;
            }
            map.getStyle().addImage(std::make_unique<style::Image>("dot", std::move(dot), 1.0f));
        }
        map.jumpTo(CameraOptions().withCenter(LatLng{centreLat, centreLon}).withZoom(12.0).withPitch(60.0));
    }

    HeadlessFrontend::RenderResult settle() {
        HeadlessFrontend::RenderResult result;
        for (int i = 0; i < 4; ++i) {
            loop.runOnce();
            result = frontend.render(map);
        }
        return result;
    }

    util::RunLoop loop;
    std::shared_ptr<StubFileSource> fileSource;
    HeadlessFrontend frontend{{512, 512}, 1};
    MapAdapter map;
};

size_t differingPixels(const PremultipliedImage& a, const PremultipliedImage& b) {
    size_t count = 0;
    for (size_t i = 0; i < a.bytes(); i += 4) {
        if (std::abs(a.data[i] - b.data[i]) > 16 || std::abs(a.data[i + 1] - b.data[i + 1]) > 16 ||
            std::abs(a.data[i + 2] - b.data[i + 2]) > 16) {
            ++count;
        }
    }
    return count;
}

// Whether any pixel within four of `point` is the icon's red
bool redNear(const PremultipliedImage& image, const ScreenCoordinate& point) {
    for (int dy = -4; dy <= 4; ++dy) {
        for (int dx = -4; dx <= 4; ++dx) {
            const int x = static_cast<int>(std::lround(point.x)) + dx;
            const int y = static_cast<int>(std::lround(point.y)) + dy;
            if (x < 0 || y < 0 || std::cmp_greater_equal(x, image.size.width) ||
                std::cmp_greater_equal(y, image.size.height)) {
                continue;
            }
            const uint8_t* px = image.data.get() + (static_cast<size_t>(y) * image.size.width + x) * 4;
            if (px[0] > 200 && px[1] < 80 && px[2] < 80) {
                return true;
            }
        }
    }
    return false;
}

} // namespace

// The mesh, the DEM textures and the drape targets do not depend on the exaggeration, so a new
// value is taken as it is: nothing is built again, and the frame is the one a map loaded with that
// value draws.
TEST(TerrainExaggeration, AChangeKeepsWhatTheTerrainBuilt) {
    RidgeMap fresh(1.0f);
    const PremultipliedImage expected = fresh.settle().image;

    RidgeMap changed(0.2f);
    const auto low = changed.settle();
    ASSERT_GT(differingPixels(low.image, expected), 2000u) << "the two exaggerations draw the band apart";

    changed.map.getStyle().getTerrain()->setExaggeration(1.0f);
    const auto raised = changed.frontend.render(changed.map);
    EXPECT_EQ(raised.stats.numCreatedTextures, low.stats.numCreatedTextures);
    EXPECT_LT(differingPixels(raised.image, expected), 50u);
}

// Stepping the exaggeration up frame by frame, as a rise out of the flat map does, ends on the same frame.
TEST(TerrainExaggeration, ARiseInStepsEndsWhereAJumpDoes) {
    RidgeMap fresh(1.0f);
    const PremultipliedImage expected = fresh.settle().image;

    RidgeMap rising(0.0f);
    const auto flat = rising.settle();
    HeadlessFrontend::RenderResult frame;
    for (int step = 1; step <= 10; ++step) {
        rising.map.getStyle().getTerrain()->setExaggeration(static_cast<float>(step) / 10.0f);
        frame = rising.frontend.render(rising.map);
    }
    EXPECT_EQ(frame.stats.numCreatedTextures, flat.stats.numCreatedTextures);
    EXPECT_LT(differingPixels(frame.image, expected), 50u);
}

// The pass that hides symbols behind the terrain is drawn again for a new exaggeration, under a camera
// that has not moved.
TEST(TerrainExaggeration, ANewExaggerationHidesWhatTheGroundNowCovers) {
    RidgeMap ridge(0.02f, "mapbox", /*icons=*/true);
    const auto low = ridge.settle();
    const ScreenCoordinate behind = ridge.map.pixelForLatLng(LatLng{behindLat, centreLon});
    ASSERT_TRUE(redNear(low.image, behind)) << "a 60 m ridge hides nothing";

    ridge.map.getStyle().getTerrain()->setExaggeration(1.0f);
    EXPECT_FALSE(redNear(ridge.frontend.render(ridge.map).image, behind)) << "the 3000 m ridge hides the icon";
}

// A source put in under the same name is another DEM. Its tiles may be encoded differently, so nothing
// built from the old one may stay.
TEST(TerrainExaggeration, ASourceSwappedInUnderTheSameNameStartsAgain) {
    RidgeMap fresh(1.0f, "mapbox");
    const PremultipliedImage expected = fresh.settle().image;

    RidgeMap swapped(1.0f, "terrarium");
    ASSERT_LT(differingPixels(swapped.settle().image, expected), 50u) << "both encodings carry the same ridge";

    auto& style = swapped.map.getStyle();
    style.removeSource("dem");
    Tileset tileset;
    tileset.tiles = {"http://example.com/mapbox/{z}-{x}-{y}.png"};
    tileset.zoomRange = {0, 12};
    tileset.rasterEncoding = Tileset::RasterEncoding::Mapbox;
    style.addSource(std::make_unique<style::RasterDEMSource>("dem", tileset, 256));
    style.setTerrain(std::make_unique<style::Terrain>("dem", 1.0f));
    EXPECT_LT(differingPixels(swapped.settle().image, expected), 50u);
}
