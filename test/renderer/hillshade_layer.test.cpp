#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/style/style.hpp>
#include <mln/util/image.hpp>
#include <mln/util/run_loop.hpp>

using namespace mln;

namespace {

std::string makeDEMTile() {
    PremultipliedImage image({64, 64});
    for (uint32_t i = 0; i < 64 * 64; ++i) {
        uint8_t* px = image.data.get() + i * 4;
        px[0] = 1;
        px[1] = 173;
        px[2] = 176;
        px[3] = 255;
    }
    return encodePNG(image);
}

std::size_t buffersCreatedByAStillFrame(const char* layerType) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    const std::string tile = makeDEMTile();
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
    map.getStyle().loadJSON(std::string(R"STYLE({
      "version": 8,
      "sources": {
        "dem": {"type": "raster-dem", "tiles": ["http://example.com/{z}-{x}-{y}.png"], "tileSize": 256, "maxzoom": 9}
      },
      "layers": [{"id": "relief", "source": "dem", "type": ")STYLE") +
                            layerType + R"STYLE("}]})STYLE");
    map.jumpTo(CameraOptions().withCenter(LatLng{47.2692, 11.4041}).withZoom(5.0));

    (void)frontend.render(map);
    const auto settled = frontend.render(map).stats.totalBuffers;
    return frontend.render(map).stats.totalBuffers - settled;
}

} // namespace

// Tiles without a mask share one quad. A frame that changes nothing must not upload it again.
TEST(HillshadeLayer, AStillFrameCreatesNoBuffers) {
    EXPECT_EQ(0u, buffersCreatedByAStillFrame("hillshade"));
}

TEST(ColorReliefLayer, AStillFrameCreatesNoBuffers) {
    EXPECT_EQ(0u, buffersCreatedByAStillFrame("color-relief"));
}
