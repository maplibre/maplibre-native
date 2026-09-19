#include <gtest/gtest.h>

#include <mln/util/mat3.hpp>

#include <cmath>

using namespace mln;

namespace {

// Rotating a vector about z must not change its length in the xy plane.
float lengthXY(const vec3f& v) {
    return std::hypot(v[0], v[1]);
}

} // namespace

TEST(Mat3, TransformMat3fAliased) {
    // The only caller in the tree passes the same vector as both output and input
    // (FillExtrusionBucket::lightPosition), so writing out[0] before reading a[0] leaves
    // out[1] computed from the transformed x rather than the original.
    mat3 rotation;
    matrix::identity(rotation);
    matrix::rotate(rotation, rotation, -M_PI * 3.0 / 4.0);

    const vec3f in = {{0.2875f, -0.497964f, 0.995929f}};

    vec3f separate;
    matrix::transformMat3f(separate, in, rotation);

    vec3f aliased = in;
    matrix::transformMat3f(aliased, aliased, rotation);

    EXPECT_FLOAT_EQ(separate[0], aliased[0]);
    EXPECT_FLOAT_EQ(separate[1], aliased[1]);
    EXPECT_FLOAT_EQ(separate[2], aliased[2]);
}

TEST(Mat3, TransformMat3fRotationPreservesLength) {
    // A rotation about z leaves |xy| and z alone. Before the aliasing fix, a quarter turn of
    // this vector reported |xy| = 0.704 where the input is 0.575.
    const vec3f in = {{0.2875f, -0.497964f, 0.995929f}};

    for (int degrees = 0; degrees < 360; degrees += 15) {
        mat3 rotation;
        matrix::identity(rotation);
        matrix::rotate(rotation, rotation, degrees * M_PI / 180.0);

        vec3f out = in;
        matrix::transformMat3f(out, out, rotation);

        EXPECT_NEAR(lengthXY(in), lengthXY(out), 1e-5f) << "at " << degrees << " degrees";
        EXPECT_FLOAT_EQ(in[2], out[2]) << "at " << degrees << " degrees";
    }
}
