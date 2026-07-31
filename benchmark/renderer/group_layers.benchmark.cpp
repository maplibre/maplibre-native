#include <benchmark/benchmark.h>

// Core function to benchmark
#include <mbgl/renderer/group_by_layout.hpp>

// For mock layer properties
#include <mbgl/style/layer_properties.hpp>
#include <mbgl/style/layers/fill_layer_impl.hpp>
#include <mbgl/style/layers/fill_layer_properties.hpp>
#include <mbgl/style/layers/line_layer_impl.hpp>
#include <mbgl/style/layers/line_layer_properties.hpp>
#include <mbgl/style/layers/symbol_layer_impl.hpp>
#include <mbgl/style/layers/symbol_layer_properties.hpp>
#include <mbgl/style/layers/circle_layer_impl.hpp>
#include <mbgl/style/layers/circle_layer_properties.hpp>
#include <mbgl/util/immutable.hpp>

// For real style loading
#include <mbgl/style/style_impl.hpp>
#include <mbgl/storage/file_source.hpp>
#include <mbgl/util/io.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/layermanager/layer_manager.hpp>
#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/util/thread_pool.hpp>

using namespace mbgl;
using namespace mbgl::style;

// Minimal FileSource stub for benchmark (doesn't actually fetch resources)
class StubFileSource : public FileSource {
public:
    std::unique_ptr<AsyncRequest> request(const Resource&, Callback) override { return nullptr; }
    bool canRequest(const Resource&) const override { return false; }
    void setResourceOptions(ResourceOptions) override {}
    ResourceOptions getResourceOptions() override { return ResourceOptions(); }
    void setClientOptions(ClientOptions) override {}
    ClientOptions getClientOptions() override { return ClientOptions(); }
};

/**
 * Helper function to create mock layer properties for benchmarking.
 * Creates a diverse set of layers with varied properties to simulate
 * realistic grouping scenarios.
 */
static std::vector<Immutable<LayerProperties>> createMockLayerProperties(size_t count) {
    std::vector<Immutable<LayerProperties>> layers;
    layers.reserve(count);

    for (size_t i = 0; i < count; ++i) {
        std::string layerId = "layer-" + std::to_string(i);
        // Vary source IDs to create natural grouping (10 different sources)
        std::string sourceId = "source-" + std::to_string(i % 10);

        // Rotate through different layer types
        if (i % 4 == 0) {
            // Fill layers
            auto impl = makeMutable<FillLayer::Impl>(layerId, sourceId);
            impl->minZoom = (i % 2 == 0) ? 0.0f : 5.0f;
            impl->maxZoom = (i % 3 == 0) ? 20.0f : 22.0f;
            layers.push_back(makeMutable<FillLayerProperties>(std::move(impl)));
        } else if (i % 4 == 1) {
            // Line layers
            auto impl = makeMutable<LineLayer::Impl>(layerId, sourceId);
            impl->minZoom = (i % 2 == 0) ? 0.0f : 5.0f;
            impl->maxZoom = (i % 3 == 0) ? 20.0f : 22.0f;
            layers.push_back(makeMutable<LineLayerProperties>(std::move(impl)));
        } else if (i % 4 == 2) {
            // Symbol layers
            auto impl = makeMutable<SymbolLayer::Impl>(layerId, sourceId);
            impl->minZoom = (i % 2 == 0) ? 0.0f : 5.0f;
            impl->maxZoom = (i % 3 == 0) ? 20.0f : 22.0f;
            layers.push_back(makeMutable<SymbolLayerProperties>(std::move(impl)));
        } else {
            // Circle layers
            auto impl = makeMutable<CircleLayer::Impl>(layerId, sourceId);
            impl->minZoom = (i % 2 == 0) ? 0.0f : 5.0f;
            impl->maxZoom = (i % 3 == 0) ? 20.0f : 22.0f;
            layers.push_back(makeMutable<CircleLayerProperties>(std::move(impl)));
        }
    }

    return layers;
}

/**
 * Helper function to load a real style and extract LayerProperties.
 * This uses the actual style parsing infrastructure to create realistic
 * benchmark data from a production style JSON.
 */
static std::vector<Immutable<LayerProperties>> loadStyleLayerProperties(const std::string& stylePath) {
    // Load JSON
    std::string styleJSON = util::read_file(stylePath);

    // Create style infrastructure (minimal setup for parsing)
    util::RunLoop loop;
    auto fileSource = std::make_shared<StubFileSource>();
    auto threadPool = std::make_shared<ThreadPool>();
    TaggedScheduler scheduler{threadPool, util::SimpleIdentity{}};

    Style::Impl style{fileSource, 1.0f, scheduler};

    // Parse style
    style.loadJSON(styleJSON);

    // Get layer implementations from parsed style
    auto layerImpls = style.getLayerImpls();

    // Create LayerProperties from each layer implementation
    std::vector<Immutable<LayerProperties>> layerProperties;
    layerProperties.reserve(layerImpls->size());

    for (const auto& impl : *layerImpls) {
        // Use LayerManager to create the appropriate RenderLayer type
        // which will initialize LayerProperties from the impl
        auto renderLayer = LayerManager::get()->createRenderLayer(impl);
        if (renderLayer) {
            layerProperties.push_back(renderLayer->evaluatedProperties);
        }
    }

    return layerProperties;
}

//
// Mock-based benchmarks: Test with controlled synthetic data
//

static void GroupLayers_Mock_SmallDataset(benchmark::State& state) {
    auto layers = createMockLayerProperties(10);

    while (state.KeepRunning()) {
        auto result = groupLayers(layers);
        benchmark::DoNotOptimize(result);
    }

    state.SetLabel("10 layers");
}

static void GroupLayers_Mock_MediumDataset(benchmark::State& state) {
    auto layers = createMockLayerProperties(50);

    while (state.KeepRunning()) {
        auto result = groupLayers(layers);
        benchmark::DoNotOptimize(result);
    }

    state.SetLabel("50 layers");
}

static void GroupLayers_Mock_LargeDataset(benchmark::State& state) {
    auto layers = createMockLayerProperties(150);

    while (state.KeepRunning()) {
        auto result = groupLayers(layers);
        benchmark::DoNotOptimize(result);
    }

    state.SetLabel("150 layers");
}

//
// Real style benchmark: Test with production data
//

class RealStyleBenchmark : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State&) override {
        // Load style once during setup to avoid measuring parsing time
        layers = loadStyleLayerProperties("benchmark/fixtures/renderer/liberty.json");
    }

    std::vector<Immutable<LayerProperties>> layers;
};

BENCHMARK_F(RealStyleBenchmark, GroupLayers_RealStyle_Liberty)(benchmark::State& state) {
    while (state.KeepRunning()) {
        auto result = groupLayers(layers);
        benchmark::DoNotOptimize(result);
    }

    state.SetLabel(std::to_string(layers.size()) + " layers (Liberty style)");
}

//
// Register benchmarks
//

BENCHMARK(GroupLayers_Mock_SmallDataset);
BENCHMARK(GroupLayers_Mock_MediumDataset);
BENCHMARK(GroupLayers_Mock_LargeDataset);
