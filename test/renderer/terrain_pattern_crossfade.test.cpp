#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/headless_frontend.hpp>
#include <mln/layermanager/layer_manager.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_observer.hpp>
#include <mln/map/map_options.hpp>
#include <mln/map/zoom_history.hpp>
#include <mln/renderer/property_evaluation_parameters.hpp>
#include <mln/renderer/render_layer.hpp>
#include <mln/style/image.hpp>
#include <mln/style/layers/background_layer.hpp>
#include <mln/style/layers/circle_layer.hpp>
#include <mln/style/layers/fill_extrusion_layer.hpp>
#include <mln/style/layers/fill_layer.hpp>
#include <mln/style/layers/line_layer.hpp>
#include <mln/style/layers/symbol_layer.hpp>
#include <mln/style/style.hpp>
#include <mln/style/transition_options.hpp>
#include <mln/util/image.hpp>
#include <mln/util/run_loop.hpp>

#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>
#include <thread>

using namespace mln;

namespace {

// Flat ground under a fill-pattern polygon that covers the whole view. The pattern is eight pixels
// across, four red and four blue: a different scale puts the stripes elsewhere, so a pixel says
// which scale it was drawn at. The layer's opacity depends on zoom, as a real layer's does: when the
// view crosses a zoom level, only a layer whose style refers to zoom is evaluated, and so starts a fade.
const char* styleJSON = R"STYLE({
  "version": 8,
  "sources": {
    "dem": {
      "type": "raster-dem",
      "tiles": ["http://example.com/dem/{z}-{x}-{y}.png"],
      "encoding": "mapbox",
      "maxzoom": 11,
      "tileSize": 256
    },
    "ground": {
      "type": "geojson",
      "data": {
        "type": "Feature",
        "properties": {},
        "geometry": {
          "type": "Polygon",
          "coordinates": [[[9.9, 45.9], [10.1, 45.9], [10.1, 46.1], [9.9, 46.1], [9.9, 45.9]]]
        }
      }
    }
  },
  "terrain": {"source": "dem", "exaggeration": 1.0},
  "layers": [{"id": "ground", "type": "fill", "source": "ground",
              "paint": {"fill-pattern": "stripes", "fill-antialias": false,
                        "fill-opacity": ["interpolate", ["linear"], ["zoom"], 0, 1, 24, 1]}}]
})STYLE";

std::string demTile() {
    PremultipliedImage image({256, 256});
    for (uint32_t i = 0; i < 256 * 256; ++i) {
        uint8_t* px = image.data.get() + i * 4;
        px[0] = 1; // sea level in Terrain-RGB
        px[1] = 134;
        px[2] = 160;
        px[3] = 255;
    }
    return encodePNG(image);
}

std::unique_ptr<style::Image> stripes() {
    PremultipliedImage image({8, 8});
    for (uint32_t y = 0; y < 8; ++y) {
        for (uint32_t x = 0; x < 8; ++x) {
            uint8_t* px = image.data.get() + (y * 8 + x) * 4;
            px[0] = x < 4 ? 255 : 0;
            px[1] = 0;
            px[2] = x < 4 ? 0 : 255;
            px[3] = 255;
        }
    }
    return std::make_unique<style::Image>("stripes", std::move(image), 1.0f);
}

constexpr uint32_t compareFrom = 64;
constexpr uint32_t compareTo = 192;
constexpr int compareArea = (compareTo - compareFrom) * (compareTo - compareFrom);

// How many pixels of the compared region are drawn in more than 150 of `channel` (0 red, 2 blue).
int strong(const PremultipliedImage& image, int channel) {
    int count = 0;
    for (uint32_t y = compareFrom; y < compareTo; ++y) {
        for (uint32_t x = compareFrom; x < compareTo; ++x) {
            if (image.data.get()[(y * image.size.width + x) * 4 + channel] > 150) ++count;
        }
    }
    return count;
}

int differing(const PremultipliedImage& a, const PremultipliedImage& b) {
    int count = 0;
    for (uint32_t y = compareFrom; y < compareTo; ++y) {
        for (uint32_t x = compareFrom; x < compareTo; ++x) {
            const uint8_t* pa = a.data.get() + (y * a.size.width + x) * 4;
            const uint8_t* pb = b.data.get() + (y * b.size.width + x) * 4;
            if (std::abs(int(pa[0]) - int(pb[0])) > 40 || std::abs(int(pa[2]) - int(pb[2])) > 40) ++count;
        }
    }
    return count;
}

// The map is idle once every tile of the view is loaded and nothing is left to repaint.
class IdleObserver final : public MapObserver {
public:
    void onDidBecomeIdle() override { idle = true; }
    bool idle = false;
};

// A continuous map at `startZoom`, moved to `zoom` (across a zoom level boundary when they differ); the
// frame it ends on. Each stop is drawn until the map is idle and the compared region shows both stripe
// colors, or for 10 s: a slow runner gets a blank frame later, not sooner, and the tiles of the new zoom
// level are in before the frame is read.
PremultipliedImage drawnAt(double startZoom, double zoom) {
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions());
    const std::string dem = demTile();
    fileSource->tileResponse = [&](const Resource&) {
        Response res;
        res.data = std::make_shared<std::string>(dem);
        return res;
    };
    IdleObserver observer;
    HeadlessFrontend frontend{{256, 256}, 1};
    MapAdapter map(
        frontend, observer, fileSource, MapOptions().withMapMode(MapMode::Continuous).withSize(frontend.getSize()));
    map.getStyle().loadJSON(styleJSON);
    // A slow fade, so the tiles of a slow runner arrive inside it, as a phone's do inside 300 ms.
    map.getStyle().setTransitionOptions(style::TransitionOptions{std::chrono::milliseconds(2500), std::nullopt, false});
    map.getStyle().addImage(stripes());
    const auto showsStripes = [&] {
        gfx::BackendScope guard{*frontend.getBackend()};
        const PremultipliedImage image = frontend.readStillImage();
        return strong(image, 0) > compareArea / 4 && strong(image, 2) > compareArea / 4;
    };
    const auto settle = [&] {
        observer.idle = false;
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
        while (std::chrono::steady_clock::now() < deadline && !(observer.idle && showsStripes())) {
            loop.runOnce();
            std::this_thread::sleep_for(std::chrono::milliseconds(8));
        }
    };
    map.jumpTo(CameraOptions().withCenter(LatLng{46.0, 10.0}).withZoom(startZoom).withPitch(0.0));
    settle();
    if (zoom != startZoom) {
        map.jumpTo(CameraOptions().withCenter(LatLng{46.0, 10.0}).withZoom(zoom).withPitch(0.0));
        settle();
    }
    gfx::BackendScope guard{*frontend.getBackend()};
    return frontend.readStillImage();
}

// The view crossed into zoom level 16 from `from`, is at `z` now, and the crossing was `elapsed` ago. The
// style's transition duration is the default 300 ms.
PropertyEvaluationParameters crossedAgo(float from, float z, Duration elapsed, bool settled) {
    const TimePoint crossedAt = TimePoint() + std::chrono::seconds(10);
    ZoomHistory history;
    history.update(from, crossedAt);
    history.update(z, crossedAt);
    PropertyEvaluationParameters parameters{history, crossedAt + elapsed, std::chrono::milliseconds(300)};
    parameters.crossfadeSettled = settled;
    return parameters;
}

void expectSame(const CrossfadeParameters& a, const CrossfadeParameters& b) {
    EXPECT_EQ(a.fromScale, b.fromScale);
    EXPECT_EQ(a.toScale, b.toScale);
    EXPECT_EQ(a.t, b.t);
}

} // namespace

// A pattern is drawn at two scales that cross-fade over the style's transition duration (300 ms by
// default) after the view crosses a zoom level. A drape target keeps what it was baked with until its
// tiles change, so a target baked inside the fade kept the blend for good: the hatch of one tile was
// finer than its neighbors'.
TEST(TerrainDrape, APatternDrapedDuringAZoomLevelFadeSettlesOnTheFinalScale) {
    const PremultipliedImage settled = drawnAt(16.1, 16.1); // never crossed a level: the final scale
    const PremultipliedImage crossed = drawnAt(15.9, 16.1); // baked as the tiles of the new level came in
    // Both red and blue stripes are there, so the comparison below is between drawn patterns, not blank frames.
    ASSERT_GT(strong(settled, 0), compareArea / 4);
    ASSERT_GT(strong(settled, 2), compareArea / 4);
    EXPECT_LT(differing(settled, crossed), compareArea / 20);
}

// With terrain on, every layer but the fill-extrusion has its fade settled, and the fill-extrusion, drawn into
// the main pass where the fade works, does not. Without terrain no layer does. Circle and symbol layers have
// no cross-fade, so the flag changes nothing for them.
TEST(TerrainDrape, EveryLayerButTheExtrusionSettlesItsCrossfade) {
    const auto settles = [](const style::Layer& layer, bool terrainEnabled) {
        return LayerManager::get()->createRenderLayer(layer.baseImpl)->settlesCrossfade(terrainEnabled);
    };
    const style::FillLayer fill("fill", "source");
    const style::LineLayer line("line", "source");
    const style::BackgroundLayer background("background");
    const style::CircleLayer circle("circle", "source");
    const style::SymbolLayer symbol("symbol", "source");
    const style::FillExtrusionLayer extrusion("extrusion", "source");
    EXPECT_TRUE(settles(fill, true));
    EXPECT_TRUE(settles(line, true));
    EXPECT_TRUE(settles(background, true));
    EXPECT_TRUE(settles(circle, true));
    EXPECT_TRUE(settles(symbol, true));
    EXPECT_FALSE(settles(extrusion, true));
    EXPECT_FALSE(settles(fill, false));
    EXPECT_FALSE(settles(line, false));
    EXPECT_FALSE(settles(background, false));
    EXPECT_FALSE(settles(circle, false));
    EXPECT_FALSE(settles(symbol, false));
    EXPECT_FALSE(settles(extrusion, false));
}

// Without the flag, the cross-fade is the one every other map has: it runs from the zoom level change
// over the transition duration, from the old scale of the image to its final one.
TEST(PropertyEvaluationParameters, ACrossfadeRunsOverTheTransitionDuration) {
    struct Step {
        int elapsedMs;
        float t;
    };
    // The fraction is .25 zooming in to 16.25 and .75 zooming out to 15.75, so t starts at .25 either way.
    constexpr Step steps[] = {{0, 0.25f}, {150, 0.625f}, {300, 1.0f}, {5000, 1.0f}};
    for (const Step& step : steps) {
        const auto zoomIn =
            crossedAgo(15.75f, 16.25f, std::chrono::milliseconds(step.elapsedMs), false).getCrossfadeParameters();
        EXPECT_EQ(zoomIn.fromScale, 2.0f); // zooming in, the image fades in from twice its scale
        EXPECT_EQ(zoomIn.toScale, 1.0f);
        EXPECT_FLOAT_EQ(zoomIn.t, step.t) << "zoom in, " << step.elapsedMs << " ms";

        const auto zoomOut =
            crossedAgo(16.25f, 15.75f, std::chrono::milliseconds(step.elapsedMs), false).getCrossfadeParameters();
        EXPECT_EQ(zoomOut.fromScale, 0.5f); // zooming out, from half its scale
        EXPECT_EQ(zoomOut.toScale, 1.0f);
        EXPECT_FLOAT_EQ(zoomOut.t, step.t) << "zoom out, " << step.elapsedMs << " ms";
    }
}

// Settled, the cross-fade is where the unsettled one ends up once its duration has passed, at any time
// since the zoom level changed and whichever way the view went.
TEST(PropertyEvaluationParameters, ASettledCrossfadeIsTheEndOfTheFadeAtAnyTime) {
    using std::chrono::milliseconds;
    const auto zoomIn = crossedAgo(15.75f, 16.25f, milliseconds(5000), false).getCrossfadeParameters();
    const auto zoomOut = crossedAgo(16.25f, 15.75f, milliseconds(5000), false).getCrossfadeParameters();
    for (const int elapsed : {0, 1, 150, 299, 300, 5000}) {
        expectSame(crossedAgo(15.75f, 16.25f, milliseconds(elapsed), true).getCrossfadeParameters(), zoomIn);
        expectSame(crossedAgo(16.25f, 15.75f, milliseconds(elapsed), true).getCrossfadeParameters(), zoomOut);
    }
}

// A layer whose fade has run out stops being evaluated every frame, which it tests as t != 1.
TEST(PropertyEvaluationParameters, ASettledCrossfadeIsExactlyOne) {
    int notOne = 0;
    // Fractions that are not powers of two apart, which float arithmetic has to round.
    for (int step = 0; step < 4099; ++step) {
        const float fraction = float(step) / 4099.0f;
        notOne += crossedAgo(15.5f, 16.0f + fraction, Duration::zero(), true).getCrossfadeParameters().t != 1.0f;
        notOne += crossedAgo(16.5f, 15.0f + fraction, Duration::zero(), true).getCrossfadeParameters().t != 1.0f;
    }
    EXPECT_EQ(notOne, 0);
}
