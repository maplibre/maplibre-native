#include <benchmark/benchmark.h>

#include <mln/style/filter.hpp>
#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/filter.hpp>
#include <mln/style/conversion_impl.hpp>
#include <mln/tile/geometry_tile_data.hpp>
#include <mln/benchmark/stub_geometry_tile_feature.hpp>

using namespace mln;

style::Filter parse(const char* expression) {
    style::conversion::Error error;
    return *style::conversion::convertJSON<style::Filter>(expression, error);
}

static void Parse_Filter(benchmark::State& state) {
    while (state.KeepRunning()) {
        parse(R"FILTER(["==", "foo", "bar"])FILTER");
    }
}

static void Parse_EvaluateFilter(benchmark::State& state) {
    const style::Filter filter = parse(R"FILTER(["==", "foo", "bar"])FILTER");
    const StubGeometryTileFeature feature = {{}, FeatureType::Unknown, {}, {{"foo", std::string("bar")}}};
    const style::expression::EvaluationContext context(&feature);

    while (state.KeepRunning()) {
        filter(context);
    }
}

BENCHMARK(Parse_Filter);
BENCHMARK(Parse_EvaluateFilter);
