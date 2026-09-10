#include <mln/util/run_loop.hpp>
#include <gtest/gtest.h>

int main(int argc, char** argv) {
    mln::util::RunLoop loop;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
