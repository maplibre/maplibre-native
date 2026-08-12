#include <benchmark/benchmark.h>

#include <mbgl/tile/vector_mlt_tile_data.hpp>
#include <mbgl/util/io.hpp>

#include <memory>
#include <string>

namespace {

std::shared_ptr<const std::string> fixtureData(bool useFastPFOR) {
    static const auto data = std::make_shared<const std::string>(
        mbgl::util::read_file("test/fixtures/map/issue12432/0-0-0.mlt"));
    static const auto fastPFORData = std::make_shared<const std::string>(
        mbgl::util::read_file("test/fixtures/map/issue12432/0-0-0-fastpfor.mlt"));

    return useFastPFOR ? fastPFORData : data;
}

std::size_t visitTile(const std::shared_ptr<const std::string>& data, bool fastPFOREnabled) {
    mbgl::VectorMLTTileData tile(data, fastPFOREnabled);

    std::size_t result = 0;
    for (const auto& name : tile.layerNames()) {
        auto layer = tile.getLayer(name);
        if (!layer) {
            continue;
        }

        const auto featureCount = layer->featureCount();
        result += featureCount;
        for (std::size_t i = 0; i < featureCount; ++i) {
            auto feature = layer->getFeature(i);
            result += static_cast<std::size_t>(feature->getType());
            result += feature->getProperties().size();

            const auto& geometries = feature->getGeometries();
            result += geometries.size();
            result += geometries.getTriangles().size();
            for (const auto& geometry : geometries) {
                result += geometry.size();
            }
        }
    }

    return result;
}

void Parse_MLTVectorTile(benchmark::State& state) {
    const auto data = fixtureData(state.range(0) != 0);

    for (auto _ : state) {
        mbgl::VectorMLTTileData tile(data, true);
        auto layerNames = tile.layerNames();
        auto layerNameCount = layerNames.size();
        benchmark::DoNotOptimize(layerNameCount);
    }

    state.SetBytesProcessed(static_cast<int64_t>(state.iterations() * data->size()));
}

void ParseAndAccess_MLTVectorTile(benchmark::State& state) {
    const auto data = fixtureData(state.range(0) != 0);

    for (auto _ : state) {
        auto result = visitTile(data, true);
        benchmark::DoNotOptimize(result);
    }

    state.SetBytesProcessed(static_cast<int64_t>(state.iterations() * data->size()));
}

} // namespace

BENCHMARK(Parse_MLTVectorTile)->Arg(0)->Arg(1)->Unit(benchmark::kMillisecond);
BENCHMARK(ParseAndAccess_MLTVectorTile)->Arg(0)->Arg(1)->Unit(benchmark::kMillisecond);
