#include <mbgl/renderer/layers/symbol_layer_tweaker.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/symbol_drawable_data.hpp>
#include <mbgl/layout/symbol_projection.hpp>
#include <mbgl/programs/symbol_program.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/paint_property_binder.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/shaders/symbol_layer_ubo.hpp>
#include <mbgl/style/layers/symbol_layer_properties.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/math.hpp>
#include <mbgl/util/std.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/shaders/mtl/symbol_icon.hpp>
#include <mbgl/shaders/mtl/symbol_sdf.hpp>
#endif // MLN_RENDER_BACKEND_METAL

namespace mbgl {

using namespace style;
using namespace shaders;

namespace {

Size getTexSize(const gfx::Drawable& drawable, const size_t texId) {
    if (const auto& tex = drawable.getTexture(texId)) {
        return tex->getSize();
    }
    return {0, 0};
}

std::array<float, 2> toArray(const Size& s) {
    return util::cast<float>(std::array<uint32_t, 2>{s.width, s.height});
}

} // namespace

void SymbolLayerTweaker::execute(LayerGroupBase& layerGroup, const PaintParameters& parameters) {
    auto& context = parameters.context;
    const auto& state = parameters.state;
    const auto& evaluated = static_cast<const SymbolLayerProperties&>(*evaluatedProperties).evaluated;

    if (layerGroup.empty()) {
        return;
    }

#if !defined(NDEBUG)
    const auto label = layerGroup.getName() + "-update-uniforms";
    const auto debugGroup = parameters.encoder->createDebugGroup(label.c_str());
#endif

    if (!evaluatedPropsUniformBuffer || propertiesUpdated) {
        const SymbolEvaluatedPropsUBO propsUBO = {/*.text_fill_color=*/constOrDefault<TextColor>(evaluated),
                                                  /*.text_halo_color=*/constOrDefault<TextHaloColor>(evaluated),
                                                  /*.text_opacity=*/constOrDefault<TextOpacity>(evaluated),
                                                  /*.text_halo_width=*/constOrDefault<TextHaloWidth>(evaluated),
                                                  /*.text_halo_blur=*/constOrDefault<TextHaloBlur>(evaluated),
                                                  0,
                                                  /*.icon_fill_color=*/constOrDefault<IconColor>(evaluated),
                                                  /*.icon_halo_color=*/constOrDefault<IconHaloColor>(evaluated),
                                                  /*.icon_opacity=*/constOrDefault<IconOpacity>(evaluated),
                                                  /*.icon_halo_width=*/constOrDefault<IconHaloWidth>(evaluated),
                                                  /*.icon_halo_blur=*/constOrDefault<IconHaloBlur>(evaluated),
                                                  0};
        context.emplaceOrUpdateUniformBuffer(evaluatedPropsUniformBuffer, &propsUBO);
        propertiesUpdated = false;
    }
    auto& layerUniforms = layerGroup.mutableUniformBuffers();
    layerUniforms.set(idSymbolEvaluatedPropsUBO, evaluatedPropsUniformBuffer);

    int i = 0;
    std::vector<SymbolComputeUBO> computeUBOVector(layerGroup.getDrawableCount());
    
    constexpr bool aligned = false;
    constexpr bool nearClipped = false;
    constexpr bool inViewportPixelUnits = false;
    const auto camDist = state.getCameraToCenterDistance();
    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !drawable.getData()) {
            return;
        }

        const auto tileID = drawable.getTileID()->toUnwrapped();
        const auto& symbolData = static_cast<gfx::SymbolDrawableData&>(*drawable.getData());
        const auto isText = (symbolData.symbolType == SymbolType::Text);

        // from RenderTile::translatedMatrix
        const auto translate = isText ? evaluated.get<style::TextTranslate>() : evaluated.get<style::IconTranslate>();
        const auto anchor = isText ? evaluated.get<style::TextTranslateAnchor>() : evaluated.get<style::IconTranslateAnchor>();
        const bool pitchWithMap = symbolData.pitchAlignment == style::AlignmentType::Map;
        const bool rotateWithMap = symbolData.rotationAlignment == style::AlignmentType::Map;
        const bool alongLine = symbolData.placement != SymbolPlacementType::Point && symbolData.rotationAlignment == AlignmentType::Map;
        const bool hasVariablePlacement = symbolData.bucketVariablePlacement && (isText || symbolData.textFit != IconTextFitType::None);
        
        int32_t layerIndex = -1;
        int32_t subLayerIndex = 0;
        if (!drawable.getIs3D() && drawable.getEnableDepth()) {
            layerIndex = drawable.getLayerIndex();
            subLayerIndex = drawable.getSubLayerIndex();
        }
        
        // nearClippedMatrix has near plane moved further, to enhance depth buffer precision
        auto& projMatrix = aligned ? parameters.transformParams.alignedProjMatrix
                                   : (nearClipped ? parameters.transformParams.nearClippedProjMatrix
                                                  : parameters.transformParams.projMatrix);
        
        computeUBOVector[i] = {
            /*.projMatrix=*/util::cast<float>(projMatrix),

            /*.layerIndex=*/layerIndex,
            /*.subLayerIndex=*/subLayerIndex,
            /*.width=*/state.getSize().width,
            /*.height=*/state.getSize().height,
            
            /*.texsize=*/toArray(getTexSize(drawable, idSymbolImageTexture)),
            /*.texsize_icon=*/toArray(getTexSize(drawable, idSymbolImageIconTexture)),
            
            /*.tileIdCanonicalX=*/tileID.canonical.x,
            /*.tileIdCanonicalY=*/tileID.canonical.y,
            /*.tileIdCanonicalZ=*/tileID.canonical.z,
            /*.tileIdWrap=*/tileID.wrap,
            /*.camDist=*/camDist,
            
            /*.scale=*/static_cast<float>(state.getScale()),
            /*.bearing=*/static_cast<float>(state.getBearing()),
            /*.zoom=*/static_cast<float>(state.getZoom()),
            /*.pitch=*/static_cast<float>(state.getPitch()),
            
            /*.translation=*/translate,

            /*.isAnchorMap=*/(anchor == TranslateAnchorType::Map),
            /*.inViewportPixelUnits=*/inViewportPixelUnits,
            /*.pitchWithMap=*/pitchWithMap,
            /*.rotateWithMap=*/rotateWithMap,
            /*.alongLine=*/alongLine,
            /*.hasVariablePlacement=*/hasVariablePlacement,
            /*.padding=*/0
        };
        drawable.setUBOIndex(i);
        i++;
    });
    
    parameters.computePass->computeDrawableBuffer(computeUBOVector, computeBuffer, drawableBuffer);
    
    if (layerGroup.getDrawableCount() > 60 ) {
        assert(false);
    }
    layerUniforms.set(idSymbolDrawableUBO, drawableBuffer);
}

} // namespace mbgl
