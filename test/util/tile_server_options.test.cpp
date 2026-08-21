#include <mln/test/util.hpp>

#include <mln/util/tile_server_options.hpp>

TEST(TileServerOptions, CopyAssignment) {
    mln::TileServerOptions options = mln::TileServerOptions::MapboxConfiguration();
    mln::TileServerOptions optionsCopy = options;

    EXPECT_FALSE(&optionsCopy == &options);
    EXPECT_EQ(optionsCopy.baseURL(), options.baseURL());
}

TEST(TileServerOptions, CopyConstructor) {
    mln::TileServerOptions options = mln::TileServerOptions::MapboxConfiguration();
    mln::TileServerOptions optionsCopy = mln::TileServerOptions(options);

    EXPECT_FALSE(&optionsCopy == &options);
    EXPECT_EQ(optionsCopy.baseURL(), options.baseURL());
}
