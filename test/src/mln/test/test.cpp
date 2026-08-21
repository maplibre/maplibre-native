#include <mln/actor/scheduler.hpp>
#include <mln/test.hpp>
#include <mln/test/util.hpp>

#include <gtest/gtest.h>

namespace mln {

int runTests(int argc, char *argv[]) {
#if (TEST_HAS_SERVER && defined(__QT__)) || USE_CPP_TEST_SERVER
    auto server = std::make_unique<test::HttpServer>();
#endif

    testing::InitGoogleTest(&argc, argv);

    // In order to run specific tests
    // testing::GTEST_FLAG(filter) = "TileServerOptions*";
    // testing::GTEST_FLAG(filter) = "MainResourceLoader.ResourceOptions";

    return RUN_ALL_TESTS();
}

} // namespace mln
