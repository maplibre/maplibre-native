#include <mbgl/test/util.hpp>

#include <mbgl/map/transform.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/util/mat4.hpp>

using namespace mbgl;

// Build two projection matrices from the same TransformState, mirroring what
// TransformParameters does:
//   projMatrix          → nearZ = 1 (standard)
//   nearClippedProjMatrix → nearZ = 10% of camera-to-center distance (fill-extrusion z-space)
struct Matrices {
    mat4 proj;
    mat4 nearClipped;
};

static Matrices makeMatrices(double zoom, double pitch = 0.0) {
    Transform transform;
    transform.resize({512, 512});
    transform.jumpTo(CameraOptions()
                         .withCenter(LatLng{37.7749, -122.4194})
                         .withZoom(zoom)
                         .withPitch(pitch));
    const TransformState& state = transform.getState();

    Matrices m;
    state.getProjMatrix(m.proj);
    state.getProjMatrix(m.nearClipped,
                        static_cast<uint16_t>(0.1 * state.getCameraToCenterDistance()));
    return m;
}

// ---------------------------------------------------------------------------
// nearClippedProjMatrix tests (issue #4301)
// ---------------------------------------------------------------------------

// The near-clipped matrix must be populated — not the identity.
TEST(TransformParameters, NearClippedMatrixIsNonIdentity) {
    const auto m = makeMatrices(15.0, 45.0);
    EXPECT_NE(m.nearClipped, matrix::identity4());
}

// The two matrices must differ: different near planes → different depth coefficients.
TEST(TransformParameters, NearClippedMatrixDiffersFromProjMatrix) {
    const auto m = makeMatrices(15.0, 45.0);
    EXPECT_NE(m.proj, m.nearClipped);
    // Column-major: m[10] = -(f+n)/(f-n), m[14] = -2fn/(f-n) — both near-dependent.
    EXPECT_NE(m.proj[10], m.nearClipped[10]);
    EXPECT_NE(m.proj[14], m.nearClipped[14]);
}

// The XY perspective columns must be identical — viewport and FOV are shared.
// Column 0 = indices 0-3, column 1 = indices 4-7 (column-major).
TEST(TransformParameters, NearClippedMatrixSharesXYPerspective) {
    const auto m = makeMatrices(15.0, 45.0);
    for (int i = 0; i < 8; ++i) {
        EXPECT_DOUBLE_EQ(m.proj[i], m.nearClipped[i])
            << "XY perspective mismatch at index " << i;
    }
}

// Raising the near plane shrinks the depth range, reflected in a smaller |m[10]|.
TEST(TransformParameters, NearClippedMatrixHasSmallerDepthRange) {
    const auto m = makeMatrices(15.0, 0.0);
    EXPECT_LT(std::abs(m.nearClipped[10]), std::abs(m.proj[10]));
}

// The invariant must hold across a range of zoom levels.
TEST(TransformParameters, NearClippedMatrixDiffersAtVariousZooms) {
    for (double zoom : {5.0, 10.0, 15.0, 17.0}) {
        const auto m = makeMatrices(zoom, 30.0);
        EXPECT_NE(m.proj[10], m.nearClipped[10]) << "matrices identical at zoom " << zoom;
        EXPECT_NE(m.proj[14], m.nearClipped[14]) << "matrices identical at zoom " << zoom;
    }
}
