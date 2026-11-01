#include <mln/test/map_adapter.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/fake_file_source.hpp>
#include <mln/test/stub_map_observer.hpp>
#include <mln/test/fixture_log_observer.hpp>
#include <mln/test/sqlite3_test_fs.hpp>
#include <mln/test/util.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/storage/offline.hpp>
#include <mln/storage/offline_database.hpp>
#include <mln/storage/offline_download.hpp>
#include <mln/storage/http_file_source.hpp>
#include <mln/text/glyph_range.hpp>
#include <mln/util/run_loop.hpp>
#include <mln/util/io.hpp>
#include <mln/util/compression.hpp>
#include <mln/util/string.hpp>

#include <mln/storage/sqlite3.hpp>
#include <gtest/gtest.h>

using namespace mln;
using namespace std::literals::string_literals;
using mapbox::sqlite::ResultCode;

#ifndef __QT__ // Qt doesn't expose the ability to register virtual file system handlers.
static constexpr const char* filename = "test/fixtures/offline_download/offline.db";
static constexpr const char* filename_test_fs = "file:test/fixtures/offline_download/offline.db?vfs=test_fs";

static void deleteDatabaseFiles() {
    // Delete leftover journaling files as well.
    util::deleteFile(filename);
    util::deleteFile(filename + "-wal"s);
    util::deleteFile(filename + "-journal"s);
}

static FixtureLog::Message warning(ResultCode code, const char* message) {
    return {EventSeverity::Warning, Event::Database, static_cast<int64_t>(code), message};
}
#endif

class MockObserver : public OfflineRegionObserver {
public:
    void statusChanged(OfflineRegionStatus status) override {
        if (statusChangedFn) statusChangedFn(status);
    }

    void responseError(Response::Error error) override {
        if (responseErrorFn) responseErrorFn(error);
    }

    void mapboxTileCountLimitExceeded(uint64_t limit) override {
        if (mapboxTileCountLimitExceededFn) mapboxTileCountLimitExceededFn(limit);
    }

    std::function<void(OfflineRegionStatus)> statusChangedFn;
    std::function<void(Response::Error)> responseErrorFn;
    std::function<void(uint64_t)> mapboxTileCountLimitExceededFn;
};

class OfflineTest {
public:
    OfflineTest(const std::string& path = ":memory:")
        : db(path, TileServerOptions::MapTilerConfiguration()) {}

    util::RunLoop loop;
    StubFileSource fileSource;
    OfflineDatabase db;
    std::size_t size = 0;

    auto createRegion() {
        OfflineTilePyramidRegionDefinition definition{"", LatLngBounds::hull({1, 2}, {3, 4}), 5, 6, 1.0, true};
        OfflineRegionMetadata metadata;
        return db.createRegion(definition, metadata);
    }

    auto invalidateRegion(int64_t region) { return db.invalidateRegion(region); }

    Response response(const std::string& path) {
        Response result;
        result.data = std::make_shared<std::string>(util::read_file("test/fixtures/offline_download/"s + path));
        size_t uncompressed = result.data->size();
        size_t compressed = util::compress(*result.data).size();
        size += std::min(uncompressed, compressed);
        return result;
    }
};

TEST(OfflineDownload, NoSubresources) {
    OfflineTest test;
    auto region = test.createRegion();
    ASSERT_TRUE(region);
    OfflineDownload download(region->getID(),
                             OfflineTilePyramidRegionDefinition(
                                 "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, false),
                             test.db,
                             test.fileSource);

    test.fileSource.styleResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/style.json", resource.url);
        return test.response("empty.style.json");
    };

    auto observer = std::make_unique<MockObserver>();

    bool expectsInactiveStatus = false;
    observer->statusChangedFn = [&](OfflineRegionStatus status) {
        if (status.complete()) {
            if (!expectsInactiveStatus) {
                expectsInactiveStatus = true;
                EXPECT_EQ(OfflineRegionDownloadState::Active, status.downloadState);
                EXPECT_EQ(1u, status.completedResourceCount);
                EXPECT_EQ(test.size, status.completedResourceSize);
                EXPECT_TRUE(status.requiredResourceCountIsPrecise);
            } else {
                EXPECT_EQ(OfflineRegionDownloadState::Inactive, status.downloadState);
                EXPECT_EQ(1u, status.completedResourceCount);
                EXPECT_EQ(test.size, status.completedResourceSize);
                EXPECT_TRUE(status.requiredResourceCountIsPrecise);
                test.loop.stop();
            }
        }
    };

    download.setObserver(std::move(observer));
    download.setState(OfflineRegionDownloadState::Active);

    test.loop.run();
}

TEST(OfflineDownload, InlineSource) {
    OfflineTest test;
    auto region = test.createRegion();
    ASSERT_TRUE(region);
    OfflineDownload download(region->getID(),
                             OfflineTilePyramidRegionDefinition(
                                 "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, true),
                             test.db,
                             test.fileSource);

    test.fileSource.styleResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/style.json", resource.url);
        return test.response("inline_source.style.json");
    };

    test.fileSource.tileResponse = [&](const Resource& resource) {
        const Resource::TileData& tile = *resource.tileData;
        EXPECT_EQ("http://127.0.0.1:3000/{z}-{x}-{y}.vector.pbf", tile.urlTemplate);
        EXPECT_EQ(1, tile.pixelRatio);
        EXPECT_EQ(0, tile.x);
        EXPECT_EQ(0, tile.y);
        EXPECT_EQ(0, tile.z);
        return test.response("0-0-0.vector.pbf");
    };

    auto observer = std::make_unique<MockObserver>();

    observer->statusChangedFn = [&](OfflineRegionStatus status) {
        if (status.complete()) {
            EXPECT_EQ(1u, status.completedTileCount);
            EXPECT_EQ(2u, status.completedResourceCount);
            EXPECT_EQ(test.size, status.completedResourceSize);
            EXPECT_TRUE(status.requiredResourceCountIsPrecise);
            test.loop.stop();
        }
    };

    download.setObserver(std::move(observer));
    download.setState(OfflineRegionDownloadState::Active);

    test.loop.run();
}

TEST(OfflineDownload, GeoJSONSource) {
    OfflineTest test;
    auto region = test.createRegion();
    ASSERT_TRUE(region);
    OfflineDownload download(region->getID(),
                             OfflineTilePyramidRegionDefinition(
                                 "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, false),
                             test.db,
                             test.fileSource);

    test.fileSource.styleResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/style.json", resource.url);
        return test.response("geojson_source.style.json");
    };

    test.fileSource.sourceResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/geojson.json", resource.url);
        return test.response("geojson.json");
    };

    auto observer = std::make_unique<MockObserver>();

    observer->statusChangedFn = [&](OfflineRegionStatus status) {
        if (status.complete()) {
            EXPECT_EQ(2u, status.completedResourceCount);
            EXPECT_EQ(test.size, status.completedResourceSize);
            EXPECT_TRUE(status.requiredResourceCountIsPrecise);
            test.loop.stop();
        }
    };

    download.setObserver(std::move(observer));
    download.setState(OfflineRegionDownloadState::Active);

    test.loop.run();
}

TEST(OfflineDownload, Activate) {
    OfflineTest test;
    auto region = test.createRegion();
    ASSERT_TRUE(region);
    OfflineDownload download(region->getID(),
                             OfflineTilePyramidRegionDefinition(
                                 "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, true),
                             test.db,
                             test.fileSource);

    test.fileSource.styleResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/style.json", resource.url);
        return test.response("style.json");
    };

    test.fileSource.spriteImageResponse = [&](const Resource& resource) {
        EXPECT_TRUE(resource.url == "http://127.0.0.1:3000/sprite.png" ||
                    resource.url == "http://127.0.0.1:3000/sprite@2x.png");
        return test.response("sprite.png");
    };

    test.fileSource.imageResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/radar.gif", resource.url);
        return test.response("radar.gif");
    };

    test.fileSource.spriteJSONResponse = [&](const Resource& resource) {
        EXPECT_TRUE(resource.url == "http://127.0.0.1:3000/sprite.json" ||
                    resource.url == "http://127.0.0.1:3000/sprite@2x.json");
        return test.response("sprite.json");
    };

    test.fileSource.glyphsResponse = [&](const Resource&) {
        return test.response("glyph.pbf");
    };

    test.fileSource.sourceResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/streets.json", resource.url);
        return test.response("streets.json");
    };

    test.fileSource.tileResponse = [&](const Resource& resource) {
        const Resource::TileData& tile = *resource.tileData;
        EXPECT_EQ("http://127.0.0.1:3000/{z}-{x}-{y}.vector.pbf", tile.urlTemplate);
        EXPECT_EQ(1, tile.pixelRatio);
        EXPECT_EQ(0, tile.x);
        EXPECT_EQ(0, tile.y);
        EXPECT_EQ(0, tile.z);
        return test.response("0-0-0.vector.pbf");
    };

    auto observer = std::make_unique<MockObserver>();

    observer->statusChangedFn = [&](OfflineRegionStatus status) {
        if (status.complete()) {
            EXPECT_EQ(status.completedTileCount, status.requiredTileCount);
            EXPECT_EQ(264u,
                      status.completedResourceCount); // 256 glyphs, 2 sprite images,
                                                      // 2 sprite jsons, 1 tile, 1
                                                      // style, source, image
            EXPECT_EQ(test.size, status.completedResourceSize);

            download.setState(OfflineRegionDownloadState::Inactive);
            OfflineRegionStatus computedStatus = download.getStatus();
            EXPECT_EQ(OfflineRegionDownloadState::Inactive, computedStatus.downloadState);
            EXPECT_EQ(status.requiredResourceCount, computedStatus.requiredResourceCount);
            EXPECT_EQ(status.requiredTileCount, computedStatus.requiredTileCount);
            EXPECT_EQ(status.completedResourceCount, computedStatus.completedResourceCount);
            EXPECT_EQ(status.completedResourceSize, computedStatus.completedResourceSize);
            EXPECT_EQ(status.completedTileCount, computedStatus.completedTileCount);
            EXPECT_EQ(status.completedTileSize, computedStatus.completedTileSize);
            EXPECT_TRUE(status.requiredResourceCountIsPrecise);

            test.loop.stop();
        }
    };

    download.setObserver(std::move(observer));
    download.setState(OfflineRegionDownloadState::Active);

    test.loop.run();
}

TEST(OfflineDownload, ExcludeIdeographs) {
    OfflineTest test;
    auto region = test.createRegion();
    ASSERT_TRUE(region);
    OfflineDownload download(region->getID(),
                             OfflineTilePyramidRegionDefinition(
                                 "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, false),
                             test.db,
                             test.fileSource);

    test.fileSource.styleResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/style.json", resource.url);
        return test.response("style.json");
    };

    test.fileSource.spriteImageResponse = [&](const Resource& resource) {
        EXPECT_TRUE(resource.url == "http://127.0.0.1:3000/sprite.png" ||
                    resource.url == "http://127.0.0.1:3000/sprite@2x.png");
        return test.response("sprite.png");
    };

    test.fileSource.imageResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/radar.gif", resource.url);
        return test.response("radar.gif");
    };

    test.fileSource.spriteJSONResponse = [&](const Resource& resource) {
        EXPECT_TRUE(resource.url == "http://127.0.0.1:3000/sprite.json" ||
                    resource.url == "http://127.0.0.1:3000/sprite@2x.json");
        return test.response("sprite.json");
    };

    test.fileSource.glyphsResponse = [&](const Resource&) {
        return test.response("glyph.pbf");
    };

    test.fileSource.sourceResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/streets.json", resource.url);
        return test.response("streets.json");
    };

    test.fileSource.tileResponse = [&](const Resource& resource) {
        const Resource::TileData& tile = *resource.tileData;
        EXPECT_EQ("http://127.0.0.1:3000/{z}-{x}-{y}.vector.pbf", tile.urlTemplate);
        EXPECT_EQ(1, tile.pixelRatio);
        EXPECT_EQ(0, tile.x);
        EXPECT_EQ(0, tile.y);
        EXPECT_EQ(0, tile.z);
        return test.response("0-0-0.vector.pbf");
    };

    auto observer = std::make_unique<MockObserver>();

    observer->statusChangedFn = [&](OfflineRegionStatus status) {
        if (status.complete()) {
            EXPECT_EQ(status.completedTileCount, status.requiredTileCount);
            EXPECT_EQ(NON_IDEOGRAPH_GLYPH_RANGES_PER_FONT_STACK + 8u,
                      status.completedResourceCount); // glyph ranges, 2 sprite images,
                                                      // 2 sprite jsons, 1 tile, 1 style,
                                                      // source, image
            EXPECT_EQ(test.size, status.completedResourceSize);

            download.setState(OfflineRegionDownloadState::Inactive);
            OfflineRegionStatus computedStatus = download.getStatus();
            EXPECT_EQ(OfflineRegionDownloadState::Inactive, computedStatus.downloadState);
            EXPECT_EQ(status.requiredResourceCount, computedStatus.requiredResourceCount);
            EXPECT_EQ(status.requiredTileCount, computedStatus.requiredTileCount);
            EXPECT_EQ(status.completedResourceCount, computedStatus.completedResourceCount);
            EXPECT_EQ(status.completedResourceSize, computedStatus.completedResourceSize);
            EXPECT_EQ(status.completedTileCount, computedStatus.completedTileCount);
            EXPECT_EQ(status.completedTileSize, computedStatus.completedTileSize);
            EXPECT_TRUE(status.requiredResourceCountIsPrecise);

            test.loop.stop();
        }
    };

    download.setObserver(std::move(observer));
    download.setState(OfflineRegionDownloadState::Active);

    test.loop.run();
}

TEST(OfflineDownload, DoesNotFloodTheFileSourceWithRequests) {
    OfflineTest test;
    FakeOnlineFileSource fileSource;
    auto region = test.createRegion();
    ASSERT_TRUE(region);
    OfflineDownload download(region->getID(),
                             OfflineTilePyramidRegionDefinition(
                                 "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, true),
                             test.db,
                             fileSource);

    auto observer = std::make_unique<MockObserver>();

    download.setObserver(std::move(observer));
    download.setState(OfflineRegionDownloadState::Active);
    test.loop.runOnce();

    EXPECT_EQ(1u, fileSource.requests.size());

    fileSource.respond(Resource::Kind::Style, test.response("style.json"));
    test.loop.runOnce();

    EXPECT_EQ(*fileSource.getProperty(MAX_CONCURRENT_REQUESTS_KEY).getUint(), fileSource.requests.size());
}

TEST(OfflineDownload, GetStatusNoResources) {
    OfflineTest test;
    auto region = test.createRegion();
    ASSERT_TRUE(region);
    OfflineDownload download(region->getID(),
                             OfflineTilePyramidRegionDefinition(
                                 "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, false),
                             test.db,
                             test.fileSource);
    OfflineRegionStatus status = download.getStatus();

    EXPECT_EQ(OfflineRegionDownloadState::Inactive, status.downloadState);
    EXPECT_EQ(0u, status.completedResourceCount);
    EXPECT_EQ(0u, status.completedResourceSize);
    EXPECT_EQ(1u, status.requiredResourceCount);
    EXPECT_EQ(0u, status.completedTileCount);
    EXPECT_EQ(0u, status.requiredTileCount);
    EXPECT_FALSE(status.requiredResourceCountIsPrecise);
    EXPECT_FALSE(status.complete());
}

TEST(OfflineDownload, GetStatusStyleComplete) {
    OfflineTest test;
    auto region = test.createRegion();
    ASSERT_TRUE(region);
    OfflineDownload download(region->getID(),
                             OfflineTilePyramidRegionDefinition(
                                 "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, true),
                             test.db,
                             test.fileSource);

    test.db.putRegionResource(1, Resource::style("http://127.0.0.1:3000/style.json"), test.response("style.json"));

    OfflineRegionStatus status = download.getStatus();

    EXPECT_EQ(OfflineRegionDownloadState::Inactive, status.downloadState);
    EXPECT_EQ(1u, status.completedResourceCount);
    EXPECT_EQ(test.size, status.completedResourceSize);
    EXPECT_EQ(263u, status.requiredResourceCount);
    EXPECT_EQ(0u, status.completedTileCount);
    EXPECT_EQ(0u, status.requiredTileCount);
    EXPECT_FALSE(status.requiredResourceCountIsPrecise);
    EXPECT_FALSE(status.complete());
}

TEST(OfflineDownload, GetStatusStyleAndSourceComplete) {
    OfflineTest test;
    auto region = test.createRegion();
    ASSERT_TRUE(region);
    OfflineDownload download(region->getID(),
                             OfflineTilePyramidRegionDefinition(
                                 "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, true),
                             test.db,
                             test.fileSource);

    test.db.putRegionResource(1, Resource::style("http://127.0.0.1:3000/style.json"), test.response("style.json"));

    test.db.putRegionResource(1, Resource::source("http://127.0.0.1:3000/streets.json"), test.response("streets.json"));

    OfflineRegionStatus status = download.getStatus();

    EXPECT_EQ(OfflineRegionDownloadState::Inactive, status.downloadState);
    EXPECT_EQ(2u, status.completedResourceCount);
    EXPECT_EQ(test.size, status.completedResourceSize);
    EXPECT_EQ(264u, status.requiredResourceCount);
    EXPECT_EQ(0u, status.completedTileCount);
    EXPECT_EQ(1u, status.requiredTileCount);
    EXPECT_TRUE(status.requiredResourceCountIsPrecise);
    EXPECT_FALSE(status.complete());
}

TEST(OfflineDownload, RequestError) {
    OfflineTest test;
    auto region = test.createRegion();
    ASSERT_TRUE(region);
    OfflineDownload download(region->getID(),
                             OfflineTilePyramidRegionDefinition(
                                 "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, false),
                             test.db,
                             test.fileSource);

    test.fileSource.styleResponse = [&](const Resource&) {
        Response response;
        response.error = std::make_unique<Response::Error>(Response::Error::Reason::Connection, "connection error");
        return response;
    };

    auto observer = std::make_unique<MockObserver>();

    observer->responseErrorFn = [&](Response::Error error) {
        EXPECT_EQ(Response::Error::Reason::Connection, error.reason);
        EXPECT_EQ("connection error", error.message);
        test.loop.stop();
    };

    download.setObserver(std::move(observer));
    download.setState(OfflineRegionDownloadState::Active);

    test.loop.run();
}

TEST(OfflineDownload, RequestErrorsAreRetried) {
    OfflineTest test;
    auto region = test.createRegion();
    ASSERT_TRUE(region);
    OfflineDownload download(region->getID(),
                             OfflineTilePyramidRegionDefinition(
                                 "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, true),
                             test.db,
                             test.fileSource);

    test.fileSource.styleResponse = [&](const Resource&) {
        test.fileSource.styleResponse = [&](const Resource&) {
            return test.response("empty.style.json");
        };

        Response response;
        response.error = std::make_unique<Response::Error>(Response::Error::Reason::Connection, "connection error");
        return response;
    };

    auto observer = std::make_unique<MockObserver>();

    observer->statusChangedFn = [&](OfflineRegionStatus status) {
        if (status.complete()) {
            EXPECT_EQ(1u, status.completedResourceCount);
            test.loop.stop();
        }
    };

    download.setObserver(std::move(observer));
    download.setState(OfflineRegionDownloadState::Active);

    test.loop.run();
}

TEST(OfflineDownload, TileCountLimitExceededNoTileResponse) {
    OfflineTest test;
    auto region = test.createRegion();
    ASSERT_TRUE(region);
    OfflineDownload download(region->getID(),
                             OfflineTilePyramidRegionDefinition(
                                 "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, false),
                             test.db,
                             test.fileSource);

    uint64_t tileLimit = 0;

    test.db.setOfflineMapboxTileCountLimit(tileLimit);

    test.fileSource.styleResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/style.json", resource.url);
        return test.response("mapbox_source.style.json");
    };
    //    test.fileSource.tileResponse = [&] (const Resource& resource) {
    //        EXPECT_EQ("maptiler://0-0-0.vector.pbf", resource.url);
    //        return test.response("0-0-0.vector.pbf");
    //    };

    auto observer = std::make_unique<MockObserver>();
    bool mapboxTileCountLimitExceededCalled = false;

    observer->mapboxTileCountLimitExceededFn = [&](uint64_t limit) {
        EXPECT_FALSE(mapboxTileCountLimitExceededCalled);
        EXPECT_EQ(tileLimit, limit);
        mapboxTileCountLimitExceededCalled = true;
    };

    observer->statusChangedFn = [&](OfflineRegionStatus status) {
        if (!mapboxTileCountLimitExceededCalled) {
            EXPECT_FALSE(status.complete());
            EXPECT_EQ(OfflineRegionDownloadState::Active, status.downloadState);
        } else {
            EXPECT_EQ(OfflineRegionDownloadState::Inactive, status.downloadState);
            test.loop.stop();
        }
    };

    download.setObserver(std::move(observer));
    download.setState(OfflineRegionDownloadState::Active);

    test.loop.run();
}

TEST(OfflineDownload, TileCountLimitExceededWithTileResponse) {
    OfflineTest test;
    auto region = test.createRegion();
    ASSERT_TRUE(region);
    OfflineDownload download(region->getID(),
                             OfflineTilePyramidRegionDefinition(
                                 "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, true),
                             test.db,
                             test.fileSource);

    uint64_t tileLimit = 1;

    test.db.setOfflineMapboxTileCountLimit(tileLimit);

    test.fileSource.styleResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/style.json", resource.url);
        return test.response("mapbox_source.style.json");
    };

    test.fileSource.tileResponse = [&](const Resource& resource) {
        const Resource::TileData& tile = *resource.tileData;
        EXPECT_EQ("maptiler://{z}-{x}-{y}.vector.pbf", tile.urlTemplate);
        EXPECT_EQ(1, tile.pixelRatio);
        EXPECT_EQ(0, tile.x);
        EXPECT_EQ(0, tile.y);
        EXPECT_EQ(0, tile.z);
        return test.response("0-0-0.vector.pbf");
    };

    auto observer = std::make_unique<MockObserver>();
    bool mapboxTileCountLimitExceededCalled = false;

    observer->mapboxTileCountLimitExceededFn = [&](uint64_t limit) {
        EXPECT_FALSE(mapboxTileCountLimitExceededCalled);
        EXPECT_EQ(tileLimit, limit);
        mapboxTileCountLimitExceededCalled = true;
    };

    observer->statusChangedFn = [&](OfflineRegionStatus status) {
        if (!mapboxTileCountLimitExceededCalled) {
            EXPECT_EQ(OfflineRegionDownloadState::Active, status.downloadState);
        } else {
            EXPECT_EQ(OfflineRegionDownloadState::Inactive, status.downloadState);
            test.loop.stop();
        }
        if (status.completedResourceCount > tileLimit) {
            test.loop.stop();
        }
    };

    download.setObserver(std::move(observer));
    download.setState(OfflineRegionDownloadState::Active);

    test.loop.run();
}

TEST(OfflineDownload, WithPreviouslyExistingTile) {
    OfflineTest test;
    auto region = test.createRegion();
    ASSERT_TRUE(region);
    OfflineDownload download(region->getID(),
                             OfflineTilePyramidRegionDefinition(
                                 "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, false),
                             test.db,
                             test.fileSource);

    test.fileSource.styleResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/style.json", resource.url);
        return test.response("inline_source.style.json");
    };

    test.db.put(Resource::tile("http://127.0.0.1:3000/{z}-{x}-{y}.vector.pbf", 1, 0, 0, 0, Tileset::Scheme::XYZ),
                test.response("0-0-0.vector.pbf"));

    auto observer = std::make_unique<MockObserver>();

    observer->statusChangedFn = [&](OfflineRegionStatus status) {
        if (status.complete()) {
            EXPECT_EQ(2u, status.completedResourceCount);
            EXPECT_EQ(test.size, status.completedResourceSize);
            EXPECT_TRUE(status.requiredResourceCountIsPrecise);
            test.loop.stop();
        }
    };

    download.setObserver(std::move(observer));
    download.setState(OfflineRegionDownloadState::Active);

    test.loop.run();
}

class OfflineDownloadRevalidation : public testing::Test {
protected:
    OfflineTest test;
    FakeFileSource fileSource;
    const Resource style = Resource::style("http://127.0.0.1:3000/style.json");
    const Resource tile = Resource::tile(
        "http://127.0.0.1:3000/{z}-{x}-{y}.vector.pbf", 1, 0, 0, 0, Tileset::Scheme::XYZ);
    const OfflineRegionDefinition definition = OfflineTilePyramidRegionDefinition(
        style.url, LatLngBounds::world(), 0.0, 0.0, 1.0, false);
    Response originalStyle;
    Response originalTile;
    std::optional<OfflineRegion> region;
    std::unique_ptr<OfflineDownload> download;

    void SetUp() override {
        auto result = test.db.createRegion(definition, {1, 2, 3});
        ASSERT_TRUE(result);
        region.emplace(std::move(*result));
        originalStyle = test.response("inline_source.style.json");
        originalTile = test.response("0-0-0.vector.pbf");
        originalStyle.etag = "style-1";
        originalTile.etag = "tile-1";
        originalStyle.modified = originalTile.modified = Timestamp(Seconds(1417392000));
        originalStyle.expires = originalTile.expires = util::now() + std::chrono::hours(24);
        originalStyle.mustRevalidate = originalTile.mustRevalidate = true;
        test.db.putRegionResource(region->getID(), style, originalStyle);
        test.db.putRegionResource(region->getID(), tile, originalTile);
        download = std::make_unique<OfflineDownload>(region->getID(), definition, test.db, fileSource);
    }

    void start() {
        download->setState(OfflineRegionDownloadState::Active);
        test.loop.runOnce();
    }

    void invalidateTile() {
        auto invalid = originalTile;
        invalid.expires = Timestamp(Seconds(0));
        test.db.putRegionResource(region->getID(), tile, invalid);
    }

    void expectRequest(const Resource& request, const Resource& resource, const Response& previous) {
        EXPECT_EQ(resource.url, request.url);
        EXPECT_EQ(Resource::Priority::Low, request.priority);
        EXPECT_EQ(Resource::Usage::Offline, request.usage);
        EXPECT_EQ(previous.etag, request.priorEtag);
        // Compared with ==, not EXPECT_EQ: printing a failed time_point comparison uses std::format's
        // floating-point formatter, which the iOS deployment target does not have.
        EXPECT_TRUE(previous.modified == request.priorModified);
        EXPECT_TRUE(std::optional<Timestamp>(Timestamp(Seconds(0))) == request.priorExpires);
        ASSERT_TRUE(request.priorData);
        EXPECT_EQ(*previous.data, *request.priorData);
        if (resource.tileData) {
            ASSERT_TRUE(request.tileData);
            EXPECT_EQ(resource.tileData->urlTemplate, request.tileData->urlTemplate);
            EXPECT_EQ(resource.tileData->pixelRatio, request.tileData->pixelRatio);
            EXPECT_EQ(resource.tileData->x, request.tileData->x);
            EXPECT_EQ(resource.tileData->y, request.tileData->y);
            EXPECT_EQ(resource.tileData->z, request.tileData->z);
        }
    }

    void expectRetainedTile() {
        auto cached = test.db.get(tile);
        ASSERT_TRUE(cached);
        ASSERT_TRUE(cached->data);
        EXPECT_EQ(*originalTile.data, *cached->data);
        EXPECT_EQ(originalTile.etag, cached->etag);
        EXPECT_FALSE(cached->isUsable());
    }
};

TEST_F(OfflineDownloadRevalidation, InvalidatedRegionRevalidatesStyleAndTile) {
    ASSERT_FALSE(test.db.invalidateRegion(region->getID()));
    start();

    ASSERT_EQ(1u, fileSource.requests.size());
    expectRequest(fileSource.requests.front()->resource, style, originalStyle);
    EXPECT_FALSE(download->getStatus().complete());
    EXPECT_EQ(0u, download->getStatus().completedResourceCount);
    expectRetainedTile();

    auto updatedStyle = originalStyle;
    updatedStyle.etag = "style-2";
    ASSERT_TRUE(fileSource.respond(Resource::Kind::Style, updatedStyle));
    test.loop.runOnce();

    ASSERT_EQ(1u, fileSource.requests.size());
    expectRequest(fileSource.requests.front()->resource, tile, originalTile);
    EXPECT_FALSE(download->getStatus().complete());
    EXPECT_EQ(0u, download->getStatus().completedTileCount);
    expectRetainedTile();

    auto updatedTile = originalTile;
    updatedTile.data = std::make_shared<std::string>(*originalTile.data + "updated");
    updatedTile.etag = "tile-2";
    ASSERT_TRUE(fileSource.respond(Resource::Kind::Tile, updatedTile));

    EXPECT_TRUE(fileSource.requests.empty());
    EXPECT_TRUE(download->getStatus().complete());
    EXPECT_EQ(2u, download->getStatus().completedResourceCount);
    EXPECT_EQ(1u, download->getStatus().completedTileCount);
    auto cached = test.db.get(tile);
    ASSERT_TRUE(cached);
    ASSERT_TRUE(cached->data);
    EXPECT_EQ(*updatedTile.data, *cached->data);
    EXPECT_EQ(updatedTile.etag, cached->etag);
    EXPECT_TRUE(cached->isUsable());
    auto storedRegion = test.db.getRegion(region->getID());
    ASSERT_TRUE(storedRegion);
    ASSERT_TRUE(*storedRegion);
    EXPECT_EQ(region->getMetadata(), (*storedRegion)->getMetadata());
}

TEST_F(OfflineDownloadRevalidation, UsableCachedTileDoesNotRequestNetwork) {
    start();

    EXPECT_TRUE(fileSource.requests.empty());
    EXPECT_TRUE(download->getStatus().complete());
    EXPECT_EQ(1u, download->getStatus().completedTileCount);
    EXPECT_EQ(test.size, download->getStatus().completedResourceSize);
}

TEST_F(OfflineDownloadRevalidation, StaleTileWithoutMustRevalidateDoesNotRequestNetwork) {
    auto stale = originalTile;
    stale.expires = Timestamp(Seconds(0));
    stale.mustRevalidate = false;
    test.db.putRegionResource(region->getID(), tile, stale);
    start();

    EXPECT_TRUE(fileSource.requests.empty());
    EXPECT_TRUE(download->getStatus().complete());
    EXPECT_EQ(1u, download->getStatus().completedTileCount);
}

TEST_F(OfflineDownloadRevalidation, UnusableSharedTileIsRevalidatedBeforeJoiningRegion) {
    auto newRegion = test.db.createRegion(definition, {4, 5, 6});
    ASSERT_TRUE(newRegion);
    test.db.putRegionResource(newRegion->getID(), style, originalStyle);
    invalidateTile();
    download = std::make_unique<OfflineDownload>(newRegion->getID(), definition, test.db, fileSource);
    start();

    ASSERT_EQ(1u, fileSource.requests.size());
    expectRequest(fileSource.requests.front()->resource, tile, originalTile);
    auto before = test.db.getRegionCompletedStatus(newRegion->getID());
    ASSERT_TRUE(before);
    EXPECT_EQ(0u, before->completedTileCount);
    expectRetainedTile();

    ASSERT_TRUE(fileSource.respond(Resource::Kind::Tile, originalTile));
    EXPECT_TRUE(download->getStatus().complete());
    auto after = test.db.getRegionCompletedStatus(newRegion->getID());
    ASSERT_TRUE(after);
    EXPECT_EQ(1u, after->completedTileCount);
}

TEST_F(OfflineDownloadRevalidation, TileWithMustRevalidateAndNoExpirationRequestsNetwork) {
    auto invalid = originalTile;
    invalid.expires = std::nullopt;
    test.db.putRegionResource(region->getID(), tile, invalid);
    start();

    ASSERT_EQ(1u, fileSource.requests.size());
    EXPECT_EQ(tile.url, fileSource.requests.front()->resource.url);
    EXPECT_FALSE(fileSource.requests.front()->resource.priorExpires);
    EXPECT_EQ(originalTile.etag, fileSource.requests.front()->resource.priorEtag);
    ASSERT_TRUE(fileSource.requests.front()->resource.priorData);
    EXPECT_EQ(*originalTile.data, *fileSource.requests.front()->resource.priorData);
    EXPECT_FALSE(download->getStatus().complete());
    expectRetainedTile();
    ASSERT_TRUE(fileSource.respond(Resource::Kind::Tile, originalTile));
    EXPECT_TRUE(download->getStatus().complete());
}

TEST_F(OfflineDownloadRevalidation, CancelledRevalidationRetainsTileAndCanResume) {
    invalidateTile();
    start();
    ASSERT_EQ(1u, fileSource.requests.size());
    EXPECT_FALSE(download->getStatus().complete());

    download->setState(OfflineRegionDownloadState::Inactive);
    EXPECT_TRUE(fileSource.requests.empty());
    expectRetainedTile();

    start();
    ASSERT_EQ(1u, fileSource.requests.size());
    expectRequest(fileSource.requests.front()->resource, tile, originalTile);
    ASSERT_TRUE(fileSource.respond(Resource::Kind::Tile, originalTile));
    EXPECT_TRUE(download->getStatus().complete());
    EXPECT_EQ(1u, download->getStatus().completedTileCount);
    auto cached = test.db.get(tile);
    ASSERT_TRUE(cached);
    ASSERT_TRUE(cached->data);
    EXPECT_EQ(*originalTile.data, *cached->data);
    EXPECT_TRUE(cached->isUsable());
}

TEST_F(OfflineDownloadRevalidation, FailedRevalidationRetainsTileUntilSuccess) {
    unsigned errors = 0;
    auto observer = std::make_unique<MockObserver>();
    observer->responseErrorFn = [&](Response::Error) {
        ++errors;
    };
    download->setObserver(std::move(observer));
    invalidateTile();
    start();
    ASSERT_EQ(1u, fileSource.requests.size());

    for (auto reason : {Response::Error::Reason::Connection, Response::Error::Reason::Server}) {
        Response failed;
        failed.error = std::make_unique<Response::Error>(reason);
        failed.data = std::make_shared<std::string>("error response");
        ASSERT_TRUE(fileSource.respond(Resource::Kind::Tile, failed));
        EXPECT_EQ(1u, fileSource.requests.size());
        EXPECT_FALSE(download->getStatus().complete());
        EXPECT_EQ(0u, download->getStatus().completedTileCount);
        expectRetainedTile();
    }
    EXPECT_EQ(2u, errors);
    ASSERT_TRUE(fileSource.respond(Resource::Kind::Tile, originalTile));
    EXPECT_TRUE(download->getStatus().complete());
    EXPECT_EQ(1u, download->getStatus().completedTileCount);
}

TEST(OfflineDownload, RevalidationAtTileLimitDoesNotConsumeNewTileQuota) {
    OfflineTest test;
    ASSERT_EQ(0u, test.db.getOfflineMapboxTileCount());
    FakeFileSource fileSource(ResourceOptions().withTileServerOptions(TileServerOptions::MapTilerConfiguration()),
                              ClientOptions());
    const Resource style = Resource::style("http://127.0.0.1:3000/style.json");
    const Resource tile = Resource::tile("maptiler://{z}-{x}-{y}.vector.pbf", 1, 0, 0, 0, Tileset::Scheme::XYZ);
    const OfflineRegionDefinition definition = OfflineTilePyramidRegionDefinition(
        style.url, LatLngBounds::world(), 0.0, 0.0, 1.0, false);
    auto region = test.db.createRegion(definition, {});
    ASSERT_TRUE(region);
    test.db.putRegionResource(region->getID(), style, test.response("mapbox_source.style.json"));
    auto cachedTile = test.response("0-0-0.vector.pbf");
    cachedTile.etag = "tile-1";
    cachedTile.expires = Timestamp(Seconds(0));
    cachedTile.mustRevalidate = true;
    test.db.putRegionResource(region->getID(), tile, cachedTile);
    test.db.setOfflineMapboxTileCountLimit(1);
    ASSERT_EQ(1u, test.db.getOfflineMapboxTileCount());
    ASSERT_TRUE(test.db.offlineMapboxTileCountLimitExceeded());

    OfflineDownload download(region->getID(), definition, test.db, fileSource);
    unsigned quotaNotifications = 0;
    auto observer = std::make_unique<MockObserver>();
    observer->mapboxTileCountLimitExceededFn = [&](uint64_t) {
        ++quotaNotifications;
    };
    download.setObserver(std::move(observer));
    download.setState(OfflineRegionDownloadState::Active);
    test.loop.runOnce();

    ASSERT_EQ(1u, fileSource.requests.size());
    EXPECT_EQ(tile.url, fileSource.requests.front()->resource.url);
    EXPECT_EQ(cachedTile.etag, fileSource.requests.front()->resource.priorEtag);
    EXPECT_FALSE(download.getStatus().complete());
    cachedTile.expires = util::now() + std::chrono::hours(24);
    ASSERT_TRUE(fileSource.respond(Resource::Kind::Tile, cachedTile));
    EXPECT_TRUE(download.getStatus().complete());
    EXPECT_EQ(1u, download.getStatus().completedTileCount);
    EXPECT_EQ(1u, test.db.getOfflineMapboxTileCount());
    EXPECT_EQ(0u, quotaNotifications);
    auto cached = test.db.get(tile);
    ASSERT_TRUE(cached);
    ASSERT_TRUE(cached->data);
    EXPECT_EQ(*cachedTile.data, *cached->data);
    EXPECT_TRUE(cached->isUsable());
}

TEST(OfflineDownload, TEST_REQUIRES_SERVER(RevalidationNotModifiedRetainsTileDataAndSize)) {
    OfflineTest test;
    OnlineFileSource fileSource(ResourceOptions::Default(), ClientOptions());
    const Resource style = Resource::style("http://127.0.0.1:3000/style.json");
    const Resource tile = Resource::tile("http://127.0.0.1:3000/revalidate-same", 1, 0, 0, 0, Tileset::Scheme::XYZ);
    const OfflineRegionDefinition definition = OfflineTilePyramidRegionDefinition(
        style.url, LatLngBounds::world(), 0.0, 0.0, 1.0, false);
    auto region = test.db.createRegion(definition, {});
    ASSERT_TRUE(region);
    Response cachedStyle;
    cachedStyle.data = std::make_shared<std::string>(
        R"({"version":8,"sources":{"tiles":{"type":"vector","tiles":["http://127.0.0.1:3000/revalidate-same"]}},"layers":[]})");
    test.db.putRegionResource(region->getID(), style, cachedStyle);
    auto cachedTile = test.response("0-0-0.vector.pbf");
    cachedTile.etag = "snowfall";
    cachedTile.modified = Timestamp(Seconds(1417392000));
    cachedTile.expires = Timestamp(Seconds(0));
    cachedTile.mustRevalidate = true;
    const auto tileSize = test.db.putRegionResource(region->getID(), tile, cachedTile);
    OfflineDownload download(region->getID(), definition, test.db, fileSource);
    auto observer = std::make_unique<MockObserver>();
    observer->statusChangedFn = [&](OfflineRegionStatus status) {
        if (status.complete()) test.loop.stop();
    };
    download.setObserver(std::move(observer));
    download.setState(OfflineRegionDownloadState::Active);
    util::Timer timeout;
    timeout.start(std::chrono::seconds(5), Duration::zero(), [&] {
        ADD_FAILURE() << "Offline revalidation did not finish";
        test.loop.stop();
    });
    test.loop.run();

    auto cached = test.db.get(tile);
    ASSERT_TRUE(cached);
    ASSERT_TRUE(cached->data);
    EXPECT_EQ(*cachedTile.data, *cached->data);
    EXPECT_EQ(cachedTile.etag, cached->etag);
    EXPECT_TRUE(cached->mustRevalidate);
    ASSERT_TRUE(cached->expires);
    EXPECT_TRUE(*cached->expires > *cachedTile.expires); // not EXPECT_GT: see expectRequest
    EXPECT_EQ(tileSize, download.getStatus().completedTileSize);
    EXPECT_EQ(1u, download.getStatus().completedTileCount);
}

#ifndef __QT__ // Needs a database file that a second connection can edit.
// Resuming a region must not load or touch tiles that are still usable: only entries that are past `expires`
// with `must_revalidate` set need the full read, which also bumps `accessed`.
TEST(OfflineDownload, ResumeOnlyReadsTilesThatNeedRevalidation) {
    deleteDatabaseFiles();
    OfflineTest test{filename};
    FakeFileSource fileSource;
    const Resource style = Resource::style("http://127.0.0.1:3000/style.json");
    const Resource tile = Resource::tile(
        "http://127.0.0.1:3000/{z}-{x}-{y}.vector.pbf", 1, 0, 0, 0, Tileset::Scheme::XYZ);
    const OfflineRegionDefinition definition = OfflineTilePyramidRegionDefinition(
        style.url, LatLngBounds::world(), 0.0, 0.0, 1.0, false);
    auto region = test.db.createRegion(definition, {});
    ASSERT_TRUE(region);

    auto cachedStyle = test.response("inline_source.style.json");
    cachedStyle.expires = util::now() + std::chrono::hours(24);
    cachedStyle.mustRevalidate = true;
    test.db.putRegionResource(region->getID(), style, cachedStyle);
    auto cachedTile = test.response("0-0-0.vector.pbf");
    cachedTile.etag = "tile-1";
    cachedTile.expires = util::now() + std::chrono::hours(24);
    cachedTile.mustRevalidate = true;
    test.db.putRegionResource(region->getID(), tile, cachedTile);

    constexpr int64_t oldAccessed = 1000;
    auto tileAccessed = [&](std::optional<int64_t> set = std::nullopt) {
        mapbox::sqlite::Database db = mapbox::sqlite::Database::open(filename, mapbox::sqlite::ReadWriteCreate);
        if (set) {
            mapbox::sqlite::Statement update{db, "UPDATE tiles SET accessed = ?1"};
            mapbox::sqlite::Query updateQuery{update};
            updateQuery.bind(1, *set);
            updateQuery.run();
        }
        mapbox::sqlite::Statement select{db, "SELECT accessed FROM tiles"};
        mapbox::sqlite::Query selectQuery{select};
        EXPECT_TRUE(selectQuery.run());
        return selectQuery.get<int64_t>(0);
    };

    // A usable tile completes without a network request, a blob read or an `accessed` write.
    ASSERT_EQ(oldAccessed, tileAccessed(oldAccessed));
    {
        OfflineDownload download(region->getID(), definition, test.db, fileSource);
        download.setState(OfflineRegionDownloadState::Active);
        test.loop.runOnce();
        EXPECT_TRUE(fileSource.requests.empty());
        EXPECT_TRUE(download.getStatus().complete());
        EXPECT_EQ(1u, download.getStatus().completedTileCount);
        EXPECT_EQ(oldAccessed, tileAccessed());
    }

    // An expired must-revalidate tile still gets the full read and is requested with its validators.
    cachedTile.expires = Timestamp(Seconds(0));
    test.db.putRegionResource(region->getID(), tile, cachedTile);
    ASSERT_EQ(oldAccessed, tileAccessed(oldAccessed));
    {
        OfflineDownload download(region->getID(), definition, test.db, fileSource);
        download.setState(OfflineRegionDownloadState::Active);
        test.loop.runOnce();
        ASSERT_EQ(1u, fileSource.requests.size());
        EXPECT_EQ(tile.url, fileSource.requests.front()->resource.url);
        EXPECT_EQ(cachedTile.etag, fileSource.requests.front()->resource.priorEtag);
        ASSERT_TRUE(fileSource.requests.front()->resource.priorData);
        EXPECT_EQ(*cachedTile.data, *fileSource.requests.front()->resource.priorData);
        EXPECT_FALSE(download.getStatus().complete());
        EXPECT_LT(oldAccessed, tileAccessed());
    }
}
#endif // __QT__

TEST(OfflineDownload, ReactivatePreviouslyCompletedDownload) {
    OfflineTest test;
    auto region = test.createRegion();
    ASSERT_TRUE(region);
    OfflineDownload download(region->getID(),
                             OfflineTilePyramidRegionDefinition(
                                 "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, true),
                             test.db,
                             test.fileSource);

    test.fileSource.styleResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/style.json", resource.url);
        return test.response("inline_source.style.json");
    };

    test.db.put(Resource::tile("http://127.0.0.1:3000/{z}-{x}-{y}.vector.pbf", 1, 0, 0, 0, Tileset::Scheme::XYZ),
                test.response("0-0-0.vector.pbf"));

    auto observer = std::make_unique<MockObserver>();
    observer->statusChangedFn = [&](OfflineRegionStatus status) {
        if (status.complete()) {
            test.loop.stop();
        }
    };

    download.setObserver(std::move(observer));
    download.setState(OfflineRegionDownloadState::Active);

    test.loop.run();

    OfflineDownload redownload(region->getID(),
                               OfflineTilePyramidRegionDefinition(
                                   "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, false),
                               test.db,
                               test.fileSource);

    std::vector<OfflineRegionStatus> statusesAfterReactivate;

    observer = std::make_unique<MockObserver>();
    observer->statusChangedFn = [&](OfflineRegionStatus status) {
        statusesAfterReactivate.push_back(status);
        if (status.complete()) {
            test.loop.stop();
        }
    };

    redownload.setObserver(std::move(observer));
    redownload.setState(OfflineRegionDownloadState::Active);

    test.loop.run();

    ASSERT_EQ(4u, statusesAfterReactivate.size());

    EXPECT_EQ(OfflineRegionDownloadState::Active, statusesAfterReactivate[0].downloadState);
    EXPECT_FALSE(statusesAfterReactivate[0].requiredResourceCountIsPrecise);
    EXPECT_EQ(1u, statusesAfterReactivate[0].requiredResourceCount);
    EXPECT_EQ(0u, statusesAfterReactivate[0].completedResourceCount);

    EXPECT_EQ(OfflineRegionDownloadState::Active, statusesAfterReactivate[1].downloadState);
    EXPECT_TRUE(statusesAfterReactivate[1].requiredResourceCountIsPrecise);
    EXPECT_EQ(2u, statusesAfterReactivate[1].requiredResourceCount);
    EXPECT_EQ(1u, statusesAfterReactivate[1].completedResourceCount);

    EXPECT_EQ(OfflineRegionDownloadState::Active, statusesAfterReactivate[2].downloadState);
    EXPECT_TRUE(statusesAfterReactivate[2].requiredResourceCountIsPrecise);
    EXPECT_EQ(2u, statusesAfterReactivate[2].requiredResourceCount);
    EXPECT_EQ(2u, statusesAfterReactivate[2].completedResourceCount);
}

TEST(OfflineDownload, Deactivate) {
    OfflineTest test;
    auto region = test.createRegion();
    ASSERT_TRUE(region);
    OfflineDownload download(region->getID(),
                             OfflineTilePyramidRegionDefinition(
                                 "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, false),
                             test.db,
                             test.fileSource);

    test.fileSource.styleResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/style.json", resource.url);
        return test.response("mapbox_source.style.json");
    };

    auto observer = std::make_unique<MockObserver>();

    observer->statusChangedFn = [&](OfflineRegionStatus status) {
        if (status.downloadState == OfflineRegionDownloadState::Inactive) {
            test.loop.stop();
        } else {
            download.setState(OfflineRegionDownloadState::Inactive);
        }
    };

    download.setObserver(std::move(observer));
    download.setState(OfflineRegionDownloadState::Active);

    test.loop.run();
}

TEST(OfflineDownload, AllOfflineRequestsHaveLowPriorityAndOfflineUsage) {
    OfflineTest test;
    auto region = test.createRegion();
    ASSERT_TRUE(region);
    OfflineDownload download(region->getID(),
                             OfflineTilePyramidRegionDefinition(
                                 "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, true),
                             test.db,
                             test.fileSource);

    test.fileSource.styleResponse = [&](const Resource& resource) {
        EXPECT_TRUE(resource.priority == Resource::Priority::Low);
        EXPECT_TRUE(resource.usage == Resource::Usage::Offline);
        return test.response("style.json");
    };

    test.fileSource.spriteImageResponse = [&](const Resource& resource) {
        EXPECT_TRUE(resource.priority == Resource::Priority::Low);
        EXPECT_TRUE(resource.usage == Resource::Usage::Offline);
        return test.response("sprite.png");
    };

    test.fileSource.imageResponse = [&](const Resource& resource) {
        EXPECT_TRUE(resource.priority == Resource::Priority::Low);
        EXPECT_TRUE(resource.usage == Resource::Usage::Offline);
        return test.response("radar.gif");
    };

    test.fileSource.spriteJSONResponse = [&](const Resource& resource) {
        EXPECT_TRUE(resource.priority == Resource::Priority::Low);
        EXPECT_TRUE(resource.usage == Resource::Usage::Offline);
        return test.response("sprite.json");
    };

    test.fileSource.glyphsResponse = [&](const Resource& resource) {
        EXPECT_TRUE(resource.priority == Resource::Priority::Low);
        EXPECT_TRUE(resource.usage == Resource::Usage::Offline);
        return test.response("glyph.pbf");
    };

    test.fileSource.sourceResponse = [&](const Resource& resource) {
        EXPECT_TRUE(resource.priority == Resource::Priority::Low);
        EXPECT_TRUE(resource.usage == Resource::Usage::Offline);
        return test.response("streets.json");
    };

    test.fileSource.tileResponse = [&](const Resource& resource) {
        EXPECT_TRUE(resource.priority == Resource::Priority::Low);
        EXPECT_TRUE(resource.usage == Resource::Usage::Offline);
        return test.response("0-0-0.vector.pbf");
    };

    auto observer = std::make_unique<MockObserver>();

    observer->statusChangedFn = [&](OfflineRegionStatus status) {
        if (status.complete()) {
            test.loop.stop();
        }
    };

    download.setObserver(std::move(observer));
    download.setState(OfflineRegionDownloadState::Active);

    test.loop.run();
}

#ifndef __QT__ // Qt doesn't expose the ability to register virtual file system handlers.
TEST(OfflineDownload, DiskFull) {
    FixtureLog log;
    deleteDatabaseFiles();
    test::SQLite3TestFS fs;

    OfflineTest test{filename_test_fs};
    EXPECT_EQ(0u, log.uncheckedCount());

    auto region = test.createRegion();
    ASSERT_TRUE(region);
    EXPECT_EQ(0u, log.uncheckedCount());

    // Simulate a full disk.
    fs.setWriteLimit(8192);

    OfflineDownload download(region->getID(),
                             OfflineTilePyramidRegionDefinition(
                                 "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, false),
                             test.db,
                             test.fileSource);

    bool hasRequestedStyle = false;

    test.fileSource.styleResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/style.json", resource.url);
        hasRequestedStyle = true;
        return test.response("empty.style.json");
    };

    auto observer = std::make_unique<MockObserver>();

    observer->statusChangedFn = [&](OfflineRegionStatus status) {
        EXPECT_EQ(OfflineRegionDownloadState::Active, status.downloadState);
        EXPECT_EQ(0u, status.completedResourceCount);
        EXPECT_EQ(0u, status.completedResourceSize);
        EXPECT_EQ(hasRequestedStyle, status.requiredResourceCountIsPrecise);
        EXPECT_FALSE(status.complete());

        if (hasRequestedStyle) {
            EXPECT_EQ(1u,
                      log.count(warning(ResultCode::Full, "Can't write region resources: database or disk is full")));
            EXPECT_EQ(0u, log.uncheckedCount());
            test.loop.stop();
        }
    };

    download.setObserver(std::move(observer));
    download.setState(OfflineRegionDownloadState::Active);

    test.loop.run();
    EXPECT_EQ(0u, log.uncheckedCount());
}

// Test verifies that resource requests for invalidated region don't
// have Resource::Usage::Offline tag set.
TEST(OfflineDownload, ResourceOfflineUsageUnset) {
    deleteDatabaseFiles();
    test::SQLite3TestFS fs;

    OfflineTest test{filename_test_fs};
    auto region = test.createRegion();
    ASSERT_TRUE(region);

    OfflineDownload download(
        region->getID(),
        OfflineTilePyramidRegionDefinition(
            "http://127.0.0.1:3000/inline_source.style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, false),
        test.db,
        test.fileSource);

    test.fileSource.styleResponse = [&](const Resource& resource) {
        EXPECT_TRUE(resource.priority == Resource::Priority::Low);
        EXPECT_TRUE(resource.usage == Resource::Usage::Offline);
        return test.response("inline_source.style.json");
    };

    test.fileSource.tileResponse = [&](const Resource& resource) {
        EXPECT_TRUE(resource.priority == Resource::Priority::Low);
        EXPECT_TRUE(resource.usage == Resource::Usage::Offline);
        return test.response("0-0-0.vector.pbf");
    };

    auto observer = std::make_unique<MockObserver>();
    observer->statusChangedFn = [&](OfflineRegionStatus status) {
        if (status.complete()) {
            // Once download completes, invalidate region and try rendering map.
            // Resource requests must not have Offline usage tag.
            ASSERT_FALSE(test.invalidateRegion(region->getID()));
            test.loop.stop();
        }
    };

    download.setObserver(std::move(observer));
    download.setState(OfflineRegionDownloadState::Active);
    test.loop.run();

    std::shared_ptr<StubFileSource> stubfileSource = std::make_shared<StubFileSource>();
    stubfileSource->styleResponse = [&](const Resource& resource) {
        EXPECT_TRUE(resource.usage != Resource::Usage::Offline);
        return test.response("inline_source.style.json");
    };

    stubfileSource->tileResponse = [&](const Resource& resource) {
        EXPECT_TRUE(resource.usage != Resource::Usage::Offline);
        return test.response("0-0-0.vector.pbf");
    };

    StubMapObserver mapObserver;
    mapObserver.didFinishRenderingFrameCallback = [&](MapObserver::RenderFrameStatus status) {
        if (status.mode == MapObserver::RenderMode::Full) {
            test.loop.stop();
        }
    };

    HeadlessFrontend frontend{1};
    MapAdapter map{frontend,
                   mapObserver,
                   stubfileSource,
                   MapOptions().withMapMode(MapMode::Continuous).withSize(frontend.getSize())};

    map.getStyle().loadURL("http://127.0.0.1:3000/inline_source.style.json");
    map.jumpTo(CameraOptions().withCenter(LatLng{0.0, 0.0}).withZoom(0));
    test.loop.run();
}
#endif // __QT__

// Test verifies that resource batch buffer is cleared at loading interrupt.
TEST(OfflineDownload, InterruptAndResume) {
    OfflineTest test;
    auto region = test.createRegion();
    ASSERT_TRUE(region);
    OfflineDownload download(region->getID(),
                             OfflineTilePyramidRegionDefinition(
                                 "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, true),
                             test.db,
                             test.fileSource);

    test.fileSource.styleResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/style.json", resource.url);
        return test.response("style.json");
    };

    test.fileSource.spriteImageResponse = [&](const Resource& resource) {
        EXPECT_TRUE(resource.url == "http://127.0.0.1:3000/sprite.png" ||
                    resource.url == "http://127.0.0.1:3000/sprite@2x.png");
        return test.response("sprite.png");
    };

    test.fileSource.imageResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/radar.gif", resource.url);
        return test.response("radar.gif");
    };

    test.fileSource.spriteJSONResponse = [&](const Resource& resource) {
        EXPECT_TRUE(resource.url == "http://127.0.0.1:3000/sprite.json" ||
                    resource.url == "http://127.0.0.1:3000/sprite@2x.json");
        return test.response("sprite.json");
    };

    test.fileSource.glyphsResponse = [&](const Resource&) {
        return test.response("glyph.pbf");
    };

    test.fileSource.sourceResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/streets.json", resource.url);
        return test.response("streets.json");
    };

    test.fileSource.tileResponse = [&](const Resource& resource) {
        const Resource::TileData& tile = *resource.tileData;
        EXPECT_EQ("http://127.0.0.1:3000/{z}-{x}-{y}.vector.pbf", tile.urlTemplate);
        EXPECT_EQ(1, tile.pixelRatio);
        EXPECT_EQ(0, tile.x);
        EXPECT_EQ(0, tile.y);
        EXPECT_EQ(0, tile.z);
        return test.response("0-0-0.vector.pbf");
    };

    auto observer = std::make_unique<MockObserver>();
    bool interrupted = false;
    observer->statusChangedFn = [&](OfflineRegionStatus status) {
        if (!interrupted && status.completedResourceCount > 0) {
            interrupted = true;
            assert(!status.complete());
            test.loop.schedule([&]() {
                download.setState(OfflineRegionDownloadState::Inactive);
                test.loop.stop();
            });
        }
    };

    download.setObserver(std::move(observer));
    download.setState(OfflineRegionDownloadState::Active);
    test.loop.run();

    auto newObserver = std::make_unique<MockObserver>();
    newObserver->statusChangedFn = [&](OfflineRegionStatus status) {
        if (status.complete()) {
            EXPECT_EQ(status.completedTileCount, status.requiredTileCount);
            EXPECT_EQ(264u,
                      status.completedResourceCount); // 256 glyphs, 2 sprite images,
                                                      // 2 sprite jsons, 1 tile, 1
                                                      // style, source, image
            test.loop.stop();
        }
    };

    download.setObserver(std::move(newObserver));
    download.setState(OfflineRegionDownloadState::Active);
    test.loop.run();
}

TEST(OfflineDownload, NoFreezingOnCachedTilesAndNewStyle) {
    OfflineTest test;
    auto region = test.createRegion();
    ASSERT_TRUE(region);
    OfflineDownload download(region->getID(),
                             OfflineTilePyramidRegionDefinition(
                                 "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 1.0, 1.0, 1.0, true),
                             test.db,
                             test.fileSource);

    test.fileSource.setProperty(MAX_CONCURRENT_REQUESTS_KEY, 2u);
    test.fileSource.styleResponse = [&](const Resource&) {
        return test.response("inline_source.style.json");
    };
    // Number of resources must exceed MAX_CONCURRENT_REQUESTS_KEY
    test.db.put(Resource::tile("http://127.0.0.1:3000/{z}-{x}-{y}.vector.pbf", 1, 0, 0, 1, Tileset::Scheme::XYZ),
                test.response("0-0-0.vector.pbf"));
    test.db.put(Resource::tile("http://127.0.0.1:3000/{z}-{x}-{y}.vector.pbf", 1, 0, 1, 1, Tileset::Scheme::XYZ),
                test.response("0-0-0.vector.pbf"));
    test.db.put(Resource::tile("http://127.0.0.1:3000/{z}-{x}-{y}.vector.pbf", 1, 1, 0, 1, Tileset::Scheme::XYZ),
                test.response("0-0-0.vector.pbf"));
    test.db.put(Resource::tile("http://127.0.0.1:3000/{z}-{x}-{y}.vector.pbf", 1, 1, 1, 1, Tileset::Scheme::XYZ),
                test.response("0-0-0.vector.pbf"));

    auto observer = std::make_unique<MockObserver>();
    observer->statusChangedFn = [&](OfflineRegionStatus status) {
        if (status.complete()) {
            test.loop.stop();
        }
    };

    download.setObserver(std::move(observer));
    download.setState(OfflineRegionDownloadState::Active);

    test.loop.run();
    // Passes if does not freeze.
}

TEST(OfflineDownload, NoFreezingOnNotFoundError) {
    OfflineTest test;
    auto region = test.createRegion();
    ASSERT_TRUE(region);
    OfflineDownload download(region->getID(),
                             OfflineTilePyramidRegionDefinition(
                                 "http://127.0.0.1:3000/style.json", LatLngBounds::world(), 0.0, 0.0, 1.0, true),
                             test.db,
                             test.fileSource);

    test.fileSource.styleResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/style.json", resource.url);
        return test.response("style.json");
    };

    test.fileSource.spriteImageResponse = [&](const Resource& resource) {
        EXPECT_TRUE(resource.url == "http://127.0.0.1:3000/sprite.png" ||
                    resource.url == "http://127.0.0.1:3000/sprite@2x.png");
        return test.response("sprite.png");
    };

    test.fileSource.imageResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/radar.gif", resource.url);
        return test.response("radar.gif");
    };

    test.fileSource.spriteJSONResponse = [&](const Resource&) {
        Response response;
        response.error = std::make_unique<Response::Error>(Response::Error::Reason::NotFound);
        return response;
    };

    test.fileSource.glyphsResponse = [&](const Resource&) {
        return test.response("glyph.pbf");
    };

    test.fileSource.sourceResponse = [&](const Resource& resource) {
        EXPECT_EQ("http://127.0.0.1:3000/streets.json", resource.url);
        return test.response("streets.json");
    };

    test.fileSource.tileResponse = [&](const Resource& resource) {
        const Resource::TileData& tile = *resource.tileData;
        EXPECT_EQ("http://127.0.0.1:3000/{z}-{x}-{y}.vector.pbf", tile.urlTemplate);
        EXPECT_EQ(1, tile.pixelRatio);
        EXPECT_EQ(0, tile.x);
        EXPECT_EQ(0, tile.y);
        EXPECT_EQ(0, tile.z);
        return test.response("0-0-0.vector.pbf");
    };

    auto observer = std::make_unique<MockObserver>();
    bool errorReported = false;
    observer->responseErrorFn = [&](Response::Error error) {
        EXPECT_EQ(Response::Error::Reason::NotFound, error.reason);
        errorReported = true;
    };
    observer->statusChangedFn = [&](OfflineRegionStatus status) {
        if (status.complete()) {
            EXPECT_TRUE(errorReported);
            test.loop.stop();
        }
    };

    download.setObserver(std::move(observer));
    download.setState(OfflineRegionDownloadState::Active);

    test.loop.run();
    // Passes if does not freeze.
}
