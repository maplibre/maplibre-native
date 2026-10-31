#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/bound_options.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/map/mode.hpp>
#include <mln/renderer/render_terrain.hpp>
#include <mln/style/layer.hpp>
#include <mln/style/style.hpp>
#include <mln/style/types.hpp>
#include <mln/util/image.hpp>
#include <mln/util/run_loop.hpp>
#include <mln/util/tile_cover.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <numbers>
#include <set>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using namespace mln;

namespace {

std::string makeFlatDEMTile() {
    constexpr uint32_t size = 64;
    PremultipliedImage image({size, size});
    for (uint32_t i = 0; i < size * size; ++i) {
        uint8_t* px = image.data.get() + i * 4;
        px[0] = 1;
        px[1] = 173;
        px[2] = 176;
        px[3] = 255;
    }
    return encodePNG(image);
}

// The flat DEM tile above decodes to this height everywhere.
constexpr double flatDEMMeters = 1000.0;

// A vector source whose minzoom is deeper than the far field needs, beside the DEM. The
// vector layer starts hidden, so the DEM can load before the source asks for anything.
std::string deepSourceStyle(int demMinZoom, int demMaxZoom) {
    return R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/dem/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "minzoom": )STYLE" +
           std::to_string(demMinZoom) + R"STYLE(,
      "maxzoom": )STYLE" +
           std::to_string(demMaxZoom) + R"STYLE(,
      "tileSize": 64
    },
    "parcels": {
      "type": "vector",
      "tiles": ["http://example.com/parcels/{z}-{x}-{y}.pbf"],
      "minzoom": 13,
      "maxzoom": 14
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": [{
    "id": "parcels",
    "type": "fill",
    "source": "parcels",
    "source-layer": "parcels",
    "layout": {"visibility": "none"}
  }]
})STYLE";
}

CanonicalTileID tileIDFromURL(const std::string& url) {
    const size_t slash = url.rfind('/');
    unsigned z = 0, x = 0, y = 0;
    std::sscanf(url.c_str() + slash + 1, "%u-%u-%u", &z, &x, &y);
    return CanonicalTileID(static_cast<uint8_t>(z), x, y);
}

// The terrain's height everywhere, for the tile cover the renderer draws from the DEM it loaded.
class FlatTerrain : public util::TileElevationProvider {
public:
    std::optional<Range<double>> getTileElevationRange(const CanonicalTileID&) const override {
        return Range<double>{flatDEMMeters, flatDEMMeters};
    }
};

// A steep view over terrain, with a vector source and the DEM both behind a stub file source that
// records what each one is asked for.
class TerrainSourceCoverTest {
public:
    struct Options {
        int demMinZoom = 0;
        int demMaxZoom = 6;
        TerrainLoadMode mode = TerrainLoadMode::Quality;
        double pitch = 70.0;
        double zoom = 13.0;
        Size size{1200, 800};
    };

    explicit TerrainSourceCoverTest(const Options& options)
        : zoom(options.zoom),
          fileSource(std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions())),
          demTile(makeFlatDEMTile()),
          frontend(options.size, 1),
          map(frontend,
              MapObserver::nullObserver(),
              fileSource,
              MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize())) {
        fileSource->tileResponse = [this](const Resource& resource) {
            Response res;
            if (resource.url.find("/parcels/") != std::string::npos) {
                parcelTiles.insert(tileIDFromURL(resource.url));
                res.noContent = true;
            } else {
                demTiles.insert(tileIDFromURL(resource.url));
                res.data = std::make_shared<std::string>(demTile);
            }
            return res;
        };
        map.setTerrainLoadMode(options.mode);
        map.getStyle().loadJSON(deepSourceStyle(options.demMinZoom, options.demMaxZoom));
        map.setBounds(BoundOptions().withMaxPitch(85.0));
        map.jumpTo(CameraOptions().withCenter(LatLng{38.26, -120.68}).withZoom(options.zoom).withPitch(options.pitch));
    }

    void render(int frames) {
        for (int i = 0; i < frames; ++i) {
            loop.runOnce();
            frontend.render(map);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }

    // Renders until the DEM has stopped arriving: the elevated cover is then the same from one
    // frame to the next.
    void settleDEM() {
        size_t seen = 0;
        int quiet = 0;
        for (int frames = 0; frames < 200 && quiet < 10; ++frames) {
            render(1);
            quiet = demTiles.size() == seen ? quiet + 1 : 0;
            seen = demTiles.size();
        }
        ASSERT_GE(quiet, 10) << "the DEM was still loading after 200 frames";
    }

    void showParcels() { map.getStyle().getLayer("parcels")->setVisibility(style::VisibilityType::Visible); }

    size_t sourceTileLimit() const {
        return terrainLoadBudget(map.getTerrainLoadMode()).maxMeshTiles * RenderTerrain::sourceTilesPerMeshTile;
    }

    // Where the camera stands on the ground, from the free-camera API, worked out independently of
    // RenderTerrain.
    std::array<double, 2> cameraGround() const {
        const vec3 position = *map.getFreeCameraOptions().position;
        return {{position[0], position[1]}};
    }

    // The `count` tiles of the parcels source's whole cover nearest `origin` (normalized
    // web-mercator), before any cap: tileCover over the flat plateau the DEM tiles describe, or
    // over flat ground without terrain. `coverSize` gets the size of the whole cover.
    std::set<CanonicalTileID> nearestCover(const std::array<double, 2>& origin,
                                           size_t count,
                                           size_t& coverSize,
                                           bool overTerrain = true) const {
        const auto coverZoom = static_cast<uint8_t>(std::clamp(static_cast<int>(zoom), 13, 14));
        FlatTerrain terrain;
        const util::TileCoverParameters params{.transformState = map.getTransformState(),
                                               .elevationProvider = overTerrain ? &terrain : nullptr};
        std::vector<std::pair<double, CanonicalTileID>> ranked;
        for (const OverscaledTileID& id : util::tileCover(params, coverZoom, Range<uint8_t>{13, 14}, coverZoom)) {
            const double scale = static_cast<double>(1u << id.canonical.z);
            const double dx = (id.canonical.x + 0.5) / scale + id.wrap - origin[0];
            const double dy = (id.canonical.y + 0.5) / scale - origin[1];
            ranked.emplace_back(dx * dx + dy * dy, id.canonical);
        }
        std::sort(ranked.begin(), ranked.end());
        coverSize = ranked.size();
        std::set<CanonicalTileID> nearest;
        for (size_t i = 0; i < std::min(count, ranked.size()); ++i) nearest.insert(ranked[i].second);
        return nearest;
    }

    // The map centre in normalized web-mercator.
    std::array<double, 2> mapCentre() const {
        const LatLng centre = *map.getCameraOptions({}).center;
        const double latRad = centre.latitude() * std::numbers::pi / 180.0;
        return {{centre.longitude() / 360.0 + 0.5,
                 0.5 - std::log(std::tan(std::numbers::pi / 4.0 + latRad / 2.0)) / (2.0 * std::numbers::pi)}};
    }

    double zoom;
    util::RunLoop loop;
    std::shared_ptr<StubFileSource> fileSource;
    std::string demTile;
    HeadlessFrontend frontend;
    MapAdapter map;
    std::set<CanonicalTileID> parcelTiles;
    std::set<CanonicalTileID> demTiles;
};

// The source asks for the tiles nearest the camera and no others: the same set as the nearest
// of the whole cover, measured from where the camera stands on the ground.
void expectOnlyTheTilesNearestTheCamera(const TerrainSourceCoverTest::Options& options) {
    TerrainSourceCoverTest test(options);
    // The DEM arrives first: the elevated cover shifts as it lands, and a source that asked for
    // the tiles on each side of that shift has asked for more than its cap in all.
    test.settleDEM();
    test.showParcels();
    test.render(10);

    const size_t limit = test.sourceTileLimit();
    size_t coverSize = 0;
    const std::set<CanonicalTileID> nearestTheCamera = test.nearestCover(test.cameraGround(), limit, coverSize);
    ASSERT_GT(coverSize, limit) << "the view has to cover more than the cap";

    // A cap measured from the map centre would keep a few other tiles here.
    size_t unused = 0;
    ASSERT_NE(nearestTheCamera, test.nearestCover(test.mapCentre(), limit, unused));

    EXPECT_EQ(test.parcelTiles.size(), limit);
    EXPECT_EQ(test.parcelTiles, nearestTheCamera);
}

} // namespace

// A source whose minzoom is deeper than the far field is covered at that minzoom out to the
// horizon, thousands of tiles at a steep pitch. Under terrain it asks for the nearest few per
// tile of the mesh-tile cap, nearest the camera, not the map centre.
TEST(TerrainSourceCover, ASourceRequestsOnlyTheTilesNearestTheCamera) {
    expectOnlyTheTilesNearestTheCamera({.mode = TerrainLoadMode::Quality, .size = {1600, 1600}});
}

TEST(TerrainSourceCover, TheCapFollowsTheTerrainLoadMode) {
    expectOnlyTheTilesNearestTheCamera({.mode = TerrainLoadMode::Performance});
}

// The terrain draws from its DEM source, so the DEM keeps its own cover rather than the cap.
// Here the DEM is as deep as the vector source, so its cover holds far more tiles than the cap
// allows. A capped DEM would stay within a few tiles of the limit: the tiles it asks for before
// it has loaded and after differ by a handful.
TEST(TerrainSourceCover, TheDEMSourceKeepsItsOwnCover) {
    TerrainSourceCoverTest test({.demMinZoom = 13, .demMaxZoom = 13, .mode = TerrainLoadMode::Performance});
    test.settleDEM();

    EXPECT_GT(test.demTiles.size(), 2 * test.sourceTileLimit());
}

// Without terrain there is no mesh to size the cap from, and the source keeps its whole cover.
TEST(TerrainSourceCover, ASourceKeepsItsWholeCoverWithoutTerrain) {
    TerrainSourceCoverTest test({.mode = TerrainLoadMode::Performance});
    test.settleDEM();
    test.map.getStyle().setTerrain(nullptr);
    test.showParcels();
    test.render(10);

    size_t coverSize = 0;
    test.nearestCover(test.cameraGround(), 0, coverSize, false);
    ASSERT_GT(coverSize, test.sourceTileLimit()) << "the view has to cover more than the cap would allow";
    EXPECT_EQ(test.parcelTiles.size(), coverSize);
}
