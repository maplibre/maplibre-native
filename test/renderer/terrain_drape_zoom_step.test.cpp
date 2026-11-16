#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/stub_map_observer.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/bound_options.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/map/mode.hpp>
#include <mln/style/style.hpp>
#include <mln/style/terrain.hpp>
#include <mln/util/image.hpp>
#include <mln/util/run_loop.hpp>

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <thread>
#include <utility>

using namespace mln;

namespace {

// A DEM that stops at z13 under a raster map. The ground height and the picture depend only on the
// position in the world, never on the tile that carries them, so every correct drawing of a view
// comes out the same.
const char* stepStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/dem/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 13,
      "tileSize": 256
    },
    "ground": {
      "type": "raster",
      "tiles": ["http://example.com/ground/{z}-{x}-{y}.png"],
      "maxzoom": 17,
      "tileSize": 256
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": [{"id": "ground", "type": "raster", "source": "ground", "paint": {"raster-fade-duration": 0}}]
})STYLE";

constexpr uint32_t tileSize = 256;

// The position, in the unit square of the world, of pixel (pixelX, pixelY) of a tile.
std::pair<double, double> worldPosition(const Resource::TileData& tile, double pixelX, double pixelY) {
    const double scale = std::exp2(-tile.z);
    return {(tile.x + pixelX / tileSize) * scale, (tile.y + pixelY / tileSize) * scale};
}

// Hills a couple of kilometers across and a couple of hundred meters tall, the same over every tile.
double heightAt(double gx, double gy) {
    const double wave = std::exp2(14.0) * 2.0 * M_PI;
    return 600.0 + 250.0 * std::sin(gx * wave) * std::cos(gy * wave * 0.8) + 120.0 * std::sin((gx + gy) * wave * 1.7);
}

// Terrain-RGB encodes elevation as -10000 + (R * 65536 + G * 256 + B) * 0.1.
std::string demTile(const Resource::TileData& tile) {
    PremultipliedImage image({tileSize, tileSize});
    for (uint32_t y = 0; y < tileSize; ++y) {
        for (uint32_t x = 0; x < tileSize; ++x) {
            const auto [gx, gy] = worldPosition(tile, x + 0.5, y + 0.5);
            const auto value = static_cast<uint32_t>((heightAt(gx, gy) + 10000.0) * 10.0);
            uint8_t* px = image.data.get() + (y * tileSize + x) * 4;
            px[0] = static_cast<uint8_t>((value >> 16) & 0xff);
            px[1] = static_cast<uint8_t>((value >> 8) & 0xff);
            px[2] = static_cast<uint8_t>(value & 0xff);
            px[3] = 255;
        }
    }
    return encodePNG(image);
}

// A checkerboard with a gradient inside each cell. A cell is 2^-16 of the world divided by 1.37, so no
// tile holds a whole number of them and no two tiles carry the same picture: a tile drawn at the
// wrong place, or another tile's picture drawn on it, comes out in the wrong colors.
std::string groundTile(const Resource::TileData& tile) {
    PremultipliedImage image({tileSize, tileSize});
    for (uint32_t y = 0; y < tileSize; ++y) {
        for (uint32_t x = 0; x < tileSize; ++x) {
            const auto [gx, gy] = worldPosition(tile, x + 0.5, y + 0.5);
            const double cx = gx * std::exp2(16.0) * 1.37;
            const double cy = gy * std::exp2(16.0) * 1.37;
            const bool odd = ((static_cast<int64_t>(std::floor(cx)) + static_cast<int64_t>(std::floor(cy))) & 1) != 0;
            uint8_t* px = image.data.get() + (y * tileSize + x) * 4;
            px[0] = odd ? 220 : 40;
            px[1] = static_cast<uint8_t>((cx - std::floor(cx)) * 255.0);
            px[2] = static_cast<uint8_t>((cy - std::floor(cy)) * 255.0);
            px[3] = 255;
        }
    }
    return encodePNG(image);
}

CameraOptions viewAt(double zoom) {
    return CameraOptions().withCenter(LatLng{38.266, -120.686}).withZoom(zoom).withPitch(50.0);
}

// The share of pixels, in thousandths, that differ in any channel by more than a small cutoff. The cutoff
// applies to each pixel; the limit at the end of the test is on the share. Noise at the edges of tiles and
// hills stays under that limit, and a tile drawn on the wrong ground does not.
int differingPerMille(const PremultipliedImage& a, const PremultipliedImage& b) {
    constexpr int cutoff = 24;
    EXPECT_EQ(a.size.width, b.size.width);
    EXPECT_EQ(a.size.height, b.size.height);
    size_t count = 0;
    const size_t pixels = static_cast<size_t>(a.size.width) * a.size.height;
    if (pixels == 0) {
        ADD_FAILURE() << "Empty image";
        return 1000;
    }
    for (size_t i = 0; i < pixels; ++i) {
        const uint8_t* pa = a.data.get() + i * 4;
        const uint8_t* pb = b.data.get() + i * 4;
        if (std::abs(int(pa[0]) - int(pb[0])) > cutoff || std::abs(int(pa[1]) - int(pb[1])) > cutoff ||
            std::abs(int(pa[2]) - int(pb[2])) > cutoff) {
            ++count;
        }
    }
    return static_cast<int>(count * 1000 / pixels);
}

} // namespace

// The view at zoom 15.1 over a z13 DEM, reached by easing from 14.5, is the view the same map draws
// once all of its terrain drawables are made anew. Crossing level 15 changes which drape targets are
// full size, and each tile whose target the pool replaces gets new terrain drawables. One remade
// without its depth drawable made every later tile draw its picture on another tile's ground (see
// TerrainLayerTweaker). OpenGL keeps a uniform buffer per drawable, so the test cannot fail there.
TEST(TerrainDrape, CrossingALevelMatchesFreshTerrainDrawables) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    fileSource->tileResponse = [](const Resource& resource) {
        const bool dem = resource.url.find("/dem/") != std::string::npos;
        Response res;
        res.data = std::make_shared<std::string>(dem ? demTile(*resource.tileData) : groundTile(*resource.tileData));
        return res;
    };
    bool idle = false;
    StubMapObserver observer;
    observer.didBecomeIdleCallback = [&] {
        idle = true;
    };
    // A phone-sized view, in the load mode the phone app uses.
    HeadlessFrontend frontend{{402, 874}, 1};
    MapAdapter map(
        frontend, observer, fileSource, MapOptions().withMapMode(MapMode::Continuous).withSize(frontend.getSize()));
    map.setTerrainLoadMode(TerrainLoadMode::Balanced);
    map.setBounds(BoundOptions().withMaxPitch(85.0));
    map.getStyle().loadJSON(stepStyle);

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
    // True once the frame has stayed the same for a second and a half and the map has said it is idle;
    // false if a minute passes first. A frame that merely stopped changing, with tiles still on the
    // way, does not count.
    // The idle flag is cleared before each change, not here: a change can finish within the pump
    // after it, and the map does not say it is idle again until something else changes.
    const auto settle = [&] {
        const auto start = std::chrono::steady_clock::now();
        pump(std::chrono::milliseconds(500)); // a frame is drawn before the first read (Vulkan asserts on none)
        PremultipliedImage last = read();
        int unchanged = 0;
        while (std::chrono::steady_clock::now() - start < std::chrono::seconds(60)) {
            if (unchanged >= 6 && idle) return true;
            pump(std::chrono::milliseconds(250));
            PremultipliedImage now = read();
            const bool same = std::memcmp(now.data.get(), last.data.get(), now.bytes()) == 0;
            unchanged = same ? unchanged + 1 : 0;
            last = std::move(now);
        }
        return false;
    };

    idle = false;
    map.jumpTo(viewAt(14.5));
    ASSERT_TRUE(settle());
    idle = false;
    map.easeTo(viewAt(15.1), AnimationOptions(Milliseconds(1000)));
    pump(std::chrono::milliseconds(1500));
    ASSERT_TRUE(settle());
    const PremultipliedImage stepped = read();

    // The picture is drawn: both colors of the checkerboard are in the middle of the view. Two blank
    // frames would match each other and the comparison below would prove nothing.
    size_t light = 0;
    size_t dark = 0;
    for (uint32_t y = 337; y < 537; ++y) {
        for (uint32_t x = 101; x < 301; ++x) {
            const uint8_t red = stepped.data.get()[(y * stepped.size.width + x) * 4];
            light += red > 180 ? 1 : 0;
            dark += red < 80 ? 1 : 0;
        }
    }
    ASSERT_GT(light, 2000u) << "the ground tiles did not draw";
    ASSERT_GT(dark, 2000u) << "the ground tiles did not draw";

    // Taking the terrain off and setting it again makes the renderer build a new RenderTerrain: new
    // terrain drawables and new drape targets. (A new exaggeration alone no longer does.)
    idle = false;
    map.getStyle().setTerrain(nullptr);
    ASSERT_TRUE(settle());
    idle = false;
    map.getStyle().setTerrain(std::make_unique<style::Terrain>("dem", 1.0f));
    ASSERT_TRUE(settle());
    const PremultipliedImage rebuilt = read();

    // The limit: fewer than 20 in 1000 pixels (2 percent) may differ. It is a share of the image, not
    // the per-pixel cutoff in differingPerMille.
    EXPECT_LT(differingPerMille(stepped, rebuilt), 20) << "per mille of pixels differ";
}
