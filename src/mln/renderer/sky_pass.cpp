#include <mln/renderer/sky_pass.hpp>

#include <mln/gfx/color_mode.hpp>
#include <mln/gfx/context.hpp>
#include <mln/gfx/cull_face_mode.hpp>
#include <mln/gfx/drawable_builder.hpp>
#include <mln/gfx/projection_variant.hpp>
#include <mln/gfx/shader_registry.hpp>
#include <mln/map/transform_state.hpp>
#include <mln/map/vertical_perspective_projection.hpp>
#include <mln/renderer/render_pass.hpp>
#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/sky_ubo.hpp>
#include <mln/style/light_impl.hpp>
#include <mln/style/sky_impl.hpp>
#include <mln/util/convert.hpp>
#include <mln/util/mat4.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>
#include <string_view>

namespace mln {

namespace {

constexpr std::string_view SkyShaderName = "SkyShader";
constexpr std::string_view AtmosphereShaderName = "AtmosphereShader";

LayerGroupPtr makeFullscreenLayerGroup(gfx::Context& context,
                                       const gfx::ShaderProgramBasePtr& shader,
                                       std::string_view name,
                                       std::size_t vertexAttribute,
                                       bool is3D,
                                       gfx::DepthMaskType depthMask) {
    if (!shader) {
        return nullptr;
    }

    auto layerGroup = context.createLayerGroup(0, 1, std::string(name));
    if (!layerGroup) {
        return nullptr;
    }

    auto builder = context.createDrawableBuilder(std::string(name));
    builder->setShader(shader);
    builder->setRenderPass(RenderPass::Translucent);
    builder->setIs3D(is3D);
    builder->setDepthType(depthMask);
    builder->setEnableDepth(true);
    builder->setEnableStencil(false);
    builder->setColorMode(gfx::ColorMode::alphaBlended());
    builder->setCullFaceMode(gfx::CullFaceMode::disabled());
    builder->setVertexAttrId(vertexAttribute);
    builder->addQuad(-1, -1, 1, 1);
    builder->flush(context);

    for (auto& drawable : builder->clearDrawables()) {
        layerGroup->addDrawable(std::move(drawable));
    }
    return layerGroup;
}

// GL JS `getSunPos`: the atmosphere traces rays in view space, so a map-anchored light turns with the globe.
std::array<float, 4> sunPosition(const EvaluatedLight& light, const TransformState& state, float blend) {
    const auto position = light.get<style::LightPosition>().getCartesian();
    vec4 sun = {{-position[0], -position[1], -position[2], 0.0}};

    if (light.get<style::LightAnchor>() == style::LightAnchorType::Map) {
        const auto center = state.getLatLng();
        mat4 matrix;
        matrix::identity(matrix);
        matrix::rotate_z(matrix, matrix, state.getRoll());
        matrix::rotate_x(matrix, matrix, -state.getPitch());
        matrix::rotate_z(matrix, matrix, -state.getBearing());
        matrix::rotate_x(matrix, matrix, center.latitude() * std::numbers::pi / 180.0);
        matrix::rotate_y(matrix, matrix, -center.longitude() * std::numbers::pi / 180.0);
        matrix::transformMat4(sun, sun, matrix);
    }

    return {{static_cast<float>(sun[0]), static_cast<float>(sun[1]), static_cast<float>(sun[2]), blend}};
}

// GL JS `ATMOSPHERE_HEIGHT_TO_GLOBE_RADIUS`: the atmosphere is 100 km deep on a 6371 km planet.
constexpr double atmosphereHeightToGlobeRadius = 100000.0 / 6371000.0;

// GL JS `getAtmosphereAltitudeBlend`: 0 inside the atmosphere, 1 from ten times its height above the surface.
double atmosphereAltitudeBlend(double altitude, double globeRadius) {
    const double atmosphereHeight = globeRadius * atmosphereHeightToGlobeRadius;
    const double x = std::clamp((altitude - atmosphereHeight) / (9.0 * atmosphereHeight), 0.0, 1.0);
    return x * x * (3.0 - 2.0 * x);
}

} // namespace

void SkyPass::update(gfx::ShaderRegistry& shaders,
                     gfx::Context& context,
                     const TransformState& state,
                     const Size& viewportSize,
                     float pixelRatio,
                     const std::optional<EvaluatedSky>& evaluated,
                     const EvaluatedLight& light) {
    if (!evaluated || viewportSize.isEmpty()) {
        if (skyLayerGroup) {
            skyLayerGroup->clearDrawables();
        }
        if (atmosphereLayerGroup) {
            atmosphereLayerGroup->clearDrawables();
        }
        return;
    }

    const double projectionTransition = std::clamp(state.getProjectionTransition(), 0.0, 1.0);

    // GL JS `getGlobeCenterInViewSpace` and `getGlobeRadiusPixels`: the sky curves over the globe and the atmosphere
    // fades in as the camera climbs out of it.
    mat4 invProjection;
    matrix::identity(invProjection);
    std::array<float, 4> globe = {{0.0f, 0.0f, 0.0f, 0.0f}};
    double atmosphereHeight = 0.0;
    double altitudeBlend = 1.0;
    if (state.isGlobeRendering()) {
        const double radius = state.getGlobeRadiusPixels();
        if (matrix::invert(invProjection, VerticalPerspectiveProjection::globeProjectionMatrix(state, radius))) {
            matrix::identity(invProjection);
        }
        vec4 center = {{0.0, 0.0, 0.0, 1.0}};
        matrix::transformMat4(center, center, state.getGlobeViewProjectionMatrix());
        matrix::transformMat4(center, center, invProjection);
        const vec3 position = {{center[0] / center[3], center[1] / center[3], center[2] / center[3]}};
        globe = {{static_cast<float>(position[0]),
                  static_cast<float>(position[1]),
                  static_cast<float>(position[2]),
                  static_cast<float>(radius)}};
        atmosphereHeight = radius * atmosphereHeightToGlobeRadius;
        altitudeBlend = atmosphereAltitudeBlend(std::hypot(position[0], position[1], position[2]) - radius, radius);
    }

    // On the globe the sky shows only from inside ten atmosphere heights, where the altitude blend is below 1.
    if (projectionTransition < 1.0 || altitudeBlend < 1.0) {
        if (!skyShader) {
            skyShader = context.getGenericShader(shaders, std::string(SkyShaderName), gfx::ProjectionVariant::Mercator);
        }
        if (!skyLayerGroup || skyLayerGroup->empty()) {
            skyLayerGroup = makeFullscreenLayerGroup(
                context, skyShader, "sky", shaders::idSkyPosVertexAttribute, false, gfx::DepthMaskType::ReadWrite);
        }

        if (skyLayerGroup) {
            constexpr double maxHorizonAngle = 89.25 * std::numbers::pi / 180.0;
            const double pitch = state.getPitch();
            const double horizonDistance = state.getCameraToCenterDistance() * pixelRatio *
                                           std::min(std::tan(std::numbers::pi / 2.0 - pitch) * 0.85,
                                                    std::tan(maxHorizonAngle - pitch));
            const double roll = state.getRoll();
            const std::array<float, 2> normal = {
                {static_cast<float>(-std::sin(roll)), static_cast<float>(std::cos(roll))}};
            const std::array<float, 2> horizon = {
                {static_cast<float>(viewportSize.width * 0.5 + normal[0] * horizonDistance),
                 static_cast<float>(viewportSize.height * 0.5 + normal[1] * horizonDistance)}};

            const shaders::SkyPropsUBO props = {
                .inv_projection = util::cast<float>(invProjection),
                .sky_color = evaluated->get<style::SkyColor>(),
                .horizon_color = evaluated->get<style::SkyHorizonColor>(),
                .globe = globe,
                .horizon = horizon,
                .horizon_normal = normal,
                .viewport_size = {{static_cast<float>(viewportSize.width), static_cast<float>(viewportSize.height)}},
                .sky_horizon_blend = evaluated->get<style::SkyHorizonBlend>() * viewportSize.height * 0.5f,
                .sky_blend = static_cast<float>(projectionTransition),
                .camera_to_center_distance = state.getCameraToCenterDistance(),
                .pixel_ratio = pixelRatio,
                .atmosphere_blend = static_cast<float>(evaluated->get<style::SkyAtmosphereBlend>() * altitudeBlend),
                .atmosphere_height = static_cast<float>(atmosphereHeight),
            };
            skyLayerGroup->mutableUniformBuffers().createOrUpdate(shaders::idSkyPropsUBO, &props, context);
        }
    } else if (skyLayerGroup) {
        skyLayerGroup->clearDrawables();
    }

    const float atmosphereBlend = evaluated->get<style::SkyAtmosphereBlend>() *
                                  static_cast<float>(projectionTransition * altitudeBlend);
    if (!state.isGlobeRendering() || atmosphereBlend <= 0.0f) {
        if (atmosphereLayerGroup) {
            atmosphereLayerGroup->clearDrawables();
        }
        return;
    }

    if (!atmosphereShader) {
        atmosphereShader = context.getGenericShader(
            shaders, std::string(AtmosphereShaderName), gfx::ProjectionVariant::Globe);
    }
    if (!atmosphereLayerGroup || atmosphereLayerGroup->empty()) {
        atmosphereLayerGroup = makeFullscreenLayerGroup(context,
                                                        atmosphereShader,
                                                        "atmosphere",
                                                        shaders::idAtmospherePosVertexAttribute,
                                                        true,
                                                        gfx::DepthMaskType::ReadOnly);
    }
    if (!atmosphereLayerGroup) {
        return;
    }

    const shaders::AtmospherePropsUBO props = {
        .inv_projection = util::cast<float>(invProjection),
        .globe = globe,
        .sun_position = sunPosition(light, state, atmosphereBlend),
    };
    atmosphereLayerGroup->mutableUniformBuffers().createOrUpdate(shaders::idAtmospherePropsUBO, &props, context);
}

} // namespace mln
