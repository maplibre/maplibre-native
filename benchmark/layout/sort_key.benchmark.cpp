#include <benchmark/benchmark.h>

#include <mln/layout/layout.hpp>
#include <mln/layermanager/circle_layer_factory.hpp>
#include <mln/layermanager/fill_layer_factory.hpp>
#include <mln/layermanager/line_layer_factory.hpp>
#include <mln/layermanager/symbol_layer_factory.hpp>
#include <mln/renderer/bucket_parameters.hpp>
#include <mln/style/expression/dsl.hpp>
#include <mln/style/layers/circle_layer_impl.hpp>
#include <mln/style/layers/fill_layer_impl.hpp>
#include <mln/style/layers/line_layer_impl.hpp>
#include <mln/style/layers/symbol_layer_impl.hpp>
#include <mln/tile/geojson_tile_data.hpp>

using namespace mln;
using namespace mln::style;

namespace {

enum class SortKeyType {
    Undefined,
    Constant,
    Camera,
    DataDriven
};

PropertyValue<float> sortKey(SortKeyType type) {
    using namespace expression::dsl;
    switch (type) {
        case SortKeyType::Undefined:
            return {};
        case SortKeyType::Constant:
            return 5.0f;
        case SortKeyType::Camera:
            return PropertyExpression<float>(step(zoom(), literal(5.0), 10.0, literal(10.0)));
        case SortKeyType::DataDriven:
            return PropertyExpression<float>(number(get("rank")));
    }
    return {};
}

template <class Properties, class Layer>
void benchmarkLayout(benchmark::State& state, const Layer& layer, LayerFactory&& factory, FeatureType type) {
    const auto count = static_cast<size_t>(state.range(0));
    mapbox::feature::feature_collection<int16_t> features;
    features.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        const auto x = static_cast<int16_t>(10 + (i % 100) * 40);
        const auto y = static_cast<int16_t>(10 + (i / 100) * 40);
        mapbox::geometry::geometry<int16_t> geometry;
        if (type == FeatureType::Polygon) {
            geometry = mapbox::geometry::polygon<int16_t>{
                {{x, y}, {x, static_cast<int16_t>(y + 10)}, {static_cast<int16_t>(x + 10), y}, {x, y}}};
        } else if (type == FeatureType::LineString) {
            geometry = mapbox::geometry::line_string<int16_t>{{x, y}, {x, static_cast<int16_t>(y + 10)}};
        } else {
            geometry = mapbox::geometry::point<int16_t>{x, y};
        }
        // A deterministic, unsorted permutation for the data-driven case.
        features.emplace_back(std::move(geometry), PropertyMap{{"rank", static_cast<double>((i * 37) % count)}});
    }
    GeoJSONTileData data(std::move(features));
    const BucketParameters bucketParameters{.tileID = OverscaledTileID(0, 0, 0),
                                            .mode = MapMode::Continuous,
                                            .pixelRatio = 1.0f,
                                            .layerType = factory.getTypeInfo()};
    GlyphDependencies glyphDependencies;
    ImageDependencies imageDependencies;
    std::set<std::string> availableImages{"marker"};
    const LayoutParameters parameters{.bucketParameters = bucketParameters,
                                      .fontFaces = nullptr,
                                      .glyphDependencies = glyphDependencies,
                                      .imageDependencies = imageDependencies,
                                      .availableImages = availableImages};
    const std::vector<Immutable<LayerProperties>> group{
        makeMutable<Properties>(staticImmutableCast<typename Layer::Impl>(layer.baseImpl))};

    while (state.KeepRunning()) {
        state.PauseTiming();
        auto sourceLayer = data.getLayer("");
        glyphDependencies = {};
        imageDependencies.clear();
        state.ResumeTiming();

        // Measure layout construction only, excluding setup and destruction.
        // Bucket creation, symbol placement, and rendering are not performed.
        auto layout = factory.createLayout(parameters, std::move(sourceLayer), group);
        benchmark::DoNotOptimize(layout);

        state.PauseTiming();
        layout.reset();
        state.ResumeTiming();
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}

template <SortKeyType Type>
void Layout_FillSortKey(benchmark::State& state) {
    FillLayer layer("fill", "source");
    layer.setFillSortKey(sortKey(Type));
    benchmarkLayout<FillLayerProperties>(state, layer, FillLayerFactory{}, FeatureType::Polygon);
}

template <SortKeyType Type>
void Layout_LineSortKey(benchmark::State& state) {
    LineLayer layer("line", "source");
    layer.setLineSortKey(sortKey(Type));
    benchmarkLayout<LineLayerProperties>(state, layer, LineLayerFactory{}, FeatureType::LineString);
}

template <SortKeyType Type>
void Layout_CircleSortKey(benchmark::State& state) {
    CircleLayer layer("circle", "source");
    layer.setCircleSortKey(sortKey(Type));
    benchmarkLayout<CircleLayerProperties>(state, layer, CircleLayerFactory{}, FeatureType::Point);
}

template <SortKeyType Type>
void Layout_SymbolSortKey(benchmark::State& state) {
    SymbolLayer layer("symbol", "source");
    layer.setSymbolSortKey(sortKey(Type));
    // Without an icon or text, SymbolLayout returns before processing features.
    layer.setIconImage(expression::Image("marker"));
    benchmarkLayout<SymbolLayerProperties>(state, layer, SymbolLayerFactory{}, FeatureType::Point);
}

void layoutArguments(benchmark::internal::Benchmark* benchmark) {
    benchmark->Arg(2000)->Arg(8000)->Unit(benchmark::kMillisecond)->UseRealTime();
}

BENCHMARK_TEMPLATE(Layout_FillSortKey, SortKeyType::Undefined)->Apply(layoutArguments);
BENCHMARK_TEMPLATE(Layout_FillSortKey, SortKeyType::Constant)->Apply(layoutArguments);
BENCHMARK_TEMPLATE(Layout_FillSortKey, SortKeyType::Camera)->Apply(layoutArguments);
BENCHMARK_TEMPLATE(Layout_FillSortKey, SortKeyType::DataDriven)->Apply(layoutArguments);
BENCHMARK_TEMPLATE(Layout_LineSortKey, SortKeyType::Undefined)->Apply(layoutArguments);
BENCHMARK_TEMPLATE(Layout_LineSortKey, SortKeyType::Constant)->Apply(layoutArguments);
BENCHMARK_TEMPLATE(Layout_LineSortKey, SortKeyType::Camera)->Apply(layoutArguments);
BENCHMARK_TEMPLATE(Layout_LineSortKey, SortKeyType::DataDriven)->Apply(layoutArguments);
BENCHMARK_TEMPLATE(Layout_CircleSortKey, SortKeyType::Undefined)->Apply(layoutArguments);
BENCHMARK_TEMPLATE(Layout_CircleSortKey, SortKeyType::Constant)->Apply(layoutArguments);
BENCHMARK_TEMPLATE(Layout_CircleSortKey, SortKeyType::Camera)->Apply(layoutArguments);
BENCHMARK_TEMPLATE(Layout_CircleSortKey, SortKeyType::DataDriven)->Apply(layoutArguments);
BENCHMARK_TEMPLATE(Layout_SymbolSortKey, SortKeyType::Undefined)->Apply(layoutArguments);
BENCHMARK_TEMPLATE(Layout_SymbolSortKey, SortKeyType::Constant)->Apply(layoutArguments);
BENCHMARK_TEMPLATE(Layout_SymbolSortKey, SortKeyType::Camera)->Apply(layoutArguments);
BENCHMARK_TEMPLATE(Layout_SymbolSortKey, SortKeyType::DataDriven)->Apply(layoutArguments);

} // namespace
