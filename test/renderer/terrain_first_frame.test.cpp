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
#include <cmath>
#include <cstdio>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <vector>

using namespace mln;

namespace {

// The camera sits on the line between two tiles of zoom 5 and every finer one: the tiles west of it
// are the left half of the view, the tiles east of it the right half.
constexpr double splitLongitude = 11.25;

std::string areaStyle(const char* encoding) {
    return std::string(R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/dem/{z}-{x}-{y}.png"],
      "encoding": ")STYLE") +
           encoding + R"STYLE(",
      "maxzoom": 9,
      "tileSize": 256
    },
    "area": {
      "type": "raster",
      "tiles": ["http://example.com/area/{z}-{x}-{y}.png"],
      "tileSize": 256
    }
  },
  "layers": [
    {"id": "area", "type": "raster", "source": "area", "paint": {"raster-fade-duration": 0}}
  ]
})STYLE";
}

// Each tile is red in its top half and green in its left half, so ground that is missing, drawn
// somewhere else or flipped shows as another color.
std::string areaTile() {
    PremultipliedImage image({256, 256});
    for (uint32_t y = 0; y < 256; ++y) {
        for (uint32_t x = 0; x < 256; ++x) {
            uint8_t* px = image.data.get() + (y * 256 + x) * 4;
            px[0] = y < 128 ? 255 : 0;
            px[1] = x < 128 ? 255 : 0;
            px[2] = 0;
            px[3] = 255;
        }
    }
    return encodePNG(image);
}

// Elevation of 0 m in the encoding: Mapbox (0 + 10000) / 0.1 = 100000 = 1 * 65536 + 134 * 256 + 160,
// Terrarium 0 + 32768 = 128 * 256.
std::string seaLevelDEMTile(bool terrarium) {
    PremultipliedImage image({256, 256});
    for (uint32_t i = 0; i < 256 * 256; ++i) {
        image.data[i * 4 + 0] = terrarium ? 128 : 1;
        image.data[i * 4 + 1] = terrarium ? 0 : 134;
        image.data[i * 4 + 2] = terrarium ? 0 : 160;
        image.data[i * 4 + 3] = 255;
    }
    return encodePNG(image);
}

// Whether the tile "z-x-y.png" of the URL lies west of the line through the camera.
bool westOfTheCamera(const std::string& url) {
    int z = 0, x = 0, y = 0;
    const auto name = url.substr(url.rfind('/') + 1);
    std::sscanf(name.c_str(), "%d-%d-%d.png", &z, &x, &y);
    const double west = (x / std::pow(2.0, z)) * 360.0 - 180.0;
    const double east = ((x + 1) / std::pow(2.0, z)) * 360.0 - 180.0;
    return z >= 5 && east <= splitLongitude + 1e-9 && west < splitLongitude;
}

struct FirstFrameTest {
    std::string area = areaTile();
    std::string dem;
    // Which elevation tiles answer: none, or the ones west of the camera.
    bool demWest;
    util::RunLoop loop;
    std::shared_ptr<StubFileSource> fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(),
                                                                                  ClientOptions());
    // Only the frames the test asks for render.
    HeadlessFrontend frontend{
        {256, 256}, 1, gfx::HeadlessBackend::SwapBehaviour::NoFlush, gfx::ContextMode::Unique, std::nullopt, false};
    MapAdapter map;
    bool demTextureMade = false;

    FirstFrameTest(bool terrarium, bool demWest_)
        : dem(seaLevelDEMTile(terrarium)),
          demWest(demWest_),
          map(frontend,
              MapObserver::nullObserver(),
              fileSource,
              MapOptions().withMapMode(MapMode::Continuous).withSize(frontend.getSize())) {
        // The area's tiles come at once. An elevation tile comes, or is asked for and does not.
        fileSource->tileResponse = [this](const Resource& resource) -> std::optional<Response> {
            Response res;
            if (resource.url.find("/dem/") == std::string::npos) {
                res.data = std::make_shared<std::string>(area);
            } else if (demWest && westOfTheCamera(resource.url)) {
                res.data = std::make_shared<std::string>(dem);
            } else {
                return std::nullopt;
            }
            return res;
        };
        map.getStyle().loadJSON(areaStyle(terrarium ? "terrarium" : "mapbox"));
        map.jumpTo(CameraOptions().withCenter(LatLng{46.0, splitLongitude}).withZoom(8.6).withPitch(0.0));
    }

    // One frame, read back.
    PremultipliedImage oneFrame() {
        loop.runOnce();
        frontend.renderFrame();
        gfx::BackendScope guard{*frontend.getBackend()};
        demTextureMade = demTextureMade ||
                         frontend.getBackend()->getContext().renderingStats().numDEMTexturesCreated > 0;
        return frontend.readStillImage();
    }

    static size_t opaquePixels(const PremultipliedImage& image) {
        size_t count = 0;
        for (size_t i = 0; i < image.bytes(); i += 4) {
            count += image.data[i + 3] > 127 ? 1 : 0;
        }
        return count;
    }

    // The flat map once every tile it asks for is in and drawn: the picture the first frame with terrain
    // has to match. A frame with no gap is not enough, since a parent tile fills the gap of a tile
    // still loading and a finer tile that arrives later redraws that part.
    PremultipliedImage flatFrame() {
        PremultipliedImage image;
        for (int i = 0; i < 500; ++i) {
            image = oneFrame();
            if (opaquePixels(image) == image.bytes() / 4 && map.isFullyLoaded()) {
                // Tiles that finished loading during that frame show in this one
                return oneFrame();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return image;
    }

    // How many pixels have the same red, green and opacity as the flat map's.
    static size_t pixelsLikeTheFlatMap(const PremultipliedImage& flat, const PremultipliedImage& image) {
        size_t same = 0;
        for (size_t i = 0; i < image.bytes(); i += 4) {
            const bool red = (flat.data[i] > 127) == (image.data[i] > 127);
            const bool green = (flat.data[i + 1] > 127) == (image.data[i + 1] > 127);
            const bool opaque = image.data[i + 3] > 127;
            same += red && green && opaque ? 1 : 0;
        }
        return same;
    }
};

} // namespace

// Terrain comes on over elevation that has not arrived. The first frame with terrain shows the same
// map as the flat frame before it: it used to draw nothing until the first elevation tile was in,
// then, with an empty elevation texel that decodes to -10,000 m, ground that far under the map.
TEST(TerrainFirstFrame, TerrainSetBeforeItsElevationLoadsShowsTheMapFromTheFirstFrame) {
    for (const float exaggeration : {1.0f, 1.4f}) {
        FirstFrameTest test(false, false);
        const PremultipliedImage flat = test.flatFrame();
        ASSERT_EQ(FirstFrameTest::opaquePixels(flat), flat.bytes() / 4) << "the flat map shows";

        test.map.getStyle().setTerrain(std::make_unique<style::Terrain>("dem", exaggeration));
        const PremultipliedImage first = test.oneFrame();

        const size_t same = FirstFrameTest::pixelsLikeTheFlatMap(flat, first);
        const size_t pixels = flat.bytes() / 4;
        // Only pixels on a color edge may differ.
        EXPECT_GT(same, pixels * 95 / 100)
            << same << " of " << pixels << " pixels of the first terrain frame match the flat map, exaggeration "
            << exaggeration;
    }
}

// An elevation tile of the mesh has loaded and its neighbor has not. The neighbor is flat ground at
// sea level, in the encoding the loaded tile has: the placeholder is read with the unpack vector of
// that encoding, which is the Mapbox one until a tile says otherwise.
TEST(TerrainFirstFrame, TheTilesWaitingForTheirElevationStayOnFlatGroundWhenAMapboxTileLoads) {
    FirstFrameTest test(false, true);
    const PremultipliedImage flat = test.flatFrame();
    ASSERT_EQ(FirstFrameTest::opaquePixels(flat), flat.bytes() / 4) << "the flat map shows";

    test.map.getStyle().setTerrain(std::make_unique<style::Terrain>("dem", 1.4f));
    PremultipliedImage image;
    for (int i = 0; i < 200 && !test.demTextureMade; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        image = test.oneFrame();
    }
    ASSERT_TRUE(test.demTextureMade) << "an elevation tile loaded";
    image = test.oneFrame();

    const size_t same = FirstFrameTest::pixelsLikeTheFlatMap(flat, image);
    const size_t pixels = flat.bytes() / 4;
    EXPECT_GT(same, pixels * 95 / 100) << same << " of " << pixels << " pixels match the flat map";
}

TEST(TerrainFirstFrame, TheTilesWaitingForTheirElevationStayOnFlatGroundWhenATerrariumTileLoads) {
    FirstFrameTest test(true, true);
    const PremultipliedImage flat = test.flatFrame();
    ASSERT_EQ(FirstFrameTest::opaquePixels(flat), flat.bytes() / 4) << "the flat map shows";

    test.map.getStyle().setTerrain(std::make_unique<style::Terrain>("dem", 1.4f));
    PremultipliedImage image;
    for (int i = 0; i < 200 && !test.demTextureMade; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        image = test.oneFrame();
    }
    ASSERT_TRUE(test.demTextureMade) << "an elevation tile loaded";
    image = test.oneFrame();

    const size_t same = FirstFrameTest::pixelsLikeTheFlatMap(flat, image);
    const size_t pixels = flat.bytes() / 4;
    EXPECT_GT(same, pixels * 95 / 100) << same << " of " << pixels << " pixels match the flat map";
}
