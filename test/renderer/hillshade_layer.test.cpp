#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_options.hpp>
#include <mln/renderer/renderer.hpp>
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

struct Fixture {
    util::RunLoop loop;
    std::shared_ptr<StubFileSource> fileSource;
    HeadlessFrontend frontend{{512, 512}, 1};
    MapAdapter map;

    static std::shared_ptr<StubFileSource> makeFileSource() {
        auto source = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
        const std::string tile = makeDEMTile();
        source->tileResponse = [tile](const Resource&) {
            Response res;
            res.data = std::make_shared<std::string>(tile);
            return res;
        };
        return source;
    }

    explicit Fixture(const char* layerType)
        : fileSource(makeFileSource()),
          map(frontend,
              MapObserver::nullObserver(),
              fileSource,
              MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize())) {
        map.getStyle().loadJSON(std::string(R"STYLE({
          "version": 8,
          "sources": {
            "dem": {"type": "raster-dem", "tiles": ["http://example.com/{z}-{x}-{y}.png"], "tileSize": 256, "maxzoom": 9}
          },
          "layers": [{"id": "relief", "source": "dem", "type": ")STYLE") +
                                layerType + R"STYLE("}]})STYLE");
        map.jumpTo(CameraOptions().withCenter(LatLng{47.2692, 11.4041}).withZoom(5.0));
    }

    // Render until a frame no longer changes the number of draw calls, then return that frame's stats.
    gfx::RenderingStats settle() {
        auto stats = frontend.render(map).stats;
        for (int i = 0; i < 8; ++i) {
            const auto previousDrawCalls = stats.totalDrawCalls;
            stats = frontend.render(map).stats;
            if (stats.totalDrawCalls - previousDrawCalls == lastFrameDrawCalls) break;
            lastFrameDrawCalls = stats.totalDrawCalls - previousDrawCalls;
        }
        return stats;
    }
    int lastFrameDrawCalls = -1;
};

// Draw calls of one frame, including the ones of offscreen passes.
int drawCallsOfOneMoreFrame(Fixture& fixture) {
    const auto before = fixture.frontend.render(fixture.map).stats.totalDrawCalls;
    return fixture.frontend.render(fixture.map).stats.totalDrawCalls - before;
}

int drawCallsOfAStillFrame(const char* layerType, const CameraOptions& camera) {
    Fixture fixture(layerType);
    fixture.map.jumpTo(camera);
    (void)fixture.settle();
    return drawCallsOfOneMoreFrame(fixture);
}

const CameraOptions firstView = CameraOptions().withCenter(LatLng{47.2692, 11.4041}).withZoom(5.0);
const CameraOptions secondView = CameraOptions().withCenter(LatLng{-33.9, 151.2}).withZoom(6.2);

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

// Every tile is drawn once on screen, and its elevation data is turned into slope data in an offscreen pass.
// That pass has to run once per tile, not on every frame: a still hillshade frame draws what a color relief
// frame with the same tiles draws.
TEST(HillshadeLayer, AStillFrameRunsNoPreparePass) {
    const int expected = drawCallsOfAStillFrame("color-relief", firstView);
    ASSERT_GT(expected, 0);
    EXPECT_EQ(expected, drawCallsOfAStillFrame("hillshade", firstView));
}

// Tiles that left the view must not leave their prepare passes behind: after moving to other tiles and
// waiting, a still frame is again only the visible tiles.
TEST(HillshadeLayer, AStillFrameAfterMovingRunsNoPreparePass) {
    const int expected = drawCallsOfAStillFrame("color-relief", secondView);
    ASSERT_GT(expected, 0);

    Fixture fixture("hillshade");
    (void)fixture.settle();
    fixture.map.jumpTo(CameraOptions().withCenter(LatLng{10.0, 20.0}).withZoom(7.0));
    (void)fixture.settle();
    fixture.map.jumpTo(secondView);
    (void)fixture.settle();
    EXPECT_EQ(expected, drawCallsOfOneMoreFrame(fixture));
}

namespace {

// Offscreen frame buffers alive once the layer has settled on `camera`, coming from `before` if given:
// then the tile cache is emptied too, and frames drawn after it. Frame buffers are released with the
// targets that own them.
int frameBuffersAfterSettling(const char* layerType, const CameraOptions& camera, const CameraOptions* before) {
    Fixture fixture(layerType);
    if (before) {
        fixture.map.jumpTo(*before);
        (void)fixture.settle();
    }
    fixture.map.jumpTo(camera);
    (void)fixture.settle();
    if (before) {
        // Tiles that left the cover stay in the tile cache, and the buckets there own their targets
        // (and their frame buffers) in turn; empty the cache, so that only a target someone else still
        // holds survives.
        fixture.frontend.getRenderer()->reduceMemoryUse();
        for (int i = 0; i < 4; ++i) {
            fixture.loop.runOnce();
            (void)fixture.frontend.render(fixture.map);
        }
    }
    return fixture.settle().numFrameBuffers;
}

} // namespace

// A tile's prepare target (and its frame buffer and DEM texture) is released once the tile has left the
// cover: the layer tells the renderer to drop it. Otherwise the renderer keeps every target the layer ever
// made, long after the tile's bucket is gone, and a long pan runs the device out of memory.
// Two places do this release (the layer's request to the renderer, and the renderer's own sweep of targets
// only its list still holds), so the test fails only when both are broken.
TEST(HillshadeLayer, PrepareTargetsOfTilesThatLeftTheCoverAreReleased) {
    const int colorReliefHere = frameBuffersAfterSettling("color-relief", secondView, nullptr);
    const int hillshadeHere = frameBuffersAfterSettling("hillshade", secondView, nullptr);
    // The test only means something if the layer holds targets at all
    if (hillshadeHere == 0) {
        GTEST_SKIP() << "this backend's hillshade layer holds no offscreen targets";
    }
    ASSERT_GT(hillshadeHere, colorReliefHere);

    // After a visit to another place, the layer holds what a layer that started here holds.
    EXPECT_LE(frameBuffersAfterSettling("hillshade", secondView, &firstView), hillshadeHere);
}

// The elevation tile of a color relief tile only has to be uploaded once.
TEST(ColorReliefLayer, AStillFrameCreatesNoTextures) {
    Fixture fixture("color-relief");
    (void)fixture.settle();
    const auto settled = fixture.frontend.render(fixture.map).stats;
    if (settled.numCreatedTextures == 0) {
        GTEST_SKIP() << "this backend does not count the textures it creates";
    }
    const auto still = fixture.frontend.render(fixture.map).stats;
    EXPECT_EQ(0, still.numCreatedTextures - settled.numCreatedTextures);
    EXPECT_EQ(0u, still.textureUpdateBytes - settled.textureUpdateBytes);
}
