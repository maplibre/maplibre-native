#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/bound_options.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/map/mode.hpp>
#include <mln/renderer/render_terrain.hpp>
#include <mln/style/style.hpp>
#include <mln/util/image.hpp>
#include <mln/util/run_loop.hpp>

#include <set>
#include <string>

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

// A vector source whose minzoom is deeper than the far field needs, beside the DEM.
const char* deepSourceStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/dem/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 6,
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
  "layers": [{"id": "parcels", "type": "fill", "source": "parcels", "source-layer": "parcels"}]
})STYLE";

} // namespace

// A source whose minzoom is deeper than the far field is covered at that minzoom out to the
// horizon, thousands of tiles at a steep pitch. Under terrain it asks for the nearest few per
// tile of the mesh-tile cap.
TEST(TerrainSourceCover, ASourceRequestsOnlyTheTilesNearestTheCamera) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    const std::string demTile = makeFlatDEMTile();
    std::set<std::string> requested;
    fileSource->tileResponse = [&](const Resource& resource) {
        Response res;
        if (resource.url.find("/parcels/") != std::string::npos) {
            requested.insert(resource.url);
            res.noContent = true;
        } else {
            res.data = std::make_shared<std::string>(demTile);
        }
        return res;
    };
    HeadlessFrontend frontend{{390, 844}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));
    map.getStyle().loadJSON(deepSourceStyle);
    map.setBounds(BoundOptions().withMaxPitch(85.0));
    map.jumpTo(CameraOptions().withCenter(LatLng{38.26, -120.68}).withZoom(13.0).withPitch(80.0));

    for (int i = 0; i < 10; ++i) {
        loop.runOnce();
        frontend.render(map);
    }

    const size_t limit = terrainLoadBudget(map.getTerrainLoadMode()).maxMeshTiles *
                         RenderTerrain::sourceTilesPerMeshTile;
    ASSERT_GT(limit, 0u);
    EXPECT_GT(requested.size(), 0u);
    EXPECT_LE(requested.size(), limit) << requested.size() << " tiles requested";
}
