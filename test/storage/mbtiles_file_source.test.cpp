#include <mbgl/storage/mbtiles_file_source.hpp>
#include <mbgl/storage/resource.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/util/platform.hpp>
#include <mbgl/util/run_loop.hpp>

#include <climits>
#include <gtest/gtest.h>

#if defined(WIN32)
#include <Windows.h>
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif /* PATH_MAX */
#else
#include <unistd.h>
#endif

namespace {

std::string toAbsoluteURL(const std::string &fileName) {
    char buff[PATH_MAX + 1];
#ifdef _MSC_VER
    char *cwd = _getcwd(buff, PATH_MAX + 1);
#else
    char *cwd = getcwd(buff, PATH_MAX + 1);
#endif
    std::string url = {"mbtiles://" + std::string(cwd) + "/test/fixtures/storage/mbtiles/" + fileName};
    assert(url.size() <= PATH_MAX);
    return url;
}

} // namespace

using namespace mbgl;

TEST(MBTilesFileSource, AcceptsURL) {
    MBTilesFileSource mbtiles(ResourceOptions::Default(), ClientOptions());
    EXPECT_TRUE(mbtiles.canRequest(Resource::style("mbtiles:///test")));
    EXPECT_FALSE(mbtiles.canRequest(Resource::style("mbtile://test")));
    EXPECT_FALSE(mbtiles.canRequest(Resource::style("mbtiles:")));
    EXPECT_FALSE(mbtiles.canRequest(Resource::style("")));
}

// mbtiles paths must be absolute
TEST(MBTilesFileSource, AbsolutePath) {
    util::RunLoop loop;

    MBTilesFileSource mbtiles(ResourceOptions::Default(), ClientOptions());

    std::unique_ptr<AsyncRequest> req = mbtiles.request(
        {Resource::Unknown, "mbtiles://not_absolute"}, [&](Response res) {
            req.reset();
            ASSERT_NE(nullptr, res.error);
            EXPECT_EQ(Response::Error::Reason::Other, res.error->reason);
            EXPECT_NE((res.error->message).find("absolute"), std::string::npos);
            ASSERT_FALSE(res.data.get());
            loop.stop();
        });

    loop.run();
}

// Nonexistent mbtiles file raises error
TEST(MBTilesFileSource, NonExistentFile) {
    util::RunLoop loop;

    MBTilesFileSource mbtiles(ResourceOptions::Default(), ClientOptions());

    std::unique_ptr<AsyncRequest> req = mbtiles.request(
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

// Existing mbtiles file default request returns TileJSON
TEST(MBTilesFileSource, TileJSON) {
    util::RunLoop loop;

    MBTilesFileSource mbtiles(ResourceOptions::Default(), ClientOptions());

    std::unique_ptr<AsyncRequest> req = mbtiles.request(
        {Resource::Unknown, toAbsoluteURL("geography-class-png.mbtiles")}, [&](Response res) {
            req.reset();
            EXPECT_EQ(nullptr, res.error);
            ASSERT_TRUE(res.data.get());
            // basic test that TileJSON included a tile URL
            EXPECT_NE((*res.data).find("geography-class-png.mbtiles?file={x}/{y}/{z}"), std::string::npos);
            loop.stop();
        });

    loop.run();
}

// Existing tiles return tile data
TEST(MBTilesFileSource, Tile) {
    util::RunLoop loop;

    MBTilesFileSource mbtiles(ResourceOptions::Default(), ClientOptions());

    std::unique_ptr<AsyncRequest> req = mbtiles.request(
        Resource::tile(
            toAbsoluteURL("geography-class-png.mbtiles?file={z}/{x}/{y}.png"), 1.0, 0, 0, 0, Tileset::Scheme::XYZ),
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
TEST(MBTilesFileSource, NonExistentTile) {
    util::RunLoop loop;

    MBTilesFileSource mbtiles(ResourceOptions::Default(), ClientOptions());

    std::unique_ptr<AsyncRequest> req = mbtiles.request(
        Resource::tile(
            toAbsoluteURL("geography-class-png.mbtiles?file={z}/{x}/{y}.png"), 1.0, 0, 0, 4, Tileset::Scheme::XYZ),
        [&](Response res) {
            req.reset();
            EXPECT_EQ(nullptr, res.error);
            ASSERT_FALSE(res.data.get());
            ASSERT_EQ(res.noContent, true);
            loop.stop();
        });

    loop.run();
}
