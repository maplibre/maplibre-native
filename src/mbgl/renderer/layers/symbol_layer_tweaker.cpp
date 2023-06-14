#include <mbgl/renderer/layers/symbol_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/style/layers/symbol_layer_properties.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/std.hpp>

namespace mbgl {

using namespace style;

struct alignas(16) SymbolDrawableUBO {
    /*   0 */ std::array<float, 4 * 4> matrix;
};
static_assert(sizeof(SymbolDrawableUBO) == 4*16);

/// Evaluated properties that do not depend on the tile
struct alignas(16) SymbolDrawablePropsUBO {
    /*  0 */ Color color;
    /* 16 */ Color outline_color;
    /* 32 */ float opacity;
    /* 36 */ std::array<float, 3> padding;
    /* 48 */
};
static_assert(sizeof(SymbolDrawablePropsUBO) == 48);

static constexpr std::string_view SymbolDrawableUBOName = "SymbolDrawableUBO";
static constexpr std::string_view SymbolDrawablePropsUBOName = "SymbolDrawablePropsUBO";

void SymbolLayerTweaker::execute(LayerGroupBase& layerGroup,
                               const RenderTree& renderTree,
                               const PaintParameters& parameters) {
    const auto& props = static_cast<const SymbolLayerProperties&>(*evaluatedProperties);
    const auto& evaluated = props.evaluated;

    if (layerGroup.empty()) {
        return;
    }

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    if (!propsBuffer) {
        const SymbolDrawablePropsUBO paramsUBO = {
        };
        propsBuffer = parameters.context.createUniformBuffer(&paramsUBO, sizeof(paramsUBO));
    }

//    layout_.get<style::IconPitchAlignment>(),
//    layout_.get<style::IconRotationAlignment>(),
//    layout_.get<style::IconKeepUpright>(),
//    evaluated_.get<style::IconTranslate>(),
//    evaluated_.get<style::IconTranslateAnchor>(),
//    evaluated_.get<style::IconHaloColor>().constantOr(Color::black()).a > 0 &&
//    evaluated_.get<style::IconHaloWidth>().constantOr(1),
//    evaluated_.get<style::IconColor>().constantOr(Color::black()).a > 0};
//    layout_.get<style::TextPitchAlignment>(),
//    layout_.get<style::TextRotationAlignment>(),
//    layout_.get<style::TextKeepUpright>(),
//    evaluated_.get<style::TextTranslate>(),
//    evaluated_.get<style::TextTranslateAnchor>(),
//    evaluated_.get<style::TextHaloColor>().constantOr(Color::black()).a > 0 &&
//    evaluated_.get<style::TextHaloWidth>().constantOr(1),
//    evaluated_.get<style::TextColor>().constantOr(Color::black()).a > 0};

    layerGroup.observeDrawables([&](gfx::Drawable& drawable) {
        drawable.mutableUniformBuffers().addOrReplace(SymbolDrawablePropsUBOName, propsBuffer);

        if (!drawable.getTileID()) {
            return;
        }

        const UnwrappedTileID tileID = drawable.getTileID()->toUnwrapped();

        const auto translate = evaluated.get<style::TextTranslate>();
        const auto anchor = evaluated.get<style::TextTranslateAnchor>();

        constexpr bool inViewportPixelUnits = false; // from RenderTile::translatedMatrix
        const auto matrix = getTileMatrix(
            tileID, renderTree, parameters.state, translate, anchor, inViewportPixelUnits);

        const SymbolDrawableUBO drawableUBO = {
            /*.matrix=*/util::cast<float>(matrix),
        };

        drawable.mutableUniformBuffers().createOrUpdate(SymbolDrawableUBOName, &drawableUBO, parameters.context);
    });
}

} // namespace mbgl
