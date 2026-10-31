#include <mln/actor/scheduler.hpp>
#include <mln/test.hpp>
#include <mln/test/util.hpp>
#include <mln/util/constants.hpp>

#include <gtest/gtest.h>

#include <string>

namespace mln {

int runTests(int argc, char *argv[]) {
#if (TEST_HAS_SERVER && defined(__QT__)) || USE_CPP_TEST_SERVER
    auto server = std::make_unique<test::HttpServer>();
#endif

    testing::InitGoogleTest(&argc, argv);

    // A build without the globe (`MLN_WITH_GLOBE`) draws a globe projection as Mercator and never subdivides, which
    // the globe's tests don't expect.
    if constexpr (!util::GLOBE_ENABLED) {
        std::string filter = testing::GTEST_FLAG(filter);
        filter += filter.find('-') == std::string::npos ? "-" : ":";
        testing::GTEST_FLAG(filter) = filter + "*Globe*:VerticalPerspective*:Subdivision.*";
    }

    // In order to run specific tests
    // testing::GTEST_FLAG(filter) = "TileServerOptions*";
    // testing::GTEST_FLAG(filter) = "MainResourceLoader.ResourceOptions";

    return RUN_ALL_TESTS();
}

} // namespace mln
