#include <mbgl/renderer/layers/hillshade_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/shaders/hillshade_layer_ubo.hpp>
#include <mbgl/style/layers/hillshade_layer_properties.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/string_indexer.hpp>

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
    const auto& evaluated = static_cast<const HillshadeLayerProperties&>(*evaluatedProperties).evaluated;

    if (layerGroup.empty()) {
        return;
    }

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    const auto getPropsBuffer = [&]() -> auto& {
        if (!evaluatedPropsUniformBuffer || propertiesUpdated) {
            const HillshadeEvaluatedPropsUBO evaluatedPropsUBO = {
                /* .highlight = */ evaluated.get<HillshadeHighlightColor>(),
                /* .shadow = */ evaluated.get<HillshadeShadowColor>(),
                /* .accent = */ evaluated.get<HillshadeAccentColor>()};
            parameters.context.emplaceOrUpdateUniformBuffer(evaluatedPropsUniformBuffer, &evaluatedPropsUBO);
            propertiesUpdated = false;
        }
        return evaluatedPropsUniformBuffer;
    };

    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !checkTweakDrawable(drawable)) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        auto& uniforms = drawable.mutableUniformBuffers();
        uniforms.set(idHillshadeEvaluatedPropsUBO, getPropsBuffer());

        const auto matrix = getTileMatrix(
            tileID, parameters, {0.f, 0.f}, TranslateAnchorType::Viewport, false, false, drawable, true);
        HillshadeDrawableUBO drawableUBO = {/* .matrix = */ util::cast<float>(matrix),
                                            /* .latrange = */ getLatRange(tileID),
                                            /* .light = */ getLight(parameters, evaluated)};
        uniforms.createOrUpdate(idHillshadeDrawableUBO, &drawableUBO, parameters.context);
    });
}

} // namespace mbgl
