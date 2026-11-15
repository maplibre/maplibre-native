#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/renderer/render_terrain.hpp>
#include <mln/style/image.hpp>
#include <mln/style/style.hpp>
#include <mln/util/image.hpp>
#include <mln/util/run_loop.hpp>

#include <cmath>
#include <string>

using namespace mln;

namespace {

// A flat 300 m plateau, with one red icon at its centre from a source that stops at zoom 4. At
// sea level the icon is under the terrain surface and the depth pass hides it, so it shows only
// while its tile still binds a DEM texture.
constexpr double plateauMetres = 300.0;
constexpr double centreLat = 46.0;
constexpr double centreLon = 10.0;
constexpr uint32_t demTileSize = 64;

std::string makePlateauTile() {
    PremultipliedImage image({demTileSize, demTileSize});
    // Terrain-RGB: elevation = -10000 + (R * 65536 + G * 256 + B) * 0.1
    const auto value = static_cast<uint32_t>(std::lround((plateauMetres + 10000.0) * 10.0));
    for (uint32_t i = 0; i < demTileSize * demTileSize; ++i) {
        uint8_t* px = image.data.get() + i * 4;
        px[0] = static_cast<uint8_t>(value >> 16);
        px[1] = static_cast<uint8_t>((value >> 8) & 0xff);
        px[2] = static_cast<uint8_t>(value & 0xff);
        px[3] = 255;
    }
    return encodePNG(image);
}

const char* peakStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 14,
      "tileSize": 512
    },
    "peaks": {
      "type": "geojson",
      "maxzoom": 4,
      "data": {"type": "Feature", "properties": {}, "geometry": {"type": "Point", "coordinates": [10.0, 46.0]}}
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": [
    {"id": "background", "type": "background", "paint": {"background-color": "white"}},
    {"id": "peaks", "type": "symbol", "source": "peaks",
     "layout": {"icon-image": "dot", "icon-allow-overlap": true, "icon-ignore-placement": true}}
  ]
})STYLE";

// Whether any pixel is the icon's red. A label lifted onto the plateau is drawn above its sea-level
// position, and nothing else in the scene is red.
bool hasRed(const PremultipliedImage& image) {
    for (size_t i = 0; i < image.size.width * image.size.height; ++i) {
        const uint8_t* px = image.data.get() + i * 4;
        if (px[0] > 200 && px[1] < 80 && px[2] < 80) {
            return true;
        }
    }
    return false;
}

// Puts the cache cap back to the default however the test ends
struct DEMCacheCap {
    explicit DEMCacheCap(size_t cap) { RenderTerrain::setMaxDEMTexturesForTesting(cap); }
    ~DEMCacheCap() { RenderTerrain::setMaxDEMTexturesForTesting(0); }
};

// Renders the plateau at zoom 4 and then in close at a pitch, with the DEM texture cache capped at
// `cap`, and returns whether the label is drawn at the end.
bool labelIsDrawnAfterZoomingIn(size_t cap) {
    DEMCacheCap capGuard(cap);
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    const std::string tile = makePlateauTile();
    fileSource->tileResponse = [&](const Resource&) {
        Response res;
        res.data = std::make_shared<std::string>(tile);
        return res;
    };
    HeadlessFrontend frontend{{512, 512}, 1};
    MapAdapter map(frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));
    map.getStyle().loadJSON(peakStyle);
    PremultipliedImage dot({12, 12});
    for (uint32_t i = 0; i < 12 * 12; ++i) {
        dot.data[i * 4 + 0] = 255;
        dot.data[i * 4 + 1] = 0;
        dot.data[i * 4 + 2] = 0;
        dot.data[i * 4 + 3] = 255;
    }
    map.getStyle().addImage(std::make_unique<style::Image>("dot", std::move(dot), 1.0f));

    // Wide view first: the DEM tile of the icon's own zoom-4 tile loads and gets its texture
    map.jumpTo(CameraOptions().withCenter(LatLng{centreLat, centreLon}).withZoom(4.0).withPitch(0.0));
    for (int i = 0; i < 4; ++i) {
        loop.runOnce();
        frontend.render(map);
    }

    // Then in close at a pitch: the source loads zoom-13 DEM tiles for the mesh and lets the zoom-4
    // tile go (it keeps ancestors eight levels up at most), while the icon's tile is still the zoom-4 one
    map.jumpTo(CameraOptions().withCenter(LatLng{centreLat, centreLon}).withZoom(13.0).withPitch(60.0));
    PremultipliedImage image;
    for (int i = 0; i < 12; ++i) {
        loop.runOnce();
        image = frontend.render(map).image;
    }

    return hasRed(image);
}

} // namespace

// An ancestor DEM texture that only symbols still sample (the DEM source has dropped its tile, and
// the mesh tiles bind their own deeper DEM) is not evictable: nothing would make it again, and the
// labels it elevates would fall to sea level, under the terrain, until the renderer was rebuilt.
TEST(TerrainDEMCache, ALabelKeepsTheAncestorDEMItSamplesWhenTheCacheEvicts) {
    // A cap of 1 puts every texture the frame's mesh does not use over budget
    EXPECT_TRUE(labelIsDrawnAfterZoomingIn(1))
        << "the label on the plateau fell to sea level and is hidden by the terrain";
}

// The same scene under the default cap, where nothing is evicted: it shows the label is drawn at
// all, so the test above fails for the eviction and for nothing else.
TEST(TerrainDEMCache, ALabelIsDrawnOnThePlateauUnderTheDefaultCap) {
    EXPECT_TRUE(labelIsDrawnAfterZoomingIn(0)) << "the label is not drawn on the plateau";
}
