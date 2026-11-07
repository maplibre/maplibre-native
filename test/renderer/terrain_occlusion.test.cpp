#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/style/image.hpp>
#include <mln/style/style.hpp>
#include <mln/util/image.hpp>
#include <mln/util/projection.hpp>
#include <mln/util/run_loop.hpp>

#include <cmath>
#include <string>

using namespace mln;

namespace {

// Symbol occlusion behind 3D terrain, checked in pixels on a scene whose DEM is generated here, so
// it does not depend on fixture tiles (the terrain/occlusion-debug render test asks for DEM tiles
// no fixture set has). A flat plain at sea level with a 3000 m ridge across it 2-3 km north of the
// centre; the camera looks north at pitch 60 from ~5 km up. One red icon stands at the centre, in
// front of the ridge, and one 5 km north, behind it: the line of sight to it passes the ridge
// about 1 km up, so the depth pass must hide it.
constexpr double centreLat = 46.0;
constexpr double centreLon = 10.0;
constexpr double metresPerDegreeLat = 111195.0;
constexpr double ridgeSouthLat = centreLat + 2000.0 / metresPerDegreeLat;
constexpr double ridgeNorthLat = centreLat + 3000.0 / metresPerDegreeLat;
constexpr double ridgeMetres = 3000.0;
constexpr double behindLat = centreLat + 5000.0 / metresPerDegreeLat;
constexpr uint32_t demTileSize = 256;

// Terrain-RGB: elevation = -10000 + (R * 65536 + G * 256 + B) * 0.1
void encodeHeight(uint8_t* px, double metres) {
    const auto value = static_cast<uint32_t>(std::lround((metres + 10000.0) * 10.0));
    px[0] = static_cast<uint8_t>(value >> 16);
    px[1] = static_cast<uint8_t>((value >> 8) & 0xff);
    px[2] = static_cast<uint8_t>(value & 0xff);
    px[3] = 255;
}

std::string makeRidgeTile(const CanonicalTileID& id) {
    PremultipliedImage image({demTileSize, demTileSize});
    const double tiles = std::pow(2.0, id.z);
    for (uint32_t y = 0; y < demTileSize; ++y) {
        // Latitude of the texel row's centre
        const double worldY = (id.y + (y + 0.5) / demTileSize) / tiles;
        const double lat = util::rad2deg(std::atan(std::sinh(M_PI * (1.0 - 2.0 * worldY))));
        const double metres = lat >= ridgeSouthLat && lat <= ridgeNorthLat ? ridgeMetres : 0.0;
        for (uint32_t x = 0; x < demTileSize; ++x) {
            encodeHeight(image.data.get() + (y * demTileSize + x) * 4, metres);
        }
    }
    return encodePNG(image);
}

const std::string occlusionStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 12,
      "tileSize": 256
    },
    "icons": {
      "type": "geojson",
      "data": {
        "type": "FeatureCollection",
        "features": [
          {"type": "Feature", "properties": {}, "geometry": {"type": "Point", "coordinates": [10.0, 46.0]}},
          {"type": "Feature", "properties": {}, "geometry": {"type": "Point", "coordinates": [10.0, BEHIND_LAT]}}
        ]
      }
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": [
    {"id": "background", "type": "background", "paint": {"background-color": "white"}},
    {"id": "icons", "type": "symbol", "source": "icons",
     "layout": {"icon-image": "dot", "icon-allow-overlap": true, "icon-ignore-placement": true}}
  ]
})STYLE";

// Whether any pixel within `radius` of `point` is the icons' red
bool redNear(const PremultipliedImage& image, const ScreenCoordinate& point, int radius) {
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            const int x = static_cast<int>(std::lround(point.x)) + dx;
            const int y = static_cast<int>(std::lround(point.y)) + dy;
            if (x < 0 || y < 0 || x >= static_cast<int>(image.size.width) || y >= static_cast<int>(image.size.height)) {
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

TEST(TerrainOcclusion, AnIconBehindARidgeIsHidden) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    fileSource->tileResponse = [](const Resource& resource) {
        Response res;
        res.data = std::make_shared<std::string>(makeRidgeTile({static_cast<uint8_t>(resource.tileData->z),
                                                                static_cast<uint32_t>(resource.tileData->x),
                                                                static_cast<uint32_t>(resource.tileData->y)}));
        return res;
    };
    HeadlessFrontend frontend{{512, 512}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));

    std::string style = occlusionStyle;
    style.replace(style.find("BEHIND_LAT"), std::string("BEHIND_LAT").size(), std::to_string(behindLat));
    map.getStyle().loadJSON(style);
    PremultipliedImage dot({12, 12});
    for (uint32_t i = 0; i < 12 * 12; ++i) {
        dot.data[i * 4 + 0] = 255;
        dot.data[i * 4 + 1] = 0;
        dot.data[i * 4 + 2] = 0;
        dot.data[i * 4 + 3] = 255;
    }
    map.getStyle().addImage(std::make_unique<style::Image>("dot", std::move(dot), 1.0f));
    map.jumpTo(
        CameraOptions().withCenter(LatLng{centreLat, centreLon}).withZoom(12.0).withPitch(60.0).withBearing(0.0));

    PremultipliedImage image;
    for (int i = 0; i < 4; ++i) {
        loop.runOnce();
        image = frontend.render(map).image;
    }

    // Both stand on sea-level ground, so their sea-level positions are where they are drawn
    const ScreenCoordinate front = map.pixelForLatLng(LatLng{centreLat, centreLon});
    const ScreenCoordinate behind = map.pixelForLatLng(LatLng{behindLat, centreLon});
    ASSERT_GT(front.y - behind.y, 20.0); // the far icon sits higher up the screen, inside it
    ASSERT_GT(behind.y, 10.0);

    EXPECT_TRUE(redNear(image, front, 4)) << "the icon in front of the ridge is drawn";
    EXPECT_FALSE(redNear(image, behind, 4)) << "the icon behind the ridge is hidden";
}
