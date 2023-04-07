#include <mbgl/actor/scheduler.hpp>
#include <mbgl/test.hpp>
#include <mbgl/test/util.hpp>

#include <gtest/gtest.h>

namespace mbgl {

int runTests(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    
    // In order to run specific tests
    // testing::GTEST_FLAG(filter) = "TileServerOptions*";
    // testing::GTEST_FLAG(filter) = "MainResourceLoader.ResourceOptions";

    return RUN_ALL_TESTS();
}

} // namespace mbgl
