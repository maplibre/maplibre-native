#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/renderer/buckets/raster_bucket.hpp>
#include <mln/style/style.hpp>
#include <mln/test/map_adapter.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/util/image.hpp>
#include <mln/util/run_loop.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <string>

using namespace mln;

namespace {

std::string rasterTile() {
    PremultipliedImage image({256, 256});
    for (uint32_t y = 0; y < 256; ++y) {
        for (uint32_t x = 0; x < 256; ++x) {
            uint8_t* px = image.data.get() + (y * 256 + x) * 4;
            px[0] = static_cast<uint8_t>(x);
            px[1] = static_cast<uint8_t>(y);
            px[2] = static_cast<uint8_t>((x + y) / 2);
            px[3] = 255;
        }
    }
    return encodePNG(image);
}

} // namespace

// Once a raster tile has its texture, the bucket must not keep the decoded pixels
// (about 256 KB per 256 px tile) next to the GPU copy, and the tile must still draw.
TEST(RasterBucket, ReleasesDecodedImageAfterTextureCreated) {
    const size_t baseline = RasterBucket::getHeldImageBytes();

    {
        util::RunLoop loop;
        auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
        const std::string tile = rasterTile();
        int tileRequests = 0;
        fileSource->tileResponse = [&](const Resource&) {
            ++tileRequests;
            Response res;
            res.data = std::make_shared<std::string>(tile);
            return res;
        };

        HeadlessFrontend frontend{{512, 512}, 1};
        MapAdapter map(frontend,
                       MapObserver::nullObserver(),
                       fileSource,
                       MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()));
        map.getStyle().loadJSON(R"STYLE({
            "version": 8,
            "sources": {"r": {"type": "raster", "tiles": ["http://example.com/{z}-{x}-{y}.png"], "tileSize": 256}},
            "layers": [{"id": "r", "type": "raster", "source": "r"}]
        })STYLE");
        const auto camera = CameraOptions().withCenter(LatLng{20.0, 10.0}).withZoom(2.0);
        map.jumpTo(camera);

        // A tile that arrives after a frame's update is drawn, and releases its image, in the next frame: render
        // until nothing is held any more, a few frames at most. An image that is never released still fails.
        const auto settle = [&] {
            for (int frames = 0; frames < 10 && baseline != RasterBucket::getHeldImageBytes(); ++frames) {
                (void)frontend.render(map);
            }
        };

        const auto first = frontend.render(map).image;
        EXPECT_GT(tileRequests, 1);
        settle();
        EXPECT_EQ(baseline, RasterBucket::getHeldImageBytes()) << "decoded raster images are still held after upload";

        // Another frame from the textures alone must look the same.
        const auto second = frontend.render(map).image;
        EXPECT_TRUE(first == second);

        // Leave the view and come back: the tiles are drawn again from their existing textures.
        map.jumpTo(CameraOptions().withCenter(LatLng{-40.0, -100.0}).withZoom(2.0));
        (void)frontend.render(map);
        map.jumpTo(camera);
        const auto third = frontend.render(map).image;
        EXPECT_TRUE(first == third);
        settle();
        EXPECT_EQ(baseline, RasterBucket::getHeldImageBytes());
    }

    // Every held image is accounted for again once the map is gone.
    EXPECT_EQ(baseline, RasterBucket::getHeldImageBytes());
}
