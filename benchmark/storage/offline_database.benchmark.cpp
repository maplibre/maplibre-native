#include <benchmark/benchmark.h>

#include <mln/storage/offline_database.hpp>
#include <mln/storage/resource.hpp>
#include <mln/storage/response.hpp>
#include <mln/storage/sqlite3.hpp>
#include <mln/util/string.hpp>
#include <mln/util/logging.hpp>

#include <random>

class OfflineDatabase : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State&) override {
        using namespace std::chrono_literals;

        db.setOfflineMapboxTileCountLimit(100000);

        response.data = std::make_shared<std::string>(50 * 1024, 0);
        response.mustRevalidate = false;
        response.expires = mln::util::now() + 1h;

        resetAmbientTiles();
        resetRegion();
    }

    void resetAmbientTiles() {
        using namespace mln;

        db.clearAmbientCache();

        for (unsigned i = 0; i < tileCount; ++i) {
            const Resource ambient = Resource::tile(
                "mapbox://tile_ambient" + util::toString(i), 1, 0, 0, 0, Tileset::Scheme::XYZ);
            db.put(ambient, response);
        }
    }

    void resetRegion() {
        using namespace mln;

        auto regions = db.listRegions().value();
        if (!regions.empty()) {
            db.deleteRegion(std::move(regions[0]));
        }

        OfflineTilePyramidRegionDefinition definition{
            "mapbox://style", LatLngBounds::hull({1, 2}, {3, 4}), 5, 6, 2.0, true};
        OfflineRegionMetadata metadata{{1, 2, 3}};

        auto region = db.createRegion(definition, metadata);
        regionID = region->getID();

        for (unsigned i = 0; i < tileCount; ++i) {
            const Resource offline = Resource::tile(
                "mapbox://tile_offline_region" + util::toString(i), 1.0, 0, 0, 0, Tileset::Scheme::XYZ);
            db.putRegionResource(regionID, offline, response);
        }
    }

    mln::Response response;
    mln::OfflineDatabase db{":memory:", mln::TileServerOptions::DefaultConfiguration()};

    const unsigned tileCount = 100;
    int64_t regionID = 0;
};

BENCHMARK_F(OfflineDatabase, InvalidateRegion)(benchmark::State& state) {
    while (state.KeepRunning()) {
        db.invalidateRegion(regionID);
    }
}

BENCHMARK_F(OfflineDatabase, DeleteRegion)(benchmark::State& state) {
    while (state.KeepRunning()) {
        resetRegion();
    }
}

BENCHMARK_F(OfflineDatabase, InsertTileRegion)(benchmark::State& state) {
    using namespace mln;

    while (state.KeepRunning()) {
        const Resource offline = Resource::tile(
            "mapbox://InsertTileRegion" + util::toString(state.iterations()), 1, 0, 0, 0, Tileset::Scheme::XYZ);
        db.putRegionResource(regionID, offline, response);
    }
}
BENCHMARK_F(OfflineDatabase, InvalidateAmbientCache)(benchmark::State& state) {
    while (state.KeepRunning()) {
        db.invalidateAmbientCache();
    }
}

BENCHMARK_F(OfflineDatabase, ClearAmbientCache)(benchmark::State& state) {
    while (state.KeepRunning()) {
        resetAmbientTiles();
    }
}

BENCHMARK_F(OfflineDatabase, InsertTileCache)(benchmark::State& state) {
    using namespace mln;

    while (state.KeepRunning()) {
        const Resource ambient = Resource::tile(
            "mapbox://InsertTileCache" + util::toString(state.iterations()), 1, 0, 0, 0, Tileset::Scheme::XYZ);
        db.put(ambient, response);
    }
}

BENCHMARK_F(OfflineDatabase, InsertBigTileCache)(benchmark::State& state) {
    using namespace mln;

    Response big;
    big.data = std::make_shared<std::string>(util::DEFAULT_MAX_CACHE_SIZE / 100, 0);

    while (state.KeepRunning()) {
        const Resource ambient = Resource::tile(
            "mapbox://InsertTileCache" + util::toString(state.iterations()), 1, 0, 0, 0, Tileset::Scheme::XYZ);
        db.put(ambient, big);
    }
}

BENCHMARK_F(OfflineDatabase, GetTile)(benchmark::State& state) {
    using namespace mln;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, tileCount - 1);

    while (state.KeepRunning()) {
        auto res = db.get(
            Resource::tile("mapbox://tile_ambient" + util::toString(dis(gen)), 1, 0, 0, 0, Tileset::Scheme::XYZ));
        assert(res != std::nullopt);
    }
}

BENCHMARK_F(OfflineDatabase, AddTilesToFullDatabase)(benchmark::State& state) {
    using namespace mln;

    Log::setObserver(std::make_unique<Log::NullObserver>());
    db.setMaximumAmbientCacheSize(50 * 1024 * 5);

    while (state.KeepRunning()) {
        const Resource ambient = Resource::tile(
            "mapbox://AddTilesToFullDatabase" + util::toString(state.iterations()), 1, 0, 0, 0, Tileset::Scheme::XYZ);
        db.put(ambient, response);
    }

    Log::removeObserver();
}

BENCHMARK_F(OfflineDatabase, AddTilesToDisabledDatabase)
(benchmark::State& state) {
    using namespace mln;

    auto regions = db.listRegions().value();
    if (!regions.empty()) {
        db.deleteRegion(std::move(regions[0]));
    }

    // Disables the ambient cache.
    db.setMaximumAmbientCacheSize(0);

    while (state.KeepRunning()) {
        const Resource ambient = Resource::tile(
            "mapbox://AddTilesToFullDatabase" + util::toString(state.iterations()), 1, 0, 0, 0, Tileset::Scheme::XYZ);
        db.put(ambient, response);
    }
}

BENCHMARK_F(OfflineDatabase, GetTileFromDisabledDatabase)
(benchmark::State& state) {
    using namespace mln;

    auto regions = db.listRegions().value();
    if (!regions.empty()) {
        db.deleteRegion(std::move(regions[0]));
    }

    // Disables the ambient cache.
    db.setMaximumAmbientCacheSize(0);

    while (state.KeepRunning()) {
        auto res = db.get(Resource::tile("mapbox://tile_ambient", 1, 0, 0, 0, Tileset::Scheme::XYZ));
        assert(res == std::nullopt);
    }
}

BENCHMARK_F(OfflineDatabase, ResizeDatabase)(benchmark::State& state) {
    uint64_t size = 25 * 1024 * 1024;

    while (state.KeepRunning()) {
        db.setMaximumAmbientCacheSize(size);

        size -= response.data->size();

        if (size < response.data->size()) {
            size = 25 * 1024 * 1024;
        }
    }
}
