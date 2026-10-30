#include <mln/storage/mbtiles_file_source.hpp>
#include <mln/storage/resource.hpp>
#include <mln/storage/resource_options.hpp>
#include <mln/util/accept_header.hpp>
#include <mln/util/platform.hpp>
#include <mln/util/run_loop.hpp>

#include <filesystem>

#include <climits>
#include <gtest/gtest.h>

namespace {

std::string toAbsoluteURL(const std::string &fileName) {
    auto path = std::filesystem::current_path() / "test/fixtures/storage/mbtiles" / fileName;
    return "mbtiles://" + path.string();
}

} // namespace

using namespace mln;

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

namespace {

// Requests the TileJSON of an MBTiles fixture and returns its content.
std::string requestTileJSON(const std::string &fileName) {
    util::RunLoop loop;
    std::string tileJSON;

    MBTilesFileSource mbtiles(ResourceOptions::Default(), ClientOptions());

    std::unique_ptr<AsyncRequest> req = mbtiles.request({Resource::Unknown, toAbsoluteURL(fileName)},
                                                        [&](Response res) {
                                                            req.reset();
                                                            EXPECT_EQ(nullptr, res.error);
                                                            ASSERT_TRUE(res.data.get());
                                                            tileJSON = *res.data;
                                                            loop.stop();
                                                        });

    loop.run();
    return tileJSON;
}

} // namespace

// A MapLibre Tile media type in the metadata is translated to the `mlt` source encoding
TEST(MBTilesFileSource, TileJSONMLTMediaType) {
    const auto tileJSON = requestTileJSON("mlt-media-type.mbtiles");
    EXPECT_NE(tileJSON.find(R"("format":")" + std::string(http::mltMediaType) + R"(")"), std::string::npos);
    EXPECT_NE(tileJSON.find(R"("encoding":"mlt")"), std::string::npos);
}

// Files written by older versions of Planetiler use the previous MapLibre Tile media type
TEST(MBTilesFileSource, TileJSONLegacyMLTMediaType) {
    const auto tileJSON = requestTileJSON("mlt-legacy-media-type.mbtiles");
    EXPECT_NE(tileJSON.find(R"("format":"application/vnd.maplibre-vector-tile")"), std::string::npos);
    EXPECT_NE(tileJSON.find(R"("encoding":"mlt")"), std::string::npos);
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
