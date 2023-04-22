#include <mbgl/test/util.hpp>

#include <mbgl/util/tile_server_options.hpp>

TEST(TileServerOptions, CopyAssignment) {
    mbgl::TileServerOptions options = mbgl::TileServerOptions::MapboxConfiguration();
    mbgl::TileServerOptions optionsCopy = options;

    EXPECT_FALSE(&optionsCopy == &options);
    EXPECT_EQ(optionsCopy.baseURL(), options.baseURL());
}

TEST(TileServerOptions, CopyConstructor) {
    mbgl::TileServerOptions options = mbgl::TileServerOptions::MapboxConfiguration();
    mbgl::TileServerOptions optionsCopy = mbgl::TileServerOptions(options);

    EXPECT_FALSE(&optionsCopy == &options);
    EXPECT_EQ(optionsCopy.baseURL(), options.baseURL());
}
