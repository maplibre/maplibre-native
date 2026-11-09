#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/stub_map_observer.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/map/mode.hpp>
#include <mln/style/style.hpp>
#include <mln/util/image.hpp>
#include <mln/util/run_loop.hpp>

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <thread>

using namespace mln;

namespace {

// Flat ground under a vector tile source whose one polygon covers each tile whole. A polygon is red when
// its tile is of zoom 15 or deeper and blue when it is shallower, so a pixel says which level of tiles
// drew it. Terrain's mesh follows the view in the Quality load mode, so the drape targets here are far
// deeper than the tiles of the source: every tile the source draws is an ancestor of the targets it covers.
const char* levelStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/dem/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 11,
      "tileSize": 256
    },
    "level": {
      "type": "vector",
      "tiles": ["http://example.com/level/{z}-{x}-{y}.pbf"],
      "minzoom": 0,
      "maxzoom": 22
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": [
    {"id": "background", "type": "background", "paint": {"background-color": "#ffffff"}},
    {"id": "level", "type": "fill", "source": "level", "source-layer": "level",
     "paint": {"fill-color": ["case", [">=", ["get", "z"], 15], "#ff0000", "#0000ff"],
               "fill-antialias": false}}
  ]
})STYLE";

std::string demTile() {
    PremultipliedImage image({256, 256});
    for (uint32_t i = 0; i < 256 * 256; ++i) {
        uint8_t* px = image.data.get() + i * 4;
        // Sea level in Terrain-RGB: (0 + 10000) / 0.1 = 100000 = 1 * 65536 + 134 * 256 + 160
        px[0] = 1;
        px[1] = 134;
        px[2] = 160;
        px[3] = 255;
    }
    return encodePNG(image);
}

void varint(std::string& out, uint64_t value) {
    while (value >= 0x80) {
        out.push_back(static_cast<char>((value & 0x7f) | 0x80));
        value >>= 7;
    }
    out.push_back(static_cast<char>(value));
}

void bytesField(std::string& out, uint32_t number, const std::string& payload) {
    varint(out, (number << 3) | 2);
    varint(out, payload.size());
    out += payload;
}

void varintField(std::string& out, uint32_t number, uint64_t value) {
    varint(out, number << 3);
    varint(out, value);
}

uint32_t zigzag(int32_t n) {
    return (static_cast<uint32_t>(n) << 1) ^ static_cast<uint32_t>(n >> 31);
}

// A vector tile of one layer, "level", with one polygon over the whole tile whose property z is the zoom
// of the tile.
std::string levelTile(int zoom) {
    constexpr int32_t extent = 4096;
    std::string geometry;
    varint(geometry, (1 << 3) | 1); // MoveTo
    varint(geometry, zigzag(0));
    varint(geometry, zigzag(0));
    varint(geometry, (3 << 3) | 2); // LineTo, three points
    varint(geometry, zigzag(extent));
    varint(geometry, zigzag(0));
    varint(geometry, zigzag(0));
    varint(geometry, zigzag(extent));
    varint(geometry, zigzag(-extent));
    varint(geometry, zigzag(0));
    varint(geometry, (1 << 3) | 7); // ClosePath

    std::string tags;
    varint(tags, 0); // key 0, value 0
    varint(tags, 0);
    std::string feature;
    varintField(feature, 1, 1); // id
    bytesField(feature, 2, tags);
    varintField(feature, 3, 3); // polygon
    bytesField(feature, 4, geometry);

    std::string value;
    varintField(value, 4, static_cast<uint64_t>(zoom)); // int_value
    std::string layer;
    varintField(layer, 15, 2); // version
    bytesField(layer, 1, "level");
    bytesField(layer, 2, feature);
    bytesField(layer, 3, "z");
    bytesField(layer, 4, value);
    varintField(layer, 5, extent);

    std::string tile;
    bytesField(tile, 3, layer);
    return tile;
}

int tileZoomOf(const std::string& url) {
    const auto at = url.find("/level/");
    return at == std::string::npos ? -1 : std::stoi(url.substr(at + 7));
}

constexpr uint32_t viewSize = 256;
constexpr uint32_t compareFrom = 64;
constexpr uint32_t compareTo = 192;
constexpr int compareArea = (compareTo - compareFrom) * (compareTo - compareFrom);

// How many pixels of the compared region are mostly `channel` (0 red, 2 blue), and none of the others.
int mostly(const PremultipliedImage& image, int channel) {
    int count = 0;
    for (uint32_t y = compareFrom; y < compareTo; ++y) {
        for (uint32_t x = compareFrom; x < compareTo; ++x) {
            const uint8_t* px = image.data.get() + (y * image.size.width + x) * 4;
            const bool other = px[2 - channel] > 100 || px[1] > 100;
            if (px[channel] > 200 && !other) ++count;
        }
    }
    return count;
}

} // namespace

// Zooming out across a zoom level leaves the source's tiles of the level it left in the render set until
// the new level's have loaded, and a drape target bakes with them meanwhile: they are the closest tiles it
// has, one level above it. Once the new level's tiles are in, they are two levels above it, which counts as
// a worse fallback, and the target kept the bake of the level left for good: its content at the wrong
// scale (a hatch twice as fine, a tile of the finer level's picture). It is only to be kept while something is
// still coming.
TEST(TerrainDrape, AZoomOutEndsOnTheTilesOfTheLevelItEndsOn) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    const std::string dem = demTile();
    fileSource->tileResponse = [&](const Resource& resource) {
        Response res;
        if (resource.url.find("/dem/") != std::string::npos) {
            res.data = std::make_shared<std::string>(dem);
        } else {
            res.data = std::make_shared<std::string>(levelTile(tileZoomOf(resource.url)));
        }
        return res;
    };
    bool idle = false;
    StubMapObserver observer;
    observer.didBecomeIdleCallback = [&] {
        idle = true;
    };
    HeadlessFrontend frontend{{viewSize, viewSize}, 1};
    MapAdapter map(
        frontend, observer, fileSource, MapOptions().withMapMode(MapMode::Continuous).withSize(frontend.getSize()));
    map.setTerrainLoadMode(TerrainLoadMode::Quality);
    map.getStyle().loadJSON(levelStyle);

    const auto pump = [&](std::chrono::milliseconds duration) {
        const auto end = std::chrono::steady_clock::now() + duration;
        while (std::chrono::steady_clock::now() < end) {
            loop.runOnce();
            std::this_thread::sleep_for(std::chrono::milliseconds(8));
        }
    };
    const auto read = [&] {
        gfx::BackendScope guard{*frontend.getBackend()};
        return frontend.readStillImage();
    };
    // True once the map has said it is idle and the frame has stayed the same for a second; false if
    // thirty seconds pass first.
    const auto settle = [&] {
        idle = false;
        const auto start = std::chrono::steady_clock::now();
        pump(std::chrono::milliseconds(500)); // a frame is drawn before the first read
        PremultipliedImage last = read();
        int unchanged = 0;
        while (std::chrono::steady_clock::now() - start < std::chrono::seconds(30)) {
            if (unchanged >= 4 && idle) return true;
            pump(std::chrono::milliseconds(250));
            PremultipliedImage now = read();
            unchanged = std::memcmp(now.data.get(), last.data.get(), now.bytes()) == 0 ? unchanged + 1 : 0;
            last = std::move(now);
        }
        return false;
    };

    const auto view = [](double zoom) {
        return CameraOptions().withCenter(LatLng{46.0, 10.0}).withZoom(zoom).withPitch(50.0);
    };

    // At 15.1 the tiles are of zoom 15: red. At 14.9 they are of zoom 14: blue.
    map.jumpTo(view(15.1));
    ASSERT_TRUE(settle());
    const PremultipliedImage closeUp = read();
    ASSERT_GT(mostly(closeUp, 0), compareArea * 9 / 10) << "the view at 15.1 draws the tiles of zoom 15";

    map.jumpTo(view(14.9));
    ASSERT_TRUE(settle());
    const PremultipliedImage out = read();
    EXPECT_GT(mostly(out, 2), compareArea * 9 / 10) << "after zooming out, the tiles of zoom 14";
    EXPECT_LT(mostly(out, 0), compareArea / 10) << "the tiles of zoom 15 left over";
}
