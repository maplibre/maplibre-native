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
