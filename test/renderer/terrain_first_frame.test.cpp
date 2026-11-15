#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/context.hpp>
#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/style/style.hpp>
#include <mln/style/terrain.hpp>
#include <mln/util/image.hpp>
#include <mln/util/run_loop.hpp>

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <vector>

using namespace mln;

namespace {

// A red polygon that covers the view, over a white background. The DEM source answers nothing, so
// no elevation tile ever loads.
const char* areaStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/dem/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 8,
      "tileSize": 256
    },
    "area": {
      "type": "raster",
      "tiles": ["http://example.com/area/{z}-{x}-{y}.png"],
      "tileSize": 256
    }
  },
  "layers": [
    {"id": "background", "type": "background", "paint": {"background-color": "#ffffff"}},
    {"id": "area", "type": "raster", "source": "area", "paint": {"raster-fade-duration": 0}}
  ]
})STYLE";

std::string redTile() {
    PremultipliedImage image({256, 256});
    for (uint32_t i = 0; i < 256 * 256; ++i) {
        uint8_t* px = image.data.get() + i * 4;
        px[0] = 255;
        px[1] = 0;
        px[2] = 0;
        px[3] = 255;
    }
    return encodePNG(image);
}

struct FirstFrameTest {
    std::string red = redTile();
    util::RunLoop loop;
    std::shared_ptr<StubFileSource> fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(),
                                                                                  ClientOptions());
    // Only the frames the test asks for render.
    HeadlessFrontend frontend{
        {128, 128}, 1, gfx::HeadlessBackend::SwapBehaviour::NoFlush, gfx::ContextMode::Unique, std::nullopt, false};
    MapAdapter map;

    FirstFrameTest()
        : map(frontend,
              MapObserver::nullObserver(),
              fileSource,
              MapOptions().withMapMode(MapMode::Continuous).withSize(frontend.getSize())) {
        // The area's tiles come at once. An elevation tile is asked for and does not come.
        fileSource->tileResponse = [this](const Resource& resource) -> std::optional<Response> {
            if (resource.url.find("/dem/") != std::string::npos) {
                return std::nullopt;
            }
            Response res;
            res.data = std::make_shared<std::string>(red);
            return res;
        };
        map.getStyle().loadJSON(areaStyle);
        map.jumpTo(CameraOptions().withCenter(LatLng{46.0, 10.0}).withZoom(5.0));
    }

    // One frame; how many draw calls it made.
    int drawCallsOfOneFrame() {
        loop.runOnce();
        frontend.renderFrame();
        gfx::BackendScope guard{*frontend.getBackend()};
        return frontend.getBackend()->getContext().renderingStats().numDrawCalls;
    }
};

} // namespace

// Terrain comes on over elevation that has not arrived. The map draws its ground, flat, from the
// first frame: it used to draw nothing until the first elevation tile was in, one empty frame between
// the flat map and the terrain.
TEST(TerrainFirstFrame, TerrainSetBeforeItsElevationLoadsDrawsGroundFromTheFirstFrame) {
    FirstFrameTest test;
    int flat = 0;
    for (int i = 0; i < 20 && flat == 0; ++i) {
        flat = test.drawCallsOfOneFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ASSERT_GT(flat, 0) << "the flat map draws";

    test.map.getStyle().setTerrain(std::make_unique<style::Terrain>("dem", 1.0f));

    EXPECT_GT(test.drawCallsOfOneFrame(), 0) << "the first frame with terrain drew nothing";
}
