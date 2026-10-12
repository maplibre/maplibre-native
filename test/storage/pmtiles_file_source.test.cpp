#include <mln/storage/file_source.hpp>
#include <mln/storage/file_source_manager.hpp>
#include <mln/storage/pmtiles_file_source.hpp>
#include <mln/storage/resource.hpp>
#include <mln/storage/resource_options.hpp>
#include <mln/util/platform.hpp>
#include <mln/util/run_loop.hpp>

#include <atomic>
#include <filesystem>

#include <climits>
#include <gtest/gtest.h>

namespace {

std::string toAbsoluteURL(const std::string &fileName) {
    auto path = std::filesystem::current_path() / "test/fixtures/storage/pmtiles" / fileName;
    return std::string(mln::util::PMTILES_PROTOCOL) + std::string(mln::util::FILE_PROTOCOL) + path.string();
}

struct RangeRequestCounter {
    std::atomic<int> created{0};
    std::atomic<int> alive{0};
};

class CountedRequest : public mln::AsyncRequest {
public:
    CountedRequest(RangeRequestCounter& counter_, std::unique_ptr<mln::AsyncRequest> request_)
        : counter(counter_),
          request(std::move(request_)) {
        ++counter.created;
        ++counter.alive;
    }

    ~CountedRequest() override { --counter.alive; }

private:
    RangeRequestCounter& counter;
    std::unique_ptr<mln::AsyncRequest> request;
};

// Stands in for the resource loader PMTilesFileSource reads byte ranges through, to count the
// range requests that are alive. Answers from `fileSource`, or never when there is none.
class CountingFileSource : public mln::FileSource {
public:
    CountingFileSource(RangeRequestCounter& counter_, std::shared_ptr<mln::FileSource> fileSource_)
        : counter(counter_),
          fileSource(std::move(fileSource_)) {}

    std::unique_ptr<mln::AsyncRequest> request(const mln::Resource& resource, Callback callback) override {
        return std::make_unique<CountedRequest>(
            counter, fileSource ? fileSource->request(resource, std::move(callback)) : nullptr);
    }

    bool canRequest(const mln::Resource&) const override { return true; }
    void setResourceOptions(mln::ResourceOptions) override {}
    mln::ResourceOptions getResourceOptions() override { return {}; }
    void setClientOptions(mln::ClientOptions) override {}
    mln::ClientOptions getClientOptions() override { return {}; }

private:
    RangeRequestCounter& counter;
    std::shared_ptr<mln::FileSource> fileSource;
};

// Registers CountingFileSource as the resource loader, and puts the original back when destroyed
class ScopedCountingResourceLoader {
public:
    ScopedCountingResourceLoader(RangeRequestCounter& counter, std::shared_ptr<mln::FileSource> fileSource)
        : original(mln::FileSourceManager::get()->unRegisterFileSourceFactory(mln::FileSourceType::ResourceLoader)) {
        mln::FileSourceManager::get()->registerFileSourceFactory(
            mln::FileSourceType::ResourceLoader,
            [&counter, fileSource](const mln::ResourceOptions&, const mln::ClientOptions&) {
                return std::make_unique<CountingFileSource>(counter, fileSource);
            });
    }

    ~ScopedCountingResourceLoader() {
        mln::FileSourceManager::get()->registerFileSourceFactory(mln::FileSourceType::ResourceLoader,
                                                                 std::move(original));
    }

private:
    mln::FileSourceManager::FileSourceFactory original;
};

// FileSourceManager shares file sources between equal options, so use options no other test uses
// to be sure the PMTilesFileSource under test creates its resource loader from the factory above
mln::ResourceOptions rangeRequestTestOptions() {
    return mln::ResourceOptions::Default().withApiKey("PMTilesFileSource range request tests").clone();
}

// The file source answers from its own thread, in the order it was asked, so once this returns
// it has handled every request and cancellation sent before
void waitForFileSourceThread(mln::PMTilesFileSource& pmtiles) {
    pmtiles.getResourceOptions();
}

} // namespace

using namespace mln;

TEST(PMTilesFileSource, AcceptsURL) {
    PMTilesFileSource pmtiles(ResourceOptions::Default(), ClientOptions());
    EXPECT_TRUE(pmtiles.canRequest(Resource::style("pmtiles:///test")));
    EXPECT_FALSE(pmtiles.canRequest(Resource::style("pmtile://test")));
    EXPECT_FALSE(pmtiles.canRequest(Resource::style("pmtiles:")));
    EXPECT_FALSE(pmtiles.canRequest(Resource::style("")));
}

// Nonexistent pmtiles file raises error
TEST(PMTilesFileSource, NonExistentFile) {
    util::RunLoop loop;

    PMTilesFileSource pmtiles(ResourceOptions::Default(), ClientOptions());

    std::unique_ptr<AsyncRequest> req = pmtiles.request(
        {Resource::Unknown, toAbsoluteURL("does_not_exist")}, [&](Response res) {
            req.reset();
            ASSERT_NE(nullptr, res.error);
            EXPECT_EQ(Response::Error::Reason::NotFound, res.error->reason);
            EXPECT_NE((res.error->message).find("path not found"), std::string::npos);
            ASSERT_FALSE(res.data.get());
            loop.stop();
        });

    loop.run();
}

// Existing pmtiles file default request returns TileJSON
TEST(PMTilesFileSource, TileJSON) {
    util::RunLoop loop;

    PMTilesFileSource pmtiles(ResourceOptions::Default(), ClientOptions());

    std::unique_ptr<AsyncRequest> req = pmtiles.request(
        {Resource::Unknown, toAbsoluteURL("geography-class-png.pmtiles")}, [&](Response res) {
            req.reset();
            EXPECT_EQ(nullptr, res.error);
            ASSERT_TRUE(res.data.get());
            // basic test that TileJSON included a tile URL
            EXPECT_NE((*res.data).find("geography-class-png.pmtiles"), std::string::npos);
            loop.stop();
        });

    loop.run();
}

// Existing tiles return tile data
TEST(PMTilesFileSource, Tile) {
    util::RunLoop loop;

    PMTilesFileSource pmtiles(ResourceOptions::Default(), ClientOptions());

    std::unique_ptr<AsyncRequest> req = pmtiles.request(
        Resource::tile(toAbsoluteURL("geography-class-png.pmtiles"), 1.0, 0, 0, 0, Tileset::Scheme::XYZ),
        [&](Response res) {
            req.reset();
            EXPECT_EQ(nullptr, res.error);
            ASSERT_TRUE(res.data.get());
            ASSERT_EQ(res.noContent, false);
            loop.stop();
        });

    loop.run();
}

// Nonexistent tiles do not raise errors, they simply return no content
TEST(PMTilesFileSource, NonExistentTile) {
    util::RunLoop loop;

    PMTilesFileSource pmtiles(ResourceOptions::Default(), ClientOptions());

    std::unique_ptr<AsyncRequest> req = pmtiles.request(
        Resource::tile(toAbsoluteURL("geography-class-png.pmtiles"), 1.0, 0, 0, 4, Tileset::Scheme::XYZ),
        [&](Response res) {
            req.reset();
            EXPECT_EQ(nullptr, res.error);
            ASSERT_FALSE(res.data.get());
            ASSERT_EQ(res.noContent, true);
            loop.stop();
        });

    loop.run();
}

// A tile whose bytes start with gzip magic but are otherwise corrupt must yield an error
// response — the decompression failure must not propagate as an exception.
TEST(PMTilesFileSource, CorruptGzipTile) {
    util::RunLoop loop;

    PMTilesFileSource pmtiles(ResourceOptions::Default(), ClientOptions());

    std::unique_ptr<AsyncRequest> req = pmtiles.request(
        Resource::tile(toAbsoluteURL("corrupt-gzip-tile.pmtiles"), 1.0, 0, 0, 0, Tileset::Scheme::XYZ),
        [&](Response res) {
            req.reset();
            ASSERT_NE(nullptr, res.error);
            EXPECT_EQ(Response::Error::Reason::Other, res.error->reason);
            EXPECT_NE(res.error->message.find("Error decompressing PMTiles tile:"), std::string::npos);
            loop.stop();
        });

    loop.run();
}

// Metadata whose bytes start with gzip magic but are otherwise corrupt must yield an error
// response — the decompression failure must not propagate as an exception. Unlike the tile path,
// this one runs on the PMTiles file source thread with no handler above it, so an escaping
// exception calls std::terminate and aborts the process.
TEST(PMTilesFileSource, CorruptGzipMetadata) {
    util::RunLoop loop;

    PMTilesFileSource pmtiles(ResourceOptions::Default(), ClientOptions());

    std::unique_ptr<AsyncRequest> req = pmtiles.request(
        {Resource::Unknown, toAbsoluteURL("corrupt-gzip-metadata.pmtiles")}, [&](Response res) {
            req.reset();
            ASSERT_NE(nullptr, res.error);
            EXPECT_EQ(Response::Error::Reason::Other, res.error->reason);
            EXPECT_NE(res.error->message.find("Error decompressing PMTiles metadata:"), std::string::npos);
            loop.stop();
        });

    loop.run();
}

// Regression test for https://github.com/maplibre/maplibre-native/issues/3690 — the loading
// method for PMTiles sub-requests changed from `Network` to `All` so that http(s)-hosted
// archives flow through the ambient cache. The file:// path bypasses the cache by design,
// so this test can only confirm that issuing the same tile twice still succeeds; a true
// cache-hit assertion requires an http(s) PMTiles fixture and a mock server.
TEST(PMTilesFileSource, RepeatedTileRequest) {
    util::RunLoop loop;

    PMTilesFileSource pmtiles(ResourceOptions::Default(), ClientOptions());

    std::unique_ptr<AsyncRequest> req1;
    std::unique_ptr<AsyncRequest> req2;

    req1 = pmtiles.request(
        Resource::tile(toAbsoluteURL("geography-class-png.pmtiles"), 1.0, 0, 0, 0, Tileset::Scheme::XYZ),
        [&](Response res1) {
            req1.reset();
            ASSERT_EQ(nullptr, res1.error);
            ASSERT_TRUE(res1.data.get());
            const auto firstSize = res1.data->size();

            req2 = pmtiles.request(
                Resource::tile(toAbsoluteURL("geography-class-png.pmtiles"), 1.0, 0, 0, 0, Tileset::Scheme::XYZ),
                [&, firstSize](Response res2) {
                    req2.reset();
                    EXPECT_EQ(nullptr, res2.error);
                    ASSERT_TRUE(res2.data.get());
                    EXPECT_EQ(firstSize, res2.data->size());
                    loop.stop();
                });
        });

    loop.run();
}

// An archive whose header flags tile_compression=GZIP but whose individual tiles are stored
// without compression must be served without error (uncompressed bytes passed through as-is).
TEST(PMTilesFileSource, UncompressedTile) {
    util::RunLoop loop;

    PMTilesFileSource pmtiles(ResourceOptions::Default(), ClientOptions());

    std::unique_ptr<AsyncRequest> req = pmtiles.request(
        Resource::tile(toAbsoluteURL("uncompressed-tiles.pmtiles"), 1.0, 0, 0, 0, Tileset::Scheme::XYZ),
        [&](Response res) {
            req.reset();
            EXPECT_EQ(nullptr, res.error);
            ASSERT_TRUE(res.data.get());
            ASSERT_EQ(res.noContent, false);
            loop.stop();
        });

    loop.run();
}

// The range request made for a tile must not outlive the tile request: the resource loader keeps
// revalidating a request for as long as it exists.
TEST(PMTilesFileSource, TileRequestReleasesRangeRequest) {
    util::RunLoop loop;

    RangeRequestCounter counter;
    ScopedCountingResourceLoader resourceLoader(
        counter, FileSourceManager::get()->getFileSource(FileSourceType::FileSystem, ResourceOptions::Default()));

    PMTilesFileSource pmtiles(rangeRequestTestOptions(), ClientOptions());

    std::unique_ptr<AsyncRequest> req = pmtiles.request(
        Resource::tile(toAbsoluteURL("geography-class-png.pmtiles"), 1.0, 0, 0, 0, Tileset::Scheme::XYZ),
        [&](Response res) {
            req.reset();
            EXPECT_EQ(nullptr, res.error);
            ASSERT_TRUE(res.data.get());
            loop.stop();
        });

    loop.run();

    waitForFileSourceThread(pmtiles);

    // header, root directory and tile
    EXPECT_EQ(3, counter.created);
    EXPECT_EQ(0, counter.alive);
}

// Cancelling a tile request must cancel the range request it is waiting for
TEST(PMTilesFileSource, CancelledTileRequestCancelsRangeRequest) {
    util::RunLoop loop;

    RangeRequestCounter counter;
    ScopedCountingResourceLoader resourceLoader(counter, nullptr);

    PMTilesFileSource pmtiles(rangeRequestTestOptions(), ClientOptions());

    std::unique_ptr<AsyncRequest> req = pmtiles.request(
        Resource::tile(toAbsoluteURL("geography-class-png.pmtiles"), 1.0, 0, 0, 0, Tileset::Scheme::XYZ),
        [&](Response) { ADD_FAILURE() << "the range request is never answered"; });

    waitForFileSourceThread(pmtiles);

    // the header request, left unanswered
    ASSERT_EQ(1, counter.created);
    ASSERT_EQ(1, counter.alive);

    req.reset();

    waitForFileSourceThread(pmtiles);

    EXPECT_EQ(1, counter.created);
    EXPECT_EQ(0, counter.alive);
}
