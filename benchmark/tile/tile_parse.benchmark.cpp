// Tile parse benchmarks: the whole GeometryTileWorker::parse() (layer grouping, filtering,
// bucket building and symbol layout preparation) on real OpenMapTiles tiles with real styles,
// plus groupLayers() alone with many layers on one source layer.

#include <benchmark/benchmark.h>

#include <mln/actor/scheduler.hpp>
#include <mln/layermanager/layer_manager.hpp>
#include <mln/renderer/group_by_layout.hpp>
#include <mln/renderer/property_evaluation_parameters.hpp>
#include <mln/renderer/render_layer.hpp>
#include <mln/renderer/transition_parameters.hpp>
#include <mln/storage/file_source.hpp>
#include <mln/style/conversion/filter.hpp>
#include <mln/style/conversion/json.hpp>
#include <mln/style/filter.hpp>
#include <mln/style/layers/line_layer.hpp>
#include <mln/style/layers/line_layer_impl.hpp>
#include <mln/style/layers/line_layer_properties.hpp>
#include <mln/style/style_impl.hpp>
#include <mln/tile/geometry_tile_worker.hpp>
#include <mln/tile/vector_mvt_tile_data.hpp>
#include <mln/util/io.hpp>
#include <mln/util/run_loop.hpp>
#include <mln/util/thread_pool.hpp>

#include <array>
#include <atomic>
#include <memory>
#include <new>
#include <set>
#include <string>
#include <vector>

using namespace mln;
using namespace mln::style;

namespace {

class StubFileSource : public FileSource {
public:
    std::unique_ptr<AsyncRequest> request(const Resource&, Callback) override { return nullptr; }
    bool canRequest(const Resource&) const override { return false; }
    void setResourceOptions(ResourceOptions) override {}
    ResourceOptions getResourceOptions() override { return {}; }
    void setClientOptions(ClientOptions) override {}
    ClientOptions getClientOptions() override { return {}; }
};

// Runs everything inline and drops render-thread jobs, so the worker's results are freed when
// it is destroyed.
class InlineScheduler : public Scheduler {
public:
    void schedule(std::function<void()>&& fn) override { fn(); }
    void schedule(const util::SimpleIdentity, std::function<void()>&& fn) override { fn(); }
    void waitForEmpty(const util::SimpleIdentity) override {}
    mapbox::base::WeakPtr<Scheduler> makeWeakPtr() override { return weakFactory.makeWeakPtr(); }

private:
    mapbox::base::WeakPtrFactory<Scheduler> weakFactory{this};
};

// Parses a style and returns the evaluated properties of the visible layers of one source,
// which is what the render orchestrator hands each tile worker.
std::vector<Immutable<LayerProperties>> loadSourceLayers(const std::string& stylePath,
                                                         const std::string& sourceID,
                                                         float zoom) {
    util::RunLoop loop;
    auto threadPool = std::make_shared<ThreadPool>();
    Style::Impl style{std::make_shared<StubFileSource>(), 1.0f, TaggedScheduler{threadPool, util::SimpleIdentity{}}};
    style.loadJSON(util::read_file(stylePath));

    std::vector<Immutable<LayerProperties>> result;
    for (const auto& impl : *style.getLayerImpls()) {
        if (impl->source != sourceID || impl->visibility != VisibilityType::Visible) continue;
        auto renderLayer = LayerManager::get()->createRenderLayer(impl);
        if (!renderLayer) continue;
        renderLayer->transition(TransitionParameters{});
        renderLayer->evaluate(PropertyEvaluationParameters(zoom));
        result.push_back(renderLayer->evaluatedProperties);
    }
    return result;
}

struct TileFixture {
    OverscaledTileID id;
    std::shared_ptr<const std::string> data;
};

std::vector<TileFixture> loadTiles(const std::string& dir,
                                   const std::vector<std::array<uint32_t, 3>>& ids,
                                   const std::string& separator) {
    std::vector<TileFixture> tiles;
    for (const auto& [z, x, y] : ids) {
        const std::string path = dir + "/" + std::to_string(z) + separator + std::to_string(x) + separator +
                                 std::to_string(y) + ".mvt";
        tiles.push_back(
            {OverscaledTileID(static_cast<uint8_t>(z), x, y), std::make_shared<std::string>(util::read_file(path))});
    }
    return tiles;
}

class TileParser {
public:
    explicit TileParser(std::string sourceID_)
        : scheduler(std::make_shared<InlineScheduler>(), util::SimpleIdentity{}),
          sourceID(std::move(sourceID_)) {}

    // Runs one complete parse of the tile and returns the worker holding its results.
    std::unique_ptr<GeometryTileWorker> parse(const TileFixture& tile,
                                              const std::vector<Immutable<LayerProperties>>& layers) {
        // The worker messages itself to leave its "coalescing" state; a synchronous reference to
        // itself delivers that at once, as the actor mailbox would between messages.
        auto* storage = static_cast<GeometryTileWorker*>(::operator new(sizeof(GeometryTileWorker)));
        std::unique_ptr<GeometryTileWorker> worker{
            new (storage) GeometryTileWorker(OptionalActorRef<GeometryTileWorker>{*storage},
                                             OptionalActorRef<GeometryTile>{},
                                             scheduler,
                                             tile.id,
                                             sourceID,
                                             obsolete,
                                             MapMode::Continuous,
                                             1.0f,
                                             false,
                                             nullptr,
                                             fontFaces,
                                             false,
                                             nullptr)};
        worker->setLayers(layers, globalState, availableImages, 0);
        // The worker is idle, so this parses at once.
        worker->setData(std::make_unique<VectorMVTTileData>(tile.data), availableImages, 0);
        return worker;
    }

private:
    TaggedScheduler scheduler;
    std::string sourceID;
    std::atomic<bool> obsolete{false};
    std::shared_ptr<FontFaces> fontFaces = std::make_shared<FontFaces>();
    std::shared_ptr<const GlobalStateMap> globalState = std::make_shared<GlobalStateMap>();
    Immutable<std::set<std::string>> availableImages = makeMutable<std::set<std::string>>();
};

const std::string omtDir = "vendor/maplibre-tile-spec/test/fixtures/omt";

const std::vector<std::array<uint32_t, 3>> omtZ14 = {{14, 8296, 10748},
                                                     {14, 8296, 10749},
                                                     {14, 8297, 10748},
                                                     {14, 8297, 10749},
                                                     {14, 8298, 10748},
                                                     {14, 8298, 10749},
                                                     {14, 8299, 10748},
                                                     {14, 8299, 10749},
                                                     {14, 8300, 10748},
                                                     {14, 8300, 10749}};
const std::vector<std::array<uint32_t, 3>> omtZ12 = {{12, 2130, 2733},
                                                     {12, 2130, 2734},
                                                     {12, 2131, 2733},
                                                     {12, 2131, 2734},
                                                     {12, 2132, 2733},
                                                     {12, 2132, 2734},
                                                     {12, 2133, 2733},
                                                     {12, 2133, 2734},
                                                     {12, 2134, 2733},
                                                     {12, 2134, 2734}};

const std::string sfDir = "vendor/maplibre-tile-spec/test/mvt-fixtures/real-world/sanfrancisco";
const std::vector<std::array<uint32_t, 3>> sfZ15 = {{15, 5237, 12665},
                                                    {15, 5237, 12666},
                                                    {15, 5237, 12667},
                                                    {15, 5238, 12665},
                                                    {15, 5238, 12666},
                                                    {15, 5238, 12667},
                                                    {15, 5239, 12665},
                                                    {15, 5239, 12666},
                                                    {15, 5239, 12667}};

void runTileParse(benchmark::State& state,
                  const std::string& stylePath,
                  const std::vector<std::array<uint32_t, 3>>& ids,
                  float zoom,
                  const std::string& sourceID = "openmaptiles",
                  const std::string& dir = omtDir,
                  const std::string& separator = "_") {
    const auto layers = loadSourceLayers(stylePath, sourceID, zoom);
    const auto tiles = loadTiles(dir, ids, separator);
    TileParser parser{sourceID};

    for (auto _ : state) {
        for (const auto& tile : tiles) {
            auto worker = parser.parse(tile, layers);
            benchmark::DoNotOptimize(worker);
            state.PauseTiming();
            worker.reset();
            state.ResumeTiming();
        }
    }

    state.counters["layers"] = static_cast<double>(layers.size());
    state.counters["groups"] = static_cast<double>(groupLayers(layers).size());
    state.counters["tiles"] = static_cast<double>(tiles.size());
    state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(tiles.size()));
}

} // namespace

static void TileParse_Liberty_z14(benchmark::State& state) {
    runTileParse(state, "benchmark/fixtures/renderer/liberty.json", omtZ14, 14.0f);
}
static void TileParse_Liberty_z12(benchmark::State& state) {
    runTileParse(state, "benchmark/fixtures/renderer/liberty.json", omtZ12, 12.0f);
}
static void TileParse_Streets_z14(benchmark::State& state) {
    runTileParse(state, "platform/ios/benchmark/assets/styles/streets.json", omtZ14, 14.0f);
}
static void TileParse_Streets_z12(benchmark::State& state) {
    runTileParse(state, "platform/ios/benchmark/assets/styles/streets.json", omtZ12, 12.0f);
}

static void TileParse_PairedLayers_z15(benchmark::State& state) {
    runTileParse(state, "benchmark/fixtures/api/style_paired_layers.json", sfZ15, 15.0f, "mapbox", sfDir, "-");
}

BENCHMARK(TileParse_Liberty_z14)->Unit(benchmark::kMicrosecond);
BENCHMARK(TileParse_Liberty_z12)->Unit(benchmark::kMicrosecond);
BENCHMARK(TileParse_Streets_z14)->Unit(benchmark::kMicrosecond);
BENCHMARK(TileParse_Streets_z12)->Unit(benchmark::kMicrosecond);
BENCHMARK(TileParse_PairedLayers_z15)->Unit(benchmark::kMicrosecond);

// --- groupLayers() alone against the number of layers on one source layer ---------------------

namespace {

std::vector<std::unique_ptr<LineLayer>> ownedLayers;

// `count` line layers on one source layer. Every `share` consecutive layers use the same filter
// text (parsed separately, as the style parser does); the filters differ only in the last
// literal, which is the slowest case for a deep comparison.
std::vector<Immutable<LayerProperties>> makeSameSourceLayer(std::size_t count, std::size_t share) {
    std::vector<Immutable<LayerProperties>> props;
    for (std::size_t i = 0; i < count; ++i) {
        auto layer = std::make_unique<LineLayer>("layer-" + std::to_string(i), "source");
        layer->setSourceLayer("transportation");
        const std::string json = R"(["all", ["==", ["get", "class"], "minor"], ["!=", ["get", "brunnel"], "tunnel"], )"
                                 R"(["==", ["get", "rank"], )" +
                                 std::to_string(i / share) + "]]";
        conversion::Error error;
        auto filter = conversion::convertJSON<Filter>(json, error);
        layer->setFilter(*filter);
        layer->setLineWidth(PropertyValue<float>(1.0f + static_cast<float>(i)));
        props.push_back(makeMutable<LineLayerProperties>(staticImmutableCast<LineLayer::Impl>(layer->baseImpl)));
        ownedLayers.push_back(std::move(layer));
    }
    return props;
}

void runGroupSameSourceLayer(benchmark::State& state, std::size_t share) {
    const auto layers = makeSameSourceLayer(static_cast<std::size_t>(state.range(0)), share);
    std::size_t groups = 0;
    for (auto _ : state) {
        auto result = groupLayers(layers);
        groups = result.size();
        benchmark::DoNotOptimize(result);
    }
    state.counters["groups"] = static_cast<double>(groups);
}

} // namespace

static void GroupLayers_SameSourceLayer_AllDistinct(benchmark::State& state) {
    runGroupSameSourceLayer(state, 1);
}
static void GroupLayers_SameSourceLayer_Pairs(benchmark::State& state) {
    runGroupSameSourceLayer(state, 2);
}

BENCHMARK(GroupLayers_SameSourceLayer_AllDistinct)->Arg(10)->Arg(50)->Arg(100)->Arg(250)->Arg(500);
BENCHMARK(GroupLayers_SameSourceLayer_Pairs)->Arg(10)->Arg(50)->Arg(100)->Arg(250)->Arg(500);
