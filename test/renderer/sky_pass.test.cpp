#include <mln/test/util.hpp>

#include <mln/map/camera.hpp>
#include <mln/map/transform.hpp>
#include <mln/map/vertical_perspective_projection.hpp>
#include <mln/renderer/sky_pass.hpp>
#include <mln/style/projection_definition.hpp>
#include <mln/util/mat4.hpp>

#include <array>

using namespace mln;

TEST(SkyPass, MapLightTurnsWithTheGlobe) {
    std::array<float, 3> spherical = {{1.15f, 210.0f, 30.0f}};
    EvaluatedLight light;
    light.get<style::LightAnchor>() = style::LightAnchorType::Map;
    light.get<style::LightPosition>() = style::Position(spherical);
    const auto position = light.get<style::LightPosition>().getCartesian();

    Transform transform;
    transform.resize({800, 600});
    transform.setProjectionDefinition(ProjectionDefinition("vertical-perspective"));
    transform.jumpTo(
        CameraOptions().withCenter(LatLng{35.0, 120.0}).withZoom(2.0).withBearing(30.0).withPitch(40.0).withRoll(10.0));

    for (const auto orientation : {NorthOrientation::Upwards,
                                   NorthOrientation::Rightwards,
                                   NorthOrientation::Downwards,
                                   NorthOrientation::Leftwards}) {
        transform.setNorthOrientation(orientation);
        const TransformState& state = transform.getState();
        const double radius = state.getGlobeRadiusPixels();

        mat4 inverseProjection;
        ASSERT_FALSE(
            matrix::invert(inverseProjection, VerticalPerspectiveProjection::globeProjectionMatrix(state, radius)));
        mat4 globeToView;
        matrix::multiply(
            globeToView, inverseProjection, VerticalPerspectiveProjection::globeViewProjectionMatrix(state, radius));
        vec4 expected = {{-position[0] / radius, -position[1] / radius, -position[2] / radius, 0.0}};
        matrix::transformMat4(expected, expected, globeToView);

        const auto sun = SkyPass::sunPosition(light, state, 0.5f);
        for (std::size_t i = 0; i < 3; ++i) {
            EXPECT_NEAR(expected[i], sun[i], 1e-6) << "orientation " << static_cast<int>(orientation) << ", axis " << i;
        }
        EXPECT_EQ(0.5f, sun[3]);
    }
}
