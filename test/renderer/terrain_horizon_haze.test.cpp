#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/bound_options.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/style/image.hpp>
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

// The same ground with two blue icons on it: one at the map center, where there is no haze, and one
// 6.6 km north of it, at 2.2 times the center's distance from the camera at pitch 70 (the camera is
// 5.1 km from the center at zoom 13), past a range of 2.
const char* hazeIconStyle = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/dem/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 11,
      "tileSize": 256
    },
    "icons": {
      "type": "geojson",
      "data": {
        "type": "FeatureCollection",
        "features": [
          {"type": "Feature", "properties": {}, "geometry": {"type": "Point", "coordinates": [10.0, 46.0]}},
          {"type": "Feature", "properties": {}, "geometry": {"type": "Point", "coordinates": [10.0, 46.0596]}}
        ]
      }
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": [
    {"id": "ground", "type": "background", "paint": {"background-color": "rgb(0, 200, 0)"}},
    {"id": "icons", "type": "symbol", "source": "icons",
     "layout": {"icon-image": "dot", "icon-allow-overlap": true, "icon-ignore-placement": true}}
  ]
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
    explicit HazeView(const char* style = hazeStyle) {
        fileSource->tileResponse = [this](const Resource&) {
            Response res;
            res.data = std::make_shared<std::string>(dem);
            return res;
        };
        map.getStyle().loadJSON(style);
        // The icon of hazeIconStyle: 24 px of solid blue.
        PremultipliedImage dot({24, 24});
        for (size_t i = 0; i < dot.bytes(); i += 4) {
            dot.data[i] = 0;
            dot.data[i + 1] = 0;
            dot.data[i + 2] = 255;
            dot.data[i + 3] = 255;
        }
        map.getStyle().addImage(std::make_unique<style::Image>("dot", std::move(dot), 1.0f));
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

// Pixels of an icon's blue in the rows from `top` up to `bottom`.
int blue(const PremultipliedImage& image, uint32_t top, uint32_t bottom) {
    int count = 0;
    for (uint32_t y = top; y < bottom; ++y) {
        for (uint32_t x = 0; x < image.size.width; ++x) {
            const uint8_t* px = pixel(image, x, y);
            count += px[2] > 200 && px[0] < 50 && px[1] < 50 ? 1 : 0;
        }
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

// An icon stands on the ground, and where the haze has taken the ground it has taken the icon: one
// left drawn there stands in the sky over ground that is no longer to be seen (owner report on iOS
// build 209: trailhead markers in a row above the horizon). The icon at the center, where there is
// no haze, is drawn as it is. Without haze both are.
TEST(TerrainHaze, AnIconOverHazedGroundGoesWithTheGround) {
    HazeView view(hazeIconStyle);

    // The far icon is in the upper half of the view, the near one about the middle.
    const PremultipliedImage clear = view.render(0.0f, 70.0);
    EXPECT_GT(blue(clear, 0, 200), 100);
    EXPECT_GT(blue(clear, 200, 320), 300);

    const PremultipliedImage hazed = view.render(2.0f, 70.0);
    EXPECT_EQ(blue(hazed, 0, 200), 0);
    EXPECT_EQ(blue(hazed, 200, 320), blue(clear, 200, 320));

    // With the haze reaching three times as far, the ground under the far icon has taken a tenth
    // of it, and the icon is drawn.
    EXPECT_GT(blue(view.render(6.0f, 70.0), 0, 200), 100);
}
