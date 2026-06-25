#include <mbgl/test/util.hpp>

#include <mbgl/map/transform.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/mat4.hpp>

using namespace mbgl;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static TransformParameters makeParams(double zoom, double pitch = 0.0) {
    Transform transform;
    transform.resize({512, 512});
    transform.jumpTo(CameraOptions()
                         .withCenter(LatLng{37.7749, -122.4194})
                         .withZoom(zoom)
                         .withPitch(pitch));
    return TransformParameters(transform.getState());
}

// ---------------------------------------------------------------------------
// TransformParameters::nearClippedProjMatrix
// ---------------------------------------------------------------------------

// The near-clipped matrix must be populated — it must not be the identity.
TEST(TransformParameters, NearClippedMatrixIsNonIdentity) {
    const auto p = makeParams(15.0, 45.0);
    EXPECT_NE(p.nearClippedProjMatrix, matrix::identity4());
}

// The two matrices must differ: they use different near clip planes so their
// depth-mapping coefficients (column-major indices 10 and 14) are different.
TEST(TransformParameters, NearClippedMatrixDiffersFromProjMatrix) {
    const auto p = makeParams(15.0, 45.0);
    EXPECT_NE(p.projMatrix, p.nearClippedProjMatrix);
    // m[10] = -(f+n)/(f-n), m[14] = -2fn/(f-n) — both near-dependent.
    EXPECT_NE(p.projMatrix[10], p.nearClippedProjMatrix[10]);
    EXPECT_NE(p.projMatrix[14], p.nearClippedProjMatrix[14]);
}

// The XY perspective columns must be identical: viewport and FOV are shared.
// Column 0 = indices 0-3, column 1 = indices 4-7 (column-major order).
TEST(TransformParameters, NearClippedMatrixSharesXYPerspective) {
    const auto p = makeParams(15.0, 45.0);
    for (int i = 0; i < 8; ++i) {
        EXPECT_DOUBLE_EQ(p.projMatrix[i], p.nearClippedProjMatrix[i])
            << "XY perspective mismatch at index " << i;
    }
}

// The near-clipped near plane is ~10% of camera-to-center distance, so its
// effective near-Z is always strictly greater than 1 (the standard near plane).
// This shows up as a smaller magnitude at m[10] (less negative for typical FOV).
TEST(TransformParameters, NearClippedMatrixHasSmallerDepthRangeAtZoom15) {
    const auto p = makeParams(15.0, 0.0);
    // |m[10]_nearClipped| < |m[10]_standard| because the near plane is farther
    // away, which compresses the depth range less aggressively.
    EXPECT_LT(std::abs(p.nearClippedProjMatrix[10]), std::abs(p.projMatrix[10]));
}

// The property must hold across a range of zoom levels.
TEST(TransformParameters, NearClippedMatrixDiffersAtVariousZooms) {
    for (double zoom : {5.0, 10.0, 15.0, 17.0}) {
        const auto p = makeParams(zoom, 30.0);
        EXPECT_NE(p.projMatrix[10], p.nearClippedProjMatrix[10])
            << "matrices identical at zoom " << zoom;
        EXPECT_NE(p.projMatrix[14], p.nearClippedProjMatrix[14])
            << "matrices identical at zoom " << zoom;
    }
}
