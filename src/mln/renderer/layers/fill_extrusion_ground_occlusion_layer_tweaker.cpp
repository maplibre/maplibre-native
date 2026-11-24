#include <mln/renderer/layers/fill_extrusion_ground_occlusion_layer_tweaker.hpp>

#include <mln/gfx/context.hpp>
#include <mln/gfx/drawable.hpp>
#include <mln/renderer/buckets/fill_extrusion_bucket.hpp>
#include <mln/renderer/layer_group.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/renderer/paint_property_binder.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/shaders/fill_extrusion_ground_occlusion_layer_ubo.hpp>
#include <mln/shaders/shader_program_base.hpp>
#include <mln/style/layers/fill_extrusion_layer_properties.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/convert.hpp>
#include <mln/util/projection.hpp>

#include <algorithm>

namespace mln {

using namespace shaders;
using namespace style;

void FillExtrusionGroundOcclusionLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    if (layerGroup.empty()) {
        return;
    }

    auto& context = parameters.context;
    const auto& props = static_cast<const FillExtrusionLayerProperties&>(*evaluatedProperties);
    const auto& evaluated = props.evaluated;
    const auto& state = parameters.state;

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    // Spec `minimum`/`maximum` are documentation only, so clamp here rather than trusting the style.
    // fill-extrusion-opacity applies to the whole layer, so the occlusion fades with the buildings.
    const float opacity = std::clamp(evaluated.get<FillExtrusionOpacity>(), 0.0f, 1.0f);
    const FillExtrusionGroundOcclusionPropsUBO propsUBO = {
        .intensity = std::clamp(evaluated.get<FillExtrusionAmbientOcclusionGroundIntensity>(), 0.0f, 1.0f) * opacity,
        .base = constOrDefault<FillExtrusionBase>(evaluated),
        .height = constOrDefault<FillExtrusionHeight>(evaluated),
        .pad1 = 0};
    auto& layerUniforms = layerGroup.mutableUniformBuffers();
    layerUniforms.createOrUpdate(idFillExtrusionGroundOcclusionPropsUBO, &propsUBO, context);

    propertiesUpdated = false;

    const float radius = std::max(evaluated.get<FillExtrusionAmbientOcclusionGroundRadius>(), 0.0f);

    // Metres on the ground convert to tile units as
    //
    //     2^tile.z * EXTENT / (cos(lat) * 2*pi * EARTH_RADIUS_M)
    //
    // because the tile matrix scales x/y by (worldSize / 2^tile.z) / EXTENT. Everything except the
    // per-tile 2^tile.z is hoisted out of the loop here. This uses the latitude of the camera
    // centre, like Camera::getWorldToCamera does for the extrusion heights, so the radius matches
    // the buildings and does not jump between tiles.
    const double metersPerPixel = Projection::getMetersPerPixelAtLatitude(state.getLatLng().latitude(),
                                                                          state.getZoom());
    const double tileUnitsPerMeterBase = util::EXTENT / (metersPerPixel * Projection::worldSize(state.getScale()));

    const auto zoom = static_cast<float>(state.getZoom());

#if MLN_UBO_CONSOLIDATION
    int i = 0;
    std::vector<FillExtrusionGroundOcclusionDrawableUBO> drawableUBOVector(layerGroup.getDrawableCount());
#endif

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !checkTweakDrawable(drawable)) {
            return;
        }

        auto* binders = static_cast<FillExtrusionBinders*>(drawable.getBinders());
        const auto* tile = drawable.getRenderTile();
        if (!binders || !tile) {
            assert(false);
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();
        const auto& translation = evaluated.get<FillExtrusionTranslate>();
        const auto anchor = evaluated.get<FillExtrusionTranslateAnchor>();
        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        constexpr bool nearClipped = true;
        const auto matrix = getTileMatrix(
            tileID, parameters, translation, anchor, nearClipped, inViewportPixelUnits, drawable);

        const auto numTiles = static_cast<double>(1ull << tileID.canonical.z);

#if MLN_UBO_CONSOLIDATION
        drawableUBOVector[i] = {
#else
        const FillExtrusionGroundOcclusionDrawableUBO drawableUBO = {
#endif
            .matrix = util::cast<float>(matrix),

            .base_t = std::get<0>(binders->get<FillExtrusionBase>()->interpolationFactor(zoom)),
            .height_t = std::get<0>(binders->get<FillExtrusionHeight>()->interpolationFactor(zoom)),

            .meters_to_tile_units = static_cast<float>(numTiles * tileUnitsPerMeterBase),
            .radius = radius
        };

#if MLN_UBO_CONSOLIDATION
        drawable.setUBOIndex(i++);
#else
        auto& drawableUniforms = drawable.mutableUniformBuffers();
        drawableUniforms.createOrUpdate(idFillExtrusionGroundOcclusionDrawableUBO, &drawableUBO, context);
#endif
    });

#if MLN_UBO_CONSOLIDATION
    const size_t drawableUBOVectorSize = sizeof(FillExtrusionGroundOcclusionDrawableUBO) * drawableUBOVector.size();
    if (!drawableUniformBuffer || drawableUniformBuffer->getSize() < drawableUBOVectorSize) {
        drawableUniformBuffer = context.createUniformBuffer(
            drawableUBOVector.data(), drawableUBOVectorSize, false, true);
    } else {
        drawableUniformBuffer->update(drawableUBOVector.data(), drawableUBOVectorSize);
    }

    layerUniforms.set(idFillExtrusionGroundOcclusionDrawableUBO, drawableUniformBuffer);
#endif
}

} // namespace mln
