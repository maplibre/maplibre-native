#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/bound_options.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/style/style.hpp>
#include <mln/util/color.hpp>
#include <mln/util/image.hpp>
#include <mln/util/run_loop.hpp>

#include <memory>
#include <string>

using namespace mln;

namespace {

// Flat ground under a solid green drape: a pixel of the ground that carries any red was changed by
// the terrain pass itself.
const char* hazeStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/dem/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 11,
      "tileSize": 256
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": [{"id": "ground", "type": "background", "paint": {"background-color": "rgb(0, 200, 0)"}}]
})STYLE";

// Sea level everywhere: Terrain-RGB (0 m + 10000) / 0.1 = 100000.
std::string flatTile() {
    PremultipliedImage image({256, 256});
    for (size_t i = 0; i < image.bytes(); i += 4) {
        image.data[i] = 1;
        image.data[i + 1] = 134;
        image.data[i + 2] = 160;
        image.data[i + 3] = 255;
    }
    return encodePNG(image);
}

// One map looking across the ground; each render sets the pitch and a red haze at `range`.
class HazeView {
public:
    HazeView() {
        fileSource->tileResponse = [this](const Resource&) {
            Response res;
            res.data = std::make_shared<std::string>(dem);
            return res;
        };
        map.getStyle().loadJSON(hazeStyle);
        map.setBounds(BoundOptions().withMaxPitch(85.0));
    }

    PremultipliedImage render(float range, double pitch) {
        map.jumpTo(CameraOptions().withCenter(LatLng{46.0, 10.0}).withZoom(13.0).withPitch(pitch));
        map.setTerrainHaze(Color::red(), range);
        PremultipliedImage image;
        for (int i = 0; i < 4; ++i) {
            loop.runOnce();
            image = frontend.render(map).image;
        }
        return image;
    }

private:
    util::RunLoop loop;
    std::shared_ptr<StubFileSource> fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(),
                                                                                  ClientOptions());
    const std::string dem = flatTile();
    HeadlessFrontend frontend{{512, 512}, 1};
    MapAdapter map{frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize())};
};

const uint8_t* pixel(const PremultipliedImage& image, uint32_t x, uint32_t y) {
    return image.data.get() + (y * image.size.width + x) * 4;
}

int reddened(const PremultipliedImage& image) {
    int count = 0;
    for (uint32_t y = 0; y < image.size.height; ++y) {
        for (uint32_t x = 0; x < image.size.width; ++x) count += pixel(image, x, y)[0] > 0 ? 1 : 0;
    }
    return count;
}

// The reddest pixel up the middle of the view.
const uint8_t* reddestInTheMiddle(const PremultipliedImage& image) {
    const uint8_t* reddest = pixel(image, 256, 511);
    for (uint32_t y = 0; y < 512; ++y) {
        if (pixel(image, 256, y)[0] > reddest[0]) reddest = pixel(image, 256, y);
    }
    return reddest;
}

} // namespace

// At a pitch of 70 degrees the ground nearer the camera than the map center keeps the drape, and the
// far ground, past twice the center's distance (the range), takes the full haze color. The green
// above the ground is the background layer's own, which the haze does not touch.
TEST(TerrainHaze, FarGroundTakesTheHazeColorAndNearGroundKeepsTheDrape) {
    HazeView view;
    const PremultipliedImage image = view.render(2.0f, 70.0);

    for (uint32_t x = 0; x < 512; x += 8) {
        const uint8_t* px = pixel(image, x, 500);
        EXPECT_EQ(px[0], 0);
        EXPECT_EQ(px[1], 200);
        EXPECT_EQ(px[3], 255);
    }

    // Up the middle of the view the red grows with the distance, and the farthest ground is all red.
    uint32_t edge = 512;
    for (uint32_t y = 0; y < 512 && edge == 512; ++y) {
        if (pixel(image, 256, y)[0] > 0) edge = y;
    }
    ASSERT_LT(edge, 400u);
    EXPECT_EQ(pixel(image, 256, edge + 2)[0], 255);
    EXPECT_EQ(pixel(image, 256, edge + 2)[1], 0);
    uint8_t previous = 255;
    for (uint32_t y = edge + 2; y < 512; y += 4) {
        const uint8_t red = pixel(image, 256, y)[0];
        EXPECT_LE(red, previous) << "y " << y;
        previous = red;
    }
}

// The haze fades in with the pitch: none at 55 degrees, half the color on the farthest ground at 65,
// all of it at 70.
TEST(TerrainHaze, TheHazeFadesInBetweenPitches60And70) {
    HazeView view;
    EXPECT_EQ(reddened(view.render(1.5f, 55.0)), 0);

    const PremultipliedImage at65 = view.render(1.5f, 65.0);
    const uint8_t* half = reddestInTheMiddle(at65);
    EXPECT_NEAR(half[0], 127.5, 1.0);
    EXPECT_NEAR(half[1], 100.0, 1.0);

    const PremultipliedImage at70 = view.render(1.5f, 70.0);
    const uint8_t* full = reddestInTheMiddle(at70);
    EXPECT_EQ(full[0], 255);
    EXPECT_EQ(full[1], 0);
}

// A range of 1 or less is no haze, 0 (the default) included.
TEST(TerrainHaze, NoHazeAtARangeOf1OrLess) {
    HazeView view;
    EXPECT_EQ(reddened(view.render(0.0f, 70.0)), 0);
    EXPECT_EQ(reddened(view.render(0.5f, 70.0)), 0);
    EXPECT_EQ(reddened(view.render(1.0f, 70.0)), 0);
    EXPECT_GT(reddened(view.render(1.5f, 70.0)), 1000);
}
