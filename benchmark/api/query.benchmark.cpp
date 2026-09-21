#include <benchmark/benchmark.h>

#include <mln/map/map.hpp>
#include <mln/map/map_options.hpp>
#include <mln/gfx/headless_frontend.hpp>
#include <mln/renderer/renderer.hpp>
#include <mln/style/style.hpp>
#include <mln/style/image.hpp>
#include <mln/storage/network_status.hpp>
#include <mln/util/image.hpp>
#include <mln/util/io.hpp>
#include <mln/util/run_loop.hpp>

using namespace mln;

namespace {

class QueryBenchmark {
public:
    QueryBenchmark() {
        NetworkStatus::Set(NetworkStatus::Status::Offline);

        map.getStyle().loadJSON(util::read_file("benchmark/fixtures/api/style.json"));
        map.jumpTo(CameraOptions().withCenter(LatLng{40.726989, -73.992857}).withZoom(15.0)); // Manhattan
        map.getStyle().addImage(std::make_unique<style::Image>(
            "test-icon", decodeImage(util::read_file("benchmark/fixtures/api/default_marker.png")), 1.0f));

        frontend.render(map);
    }

    util::RunLoop loop;
    HeadlessFrontend frontend{{1000, 1000}, 1};
    Map map{frontend,
            MapObserver::nullObserver(),
            MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()),
            ResourceOptions().withCachePath("benchmark/fixtures/api/cache.db").withAssetPath(".").withApiKey("foobar")};
    ScreenBox box{{0, 0}, {1000, 1000}};
};

} // end namespace

static void API_queryPixelsForLatLngs(::benchmark::State& state) {
    std::vector<LatLng> points;
    int count = 10000;
    for (int i = 0; i < count; ++i) {
        points.emplace_back(1, 1);
    }
    QueryBenchmark bench;
    while (state.KeepRunning()) {
        (void)bench.map.pixelsForLatLngs(points);
    }
}

static void API_queryLatLngsForPixels(::benchmark::State& state) {
    std::vector<ScreenCoordinate> points;
    int count = 10000;
    for (int i = 0; i < count; ++i) {
        points.emplace_back(1, 1);
    }
    QueryBenchmark bench;
    while (state.KeepRunning()) {
        (void)bench.map.latLngsForPixels(points);
    }
}

static void API_queryRenderedFeaturesAll(::benchmark::State& state) {
    QueryBenchmark bench;

    while (state.KeepRunning()) {
        bench.frontend.getRenderer()->queryRenderedFeatures(bench.box, {});
    }
}

static void API_queryRenderedFeaturesLayerFromLowDensity(::benchmark::State& state) {
    QueryBenchmark bench;

    while (state.KeepRunning()) {
        bench.frontend.getRenderer()->queryRenderedFeatures(bench.box, {{{"testlayer"}}, {}});
    }
}

static void API_queryRenderedFeaturesLayerFromHighDensity(::benchmark::State& state) {
    QueryBenchmark bench;

    while (state.KeepRunning()) {
        bench.frontend.getRenderer()->queryRenderedFeatures(bench.box, {{{"road-street"}}, {}});
    }
}
BENCHMARK(API_queryPixelsForLatLngs);
BENCHMARK(API_queryLatLngsForPixels);
BENCHMARK(API_queryRenderedFeaturesAll)->Iterations(50);
BENCHMARK(API_queryRenderedFeaturesLayerFromLowDensity);
BENCHMARK(API_queryRenderedFeaturesLayerFromHighDensity);
