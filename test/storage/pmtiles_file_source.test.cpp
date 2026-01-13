#include <mbgl/storage/pmtiles_file_source.hpp>
#include <mbgl/storage/resource.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/util/platform.hpp>
#include <mbgl/util/run_loop.hpp>

#include <filesystem>

#include <climits>
#include <gtest/gtest.h>

namespace {

std::string toAbsoluteURL(const std::string &fileName) {
    auto path = std::filesystem::current_path() / "test/fixtures/storage/pmtiles" / fileName;
    return std::string(mbgl::util::PMTILES_PROTOCOL) + std::string(mbgl::util::FILE_PROTOCOL) + path.string();
}

} // namespace

using namespace mbgl;

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

// Test that replacing a PMTiles file on disk while cached doesn't cause a crash
// This reproduces a bug where stale cached header/directory data causes invalid
// byte range requests after the underlying file is replaced with a different PMTiles file
TEST(PMTilesFileSource, FileReplacementWhileCached) {
    util::RunLoop loop;

    // Create a temporary file path for the test
    auto tempDir = std::filesystem::temp_directory_path();
    auto tempPmtilesPath = tempDir / "pmtiles_replacement_test.pmtiles";
    auto fixturesPath = std::filesystem::current_path() / "test/fixtures/storage/pmtiles";

    // Copy the first PMTiles file to the temp location
    std::filesystem::copy_file(
        fixturesPath / "pmt_test.pmtiles", tempPmtilesPath, std::filesystem::copy_options::overwrite_existing);

    std::string tempUrl = std::string(mbgl::util::PMTILES_PROTOCOL) + std::string(mbgl::util::FILE_PROTOCOL) +
                          tempPmtilesPath.string();

    PMTilesFileSource pmtiles(ResourceOptions::Default(), ClientOptions());

    int requestCount = 0;

    // First, request TileJSON to cache the header and metadata
    std::unique_ptr<AsyncRequest> req = pmtiles.request(
        {Resource::Unknown, tempUrl}, [&, tempUrl, fixturesPath, tempPmtilesPath](Response res) {
            req.reset();
            EXPECT_EQ(nullptr, res.error);
            ASSERT_TRUE(res.data.get());

            // Request multiple tiles at different zoom levels to populate cache thoroughly
            // This ensures the directory is cached
            auto tilesToRequest = std::make_shared<std::vector<std::tuple<int, int, int>>>(
                std::vector<std::tuple<int, int, int>>{
                    {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {1, 1, 1}, {2, 0, 0}, {2, 1, 1}, {2, 2, 2}, {2, 3, 3}});

            // Use shared_ptr to keep the recursive lambda alive
            auto requestNextTile = std::make_shared<std::function<void(size_t)>>();
            *requestNextTile = [&, tilesToRequest, requestNextTile, tempUrl, fixturesPath, tempPmtilesPath](
                                   size_t index) {
                if (index >= tilesToRequest->size()) {
                    // All initial tiles requested, now replace the file
                    std::filesystem::copy_file(fixturesPath / "pmt_test_2.pmtiles",
                                               tempPmtilesPath,
                                               std::filesystem::copy_options::overwrite_existing);

                    // Request the same tiles again with stale cache
                    // This should expose issues with cached offsets being invalid for the new file
                    requestCount = 0;
                    auto requestWithStaleCache = std::make_shared<std::function<void(size_t)>>();
                    *requestWithStaleCache =
                        [&, tilesToRequest, requestWithStaleCache, tempUrl, fixturesPath, tempPmtilesPath](size_t idx) {
                            if (idx >= tilesToRequest->size()) {
                                // Also try some high zoom level tiles that might not exist
                                req = pmtiles.request(Resource::tile(tempUrl, 1.0, 100, 200, 10, Tileset::Scheme::XYZ),
                                                      [&](Response finalRes) {
                                                          req.reset();
                                                          // The request may fail or succeed, but should not crash
                                                          // With the bug, invalid byte ranges from stale cache would
                                                          // crash here
                                                          (void)finalRes;

                                                          // Clean up temp file
                                                          std::filesystem::remove(tempPmtilesPath);
                                                          loop.stop();
                                                      });
                                return;
                            }

                            auto [z, x, y] = (*tilesToRequest)[idx];
                            req = pmtiles.request(Resource::tile(tempUrl, 1.0, x, y, z, Tileset::Scheme::XYZ),
                                                  [&, idx, requestWithStaleCache](Response tileRes) {
                                                      req.reset();
                                                      requestCount++;
                                                      // Note: We expect some of these to fail or return no content
                                                      // but they should not crash
                                                      (void)tileRes;
                                                      (*requestWithStaleCache)(idx + 1);
                                                  });
                        };
                    (*requestWithStaleCache)(0);
                    return;
                }

                auto [z, x, y] = (*tilesToRequest)[index];
                req = pmtiles.request(Resource::tile(tempUrl, 1.0, x, y, z, Tileset::Scheme::XYZ),
                                      [&, index, requestNextTile](Response tileRes) {
                                          req.reset();
                                          (void)tileRes;
                                          (*requestNextTile)(index + 1);
                                      });
            };

            (*requestNextTile)(0);
        });

    loop.run();
}
