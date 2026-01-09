#include <mbgl/renderer/layers/hillshade_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/shaders/hillshade_layer_ubo.hpp>
#include <mbgl/style/layers/hillshade_layer_properties.hpp>
#include <mbgl/util/convert.hpp>

namespace mbgl {

using namespace style;
using namespace shaders;

namespace {
std::array<float, 2> getLatRange(const UnwrappedTileID& id) {
    const LatLng latlng0 = LatLng(id);
    const LatLng latlng1 = LatLng(UnwrappedTileID(id.canonical.z, id.canonical.x, id.canonical.y + 1));
    return {{static_cast<float>(latlng0.latitude()), static_cast<float>(latlng1.latitude())}};
}

std::array<float, 2> getLight(const PaintParameters& parameters,
                              const HillshadePaintProperties::PossiblyEvaluated& evaluated) {
    float azimuthal = util::deg2radf(evaluated.get<HillshadeIlluminationDirection>());
    if (evaluated.get<HillshadeIlluminationAnchor>() == HillshadeIlluminationAnchorType::Viewport) {
        azimuthal = azimuthal - static_cast<float>(parameters.state.getBearing());
    }
    return {{evaluated.get<HillshadeExaggeration>(), azimuthal}};
}
} // namespace

void HillshadeLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    if (layerGroup.empty()) {
        return;
    }

    const auto& evaluated = static_cast<const HillshadeLayerProperties&>(*evaluatedProperties).evaluated;

#ifndef NDEBUG
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    if (!evaluatedPropsUniformBuffer || propertiesUpdated) {
        const HillshadeEvaluatedPropsUBO evaluatedPropsUBO = {.highlight = evaluated.get<HillshadeHighlightColor>(),
                                                              .shadow = evaluated.get<HillshadeShadowColor>(),
                                                              .accent = evaluated.get<HillshadeAccentColor>()};
        parameters.context.emplaceOrUpdateUniformBuffer(evaluatedPropsUniformBuffer, &evaluatedPropsUBO);
        propertiesUpdated = false;
    }
    auto& layerUniforms = layerGroup.mutableUniformBuffers();
    layerUniforms.set(idHillshadeEvaluatedPropsUBO, evaluatedPropsUniformBuffer);

#if MLN_UBO_CONSOLIDATION
    int i = 0;
    std::vector<HillshadeDrawableUBO> drawableUBOVector(layerGroup.getDrawableCount());
    std::vector<HillshadeTilePropsUBO> tilePropsUBOVector(layerGroup.getDrawableCount());
#endif

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !checkTweakDrawable(drawable)) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        const auto matrix = getTileMatrix(
            tileID, parameters, {0.f, 0.f}, TranslateAnchorType::Viewport, false, false, drawable, true);

#if MLN_UBO_CONSOLIDATION
        drawableUBOVector[i] = {
#else
        const HillshadeDrawableUBO drawableUBO = {
#endif
            /* .matrix = */ util::cast<float>(matrix)
        };

#if MLN_UBO_CONSOLIDATION
        tilePropsUBOVector[i] = {
#else
        const HillshadeTilePropsUBO tilePropsUBO = {
#endif
            .latrange = getLatRange(tileID),
            .light = getLight(parameters, evaluated)
        };

#if MLN_UBO_CONSOLIDATION
        drawable.setUBOIndex(i++);
#else
        auto& drawableUniforms = drawable.mutableUniformBuffers();
        drawableUniforms.createOrUpdate(idHillshadeDrawableUBO, &drawableUBO, parameters.context);
        drawableUniforms.createOrUpdate(idHillshadeTilePropsUBO, &tilePropsUBO, parameters.context);
#endif
    });

#if MLN_UBO_CONSOLIDATION
    auto& context = parameters.context;
    const size_t drawableUBOVectorSize = sizeof(HillshadeDrawableUBO) * drawableUBOVector.size();
    if (!drawableUniformBuffer || drawableUniformBuffer->getSize() < drawableUBOVectorSize) {
        drawableUniformBuffer = context.createUniformBuffer(
            drawableUBOVector.data(), drawableUBOVectorSize, false, true);
    } else {
        drawableUniformBuffer->update(drawableUBOVector.data(), drawableUBOVectorSize);
    }

    const size_t tilePropsUBOVectorSize = sizeof(HillshadeTilePropsUBO) * tilePropsUBOVector.size();
    if (!tilePropsUniformBuffer || tilePropsUniformBuffer->getSize() < tilePropsUBOVectorSize) {
        tilePropsUniformBuffer = context.createUniformBuffer(
            tilePropsUBOVector.data(), tilePropsUBOVectorSize, false, true);
    } else {
        tilePropsUniformBuffer->update(tilePropsUBOVector.data(), tilePropsUBOVectorSize);
    }

    layerUniforms.set(idHillshadeDrawableUBO, drawableUniformBuffer);
    layerUniforms.set(idHillshadeTilePropsUBO, tilePropsUniformBuffer);
#endif
}

} // namespace mbgl
