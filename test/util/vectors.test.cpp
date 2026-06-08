#include <mbgl/test/util.hpp>
#include <mbgl/util/vectors.hpp>

#include <gtest/gtest.h>

using namespace mbgl;
using namespace mbgl::vector;

TEST(Vectors, ConcatenateArrayAndScalars) {
    EXPECT_EQ((vec4{{1.0, 2.0, 3.0, 4.0}}), vec(1.0, 2.0, 3.0, 4.0));
    EXPECT_EQ((vec4{{1.0, 2.0, 3.0, 4.0}}), vec(vec2{{1.0, 2.0}}, 3.0, 4.0));
    EXPECT_EQ((vec4{{1.0, 2.0, 3.0, 4.0}}), vec(vec3{{1.0, 2.0, 3.0}}, 4.0));
}

TEST(Vectors, Slice) {
    EXPECT_EQ((vec2{1.0, 2.0}), (slice<0, 2>(vec4{1.0, 2.0, 3.0, 4.0})));
    EXPECT_EQ((vec2{{3.0, 4.0}}), (mbgl::slice<2, 2>(vec4{{1.0, 2.0, 3.0, 4.0}})));
    EXPECT_EQ((vec2{{2.0, 3.0}}), (slice<1, 2>(vec4{{1.0, 2.0, 3.0, 4.0}})));
    EXPECT_EQ((vec3{{1.0, 2.0, 3.0}}), (slice<0, 3>(vec4{{1.0, 2.0, 3.0, 4.0}})));
    EXPECT_EQ((vec3{{2.0, 3.0, 4.0}}), (slice<1, 3>(vec4{{1.0, 2.0, 3.0, 4.0}})));
    EXPECT_EQ((vec4{{1.0, 2.0, 3.0, 4.0}}), (slice<0, 4>(vec4{{1.0, 2.0, 3.0, 4.0}})));
    EXPECT_EQ((std::array<double, 1>{{4.0}}), (slice<3, 1>(vec4{{1.0, 2.0, 3.0, 4.0}})));
}
