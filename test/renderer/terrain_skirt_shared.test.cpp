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
#include <mln/renderer/render_terrain.hpp>
#include <mln/style/layer.hpp>
#include <mln/style/style.hpp>
#include <mln/util/image.hpp>
#include <mln/util/run_loop.hpp>

#include <chrono>
#include <cmath>
#include <cstring>
#include <memory>
#include <mutex>
#include <numbers>
#include <optional>
#include <set>
#include <string>
#include <thread>
#include <vector>

using namespace mln;

namespace {

constexpr double lat0 = 38.52313;
constexpr double lon0 = -120.08473;
constexpr double earthRadius = 6378137.0;

// A magenta background, so any pixel of it inside the terrain is a crack, over a gray checkerboard ground.
const char* sceneStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {"type": "raster-dem", "tiles": ["https://t.test/dem/{z}/{x}/{y}.png"],
            "encoding": "terrarium", "tileSize": 256, "maxzoom": 17},
    "ground": {"type": "raster", "tiles": ["https://t.test/ground/{z}/{x}/{y}.png"], "tileSize": 256}
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": [
    {"id": "bg", "type": "background", "paint": {"background-color": "#ff00ff"}},
    {"id": "ground", "type": "raster", "source": "ground", "paint": {"raster-fade-duration": 0}}
  ]
})STYLE";

// One height function for every zoom, with features down to a few meters, so a tile at one zoom and its
// neighbor at another sample it at different resolutions and their heights part at the shared edge.
double heightAt(double east, double south) {
    const double tau = 2.0 * std::numbers::pi;
    return 1500.0 - 0.12 * south + 6.0 * std::sin(tau * east / 60.0 + 0.3) * std::cos(tau * south / 53.0 + 1.1) +
           2.5 * std::sin(tau * (east + 0.6 * south) / 23.0) +
           0.9 * std::sin(tau * east / 9.1 + 2.0) * std::sin(tau * south / 8.3) +
           0.35 * std::sin(tau * (east - south) / 4.1);
}

// Meters east and south of the scene's center of the point at (u, v) in the world's unit square.
std::pair<double, double> metersOf(double u, double v) {
    const double lon = u * 360.0 - 180.0;
    const double lat = std::atan(std::sinh(std::numbers::pi * (1.0 - 2.0 * v))) * 180.0 / std::numbers::pi;
    return {(lon - lon0) * std::numbers::pi / 180.0 * earthRadius * std::cos(lat0 * std::numbers::pi / 180.0),
            (lat0 - lat) * std::numbers::pi / 180.0 * earthRadius};
}

std::string demTile(const CanonicalTileID& id) {
    const double n = std::exp2(id.z);
    PremultipliedImage image({256, 256});
    for (uint32_t y = 0; y < 256; ++y) {
        for (uint32_t x = 0; x < 256; ++x) {
            const auto [east, south] = metersOf((id.x + (x + 0.5) / 256.0) / n, (id.y + (y + 0.5) / 256.0) / n);
            const double stored = heightAt(east, south) + 32768.0;
            uint8_t* px = image.data.get() + (y * 256 + x) * 4;
            px[0] = static_cast<uint8_t>(stored / 256.0);
            px[1] = static_cast<uint8_t>(std::fmod(std::floor(stored), 256.0));
            px[2] = static_cast<uint8_t>((stored - std::floor(stored)) * 256.0);
            px[3] = 255;
        }
    }
    return encodePNG(image);
}

std::string groundTile() {
    PremultipliedImage image({256, 256});
    for (uint32_t y = 0; y < 256; ++y) {
        for (uint32_t x = 0; x < 256; ++x) {
            uint8_t* px = image.data.get() + (y * 256 + x) * 4;
            px[0] = px[1] = px[2] = (((x / 32) + (y / 32)) & 1) ? 230 : 25;
            px[3] = 255;
        }
    }
    return encodePNG(image);
}

bool isBackground(const uint8_t* px) {
    return px[0] > 250 && px[1] < 5 && px[2] > 250;
}

// Background pixels with ground above them in the same column: cracks in the terrain, not sky.
size_t crackPixels(const PremultipliedImage& image) {
    const size_t width = image.size.width;
    const size_t height = image.size.height;
    size_t count = 0;
    for (size_t x = 0; x < width; ++x) {
        bool ground = false;
        for (size_t y = 0; y < height; ++y) {
            const bool background = isBackground(image.data.get() + (y * width + x) * 4);
            ground = ground || !background;
            count += ground && background ? 1 : 0;
        }
    }
    return count;
}

// Pixels that differ in any channel by more than a small cutoff.
size_t differingPixels(const PremultipliedImage& a, const PremultipliedImage& b) {
    EXPECT_EQ(a.size, b.size);
    size_t count = 0;
    for (size_t i = 0; i < a.bytes(); i += 4) {
        for (size_t c = 0; c < 3; ++c) {
            if (std::abs(int(a.data.get()[i + c]) - int(b.data.get()[i + c])) > 8) {
                ++count;
                break;
            }
        }
    }
    return count;
}

// Drops the mesh tiles that cover any of one zoom 18 row, so the terrain has a gap across the view with a
// frontier on each side of it.
class MeshRowGap {
public:
    explicit MeshRowGap(uint32_t row) {
        RenderTerrain::setMeshCoverFilterForTesting([row](const UnwrappedTileID& id) {
            const uint32_t z = id.canonical.z;
            const uint32_t y = id.canonical.y;
            const bool covers = z <= 18 ? (row >> (18 - z)) == y : (y >> (z - 18)) == row;
            return !covers;
        });
    }
    ~MeshRowGap() { RenderTerrain::setMeshCoverFilterForTesting({}); }
};

// The tile of the given zoom a little north of the scene's center, in the middle of the screen.
CanonicalTileID middleTile(uint8_t z) {
    const double lat = lat0 + 120.0 / earthRadius * 180.0 / std::numbers::pi;
    const double sinLat = std::sin(lat * std::numbers::pi / 180.0);
    const double n = std::exp2(z);
    return {
        z,
        static_cast<uint32_t>(std::floor((lon0 + 180.0) / 360.0 * n)),
        static_cast<uint32_t>(std::floor((0.5 - std::log((1 + sinLat) / (1 - sinLat)) / (4 * std::numbers::pi)) * n))};
}

struct Render {
    PremultipliedImage image;
    std::set<uint8_t> demZooms;
};

// One frame of the scene seen from the south at the given pitch, with the given skirts. A map shows the
// view once the pictures stop changing; one that waits on a DEM tile never says it is idle.
Render renderScene(TerrainSkirtLength skirts,
                   double tileLodPitchThreshold,
                   double pitch,
                   const std::optional<CanonicalTileID>& withheldDemTile = std::nullopt) {
    util::RunLoop loop;
    std::mutex mutex;
    Render out;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    fileSource->tileResponse = [&](const Resource& resource) -> std::optional<Response> {
        const auto& data = *resource.tileData;
        const CanonicalTileID tile{
            static_cast<uint8_t>(data.z), static_cast<uint32_t>(data.x), static_cast<uint32_t>(data.y)};
        const bool dem = resource.url.find("/dem/") != std::string::npos;
        if (dem) {
            const std::lock_guard<std::mutex> lock(mutex);
            out.demZooms.insert(tile.z);
            if (tile == withheldDemTile) {
                return std::nullopt;
            }
        }
        Response res;
        res.data = std::make_shared<std::string>(dem ? demTile(tile) : groundTile());
        return res;
    };
    bool idle = false;
    StubMapObserver observer;
    observer.didBecomeIdleCallback = [&] {
        idle = true;
    };
    HeadlessFrontend frontend{{402, 874}, 1};
    MapAdapter map(
        frontend, observer, fileSource, MapOptions().withMapMode(MapMode::Continuous).withSize(frontend.getSize()));
    map.setTerrainLoadMode(TerrainLoadMode::Balanced);
    map.setBounds(BoundOptions().withMaxPitch(85.0));
    map.setTileLodPitchThreshold(tileLodPitchThreshold);
    map.setTerrainSkirtLength(skirts);
    map.getStyle().loadJSON(sceneStyle);
    map.setCenterClampedToGround(true);

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
    // Once the frame has stayed the same for a second, and the map has said it is idle unless a tile is
    // withheld; a minute is as long as it waits.
    const auto settle = [&] {
        idle = false;
        const auto start = std::chrono::steady_clock::now();
        pump(std::chrono::milliseconds(500));
        PremultipliedImage last = read();
        int unchanged = 0;
        while (std::chrono::steady_clock::now() - start < std::chrono::seconds(60)) {
            if (unchanged >= 4 && (idle || withheldDemTile)) {
                return last;
            }
            pump(std::chrono::milliseconds(250));
            PremultipliedImage now = read();
            unchanged = std::memcmp(now.data.get(), last.data.get(), now.bytes()) == 0 ? unchanged + 1 : 0;
            last = std::move(now);
        }
        ADD_FAILURE() << "the map did not settle";
        return last;
    };

    map.jumpTo(CameraOptions().withCenter(LatLng{lat0, lon0}).withZoom(17.0).withPitch(pitch).withBearing(0.0));
    out.image = settle();
    return out;
}

// The mesh covers the view with tiles of more than one zoom (finer near the camera) once the pitch passes
// tileLodPitchThreshold: a threshold of 0 does so at once, one past any pitch never does.
constexpr double uniformZoom = std::numbers::pi;
constexpr double mixedZooms = 0.0;

} // namespace

// A DEM tile that has not arrived: its mesh tiles stand on the ancestor's heights while their neighbors
// stand on their own, so the edges between them part and the background shows through. Skirts hang
// below every edge to cover that; the shared mode does so wherever a tile has a neighbor, which is on
// the other side of every edge the seam opens at.
TEST(TerrainSkirtShared, ClosesTheSeamWhileAnElevationTileLoads) {
    const auto withheld = middleTile(17);
    const size_t none = crackPixels(renderScene(TerrainSkirtLength::None, uniformZoom, 60.0, withheld).image);
    const size_t shared = crackPixels(renderScene(TerrainSkirtLength::Shared, uniformZoom, 60.0, withheld).image);
    const size_t automatic = crackPixels(renderScene(TerrainSkirtLength::Auto, uniformZoom, 60.0, withheld).image);
    EXPECT_GT(none, 500u) << "the scene shows no crack without skirts";
    EXPECT_LE(shared, 2u) << "crack pixels with shared skirts";
    EXPECT_LE(automatic, 2u) << "crack pixels with auto skirts";
}

// Neighbors at different zooms sample the DEM at different resolutions, so their heights part at the
// edge between them.
TEST(TerrainSkirtShared, ClosesTheSeamBetweenZoomLevels) {
    const auto none = renderScene(TerrainSkirtLength::None, mixedZooms, 60.0);
    ASSERT_GT(none.demZooms.size(), 1u) << "the view has tiles of one zoom only";
    const size_t shared = crackPixels(renderScene(TerrainSkirtLength::Shared, mixedZooms, 60.0).image);
    EXPECT_GT(crackPixels(none.image), 100u) << "the scene shows no crack without skirts";
    EXPECT_LE(shared, 2u) << "crack pixels with shared skirts";
}

// At the frontier, where a tile has no neighbor, skirts hang into empty space and show as streaks. A row
// of tiles across the view is left out of the mesh, and the tiles beyond it show their near edge across
// the gap. The auto mode hangs a streak below that edge, across the whole width of the view; the shared
// mode hangs none. What it does draw there is the end of each skirt between two tiles, seen through the gap
// as a thin sliver, a small share of the streak.
TEST(TerrainSkirtShared, LeavesTheFrontierBare) {
    const MeshRowGap gap(middleTile(18).y);
    const auto none = renderScene(TerrainSkirtLength::None, uniformZoom, 60.0);
    const auto shared = renderScene(TerrainSkirtLength::Shared, uniformZoom, 60.0);
    const auto automatic = renderScene(TerrainSkirtLength::Auto, uniformZoom, 60.0);
    const size_t streak = differingPixels(none.image, automatic.image);
    const size_t slivers = differingPixels(none.image, shared.image);
    EXPECT_GT(streak, 10000u) << "the auto skirts show no streak";
    EXPECT_LT(slivers * 8, streak) << "pixels the shared skirts change, against the auto streak";
}
