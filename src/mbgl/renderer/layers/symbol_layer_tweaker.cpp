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

SymbolDrawablePaintUBO buildPaintUBO(bool isText, const SymbolPaintProperties::PossiblyEvaluated& evaluated) {
    return {
        /*.fill_color=*/isText ? constOrDefault<TextColor>(evaluated) : constOrDefault<IconColor>(evaluated),
        /*.halo_color=*/
        isText ? constOrDefault<TextHaloColor>(evaluated) : constOrDefault<IconHaloColor>(evaluated),
        /*.opacity=*/isText ? constOrDefault<TextOpacity>(evaluated) : constOrDefault<IconOpacity>(evaluated),
        /*.halo_width=*/
        isText ? constOrDefault<TextHaloWidth>(evaluated) : constOrDefault<IconHaloWidth>(evaluated),
        /*.halo_blur=*/isText ? constOrDefault<TextHaloBlur>(evaluated) : constOrDefault<IconHaloBlur>(evaluated),
        /*.padding=*/0,
    };
}

float pixelsToTileUnits(const float pixelValue, const uint8_t tileIdCanonicalZ, const float zoom) {
    return pixelValue * (static_cast<float>(util::EXTENT) / (static_cast<float>(util::tileSize_D) * std::pow(2.f, zoom - tileIdCanonicalZ)));
}

mat4 computeTileMatrix(SymbolComputeUBO& computeUBO) {
    // from RenderTile::prepare
    mat4 tileMatrix;
    const uint64_t tileScale = 1ull << computeUBO.tileIdCanonicalZ;
    const double s = Projection::worldSize(computeUBO.scale) / tileScale;

    matrix::identity(tileMatrix);
    matrix::translate(tileMatrix,
                      tileMatrix,
                      int64_t(computeUBO.tileIdCanonicalX + computeUBO.tileIdWrap * static_cast<int64_t>(tileScale)) * s,
                      int64_t(computeUBO.tileIdCanonicalY) * s,
                      0);
    matrix::scale(tileMatrix, tileMatrix, s / util::EXTENT, s / util::EXTENT, 1);

    computeUBO.projMatrix[14] -= ((1 + computeUBO.layerIndex) * PaintParameters::numSublayers - computeUBO.subLayerIndex) * PaintParameters::depthEpsilon;
    matrix::multiply(tileMatrix, computeUBO.projMatrix, tileMatrix);

    if (computeUBO.translation[0] == 0 && computeUBO.translation[1] == 0) {
        return tileMatrix;
    }

    mat4 vtxMatrix;

    const float angle = computeUBO.inViewportPixelUnits
                            ? (computeUBO.isAnchorMap ? static_cast<float>(computeUBO.bearing) : 0.0f)
                            : (computeUBO.isAnchorMap ? 0.0f : static_cast<float>(-computeUBO.bearing));

    Point<float> translate = util::rotate(Point<float>{computeUBO.translation[0], computeUBO.translation[1]}, angle);

    if (computeUBO.inViewportPixelUnits) {
        matrix::translate(vtxMatrix, tileMatrix, translate.x, translate.y, 0);
    } else {
        matrix::translate(vtxMatrix,
                          tileMatrix,
                          pixelsToTileUnits(translate.x, computeUBO.tileIdCanonicalZ, static_cast<float>(computeUBO.zoom)),
                          pixelsToTileUnits(translate.y, computeUBO.tileIdCanonicalZ, static_cast<float>(computeUBO.zoom)),
                          0);
    }
    
    return vtxMatrix;
}

mat4 computeLabelPlaneMatrix(const mat4& posMatrix,
                             const SymbolComputeUBO& computeUBO,
                             const float pixelsToTileUnits) {
    if (computeUBO.alongLine || computeUBO.hasVariablePlacement) {
        return matrix::identity4();
    }
    
    mat4 m;
    matrix::identity(m);
    if (computeUBO.pitchWithMap) {
        matrix::scale(m, m, 1 / pixelsToTileUnits, 1 / pixelsToTileUnits, 1);
        if (!computeUBO.rotateWithMap) {
            matrix::rotate_z(m, m, computeUBO.bearing);
        }
    } else {
        matrix::scale(m, m, computeUBO.width / 2.0, -(computeUBO.height / 2.0), 1.0);
        matrix::translate(m, m, 1, -1, 0);
        matrix::multiply(m, m, posMatrix);
    }
    return m;
}

mat4 computeGlCoordMatrix(const mat4& posMatrix,
                          const SymbolComputeUBO& computeUBO,
                          const float pixelsToTileUnits) {
    mat4 m;
    matrix::identity(m);
    if (computeUBO.pitchWithMap) {
        matrix::multiply(m, m, posMatrix);
        matrix::scale(m, m, pixelsToTileUnits, pixelsToTileUnits, 1);
        if (!computeUBO.rotateWithMap) {
            matrix::rotate_z(m, m, -computeUBO.bearing);
        }
    } else {
        matrix::scale(m, m, 1, -1, 1);
        matrix::translate(m, m, -1, -1, 0);
        matrix::scale(m, m, 2.0 / computeUBO.width, 2.0 / computeUBO.height, 1.0);
    }
    return m;
}

void computeDrawableUBO(std::vector<SymbolDrawableUBO>& out, std::vector<SymbolComputeUBO>& in, int i) {
    auto& computeUBO = in[i];
    
    const auto matrix = computeTileMatrix(computeUBO);

    // from symbol_program, makeValues
    const auto currentZoom = static_cast<float>(computeUBO.zoom);
    const float pixelsToUnits = pixelsToTileUnits(1.f, computeUBO.tileIdCanonicalZ, currentZoom);

    const mat4 labelPlaneMatrix = computeLabelPlaneMatrix(matrix, computeUBO, pixelsToUnits);
    const mat4 glCoordMatrix = computeGlCoordMatrix(matrix, computeUBO, pixelsToUnits);

    const float gammaScale = (computeUBO.pitchWithMap ? static_cast<float>(std::cos(computeUBO.pitch)) * computeUBO.camDist : 1.0f);

    // Line label rotation happens in `updateLineLabels`/`reprojectLineLabels``
    // Pitched point labels are automatically rotated by the labelPlaneMatrix projection
    // Unpitched point labels need to have their rotation applied after projection
    const bool rotateInShader = computeUBO.rotateWithMap && !computeUBO.pitchWithMap && !computeUBO.alongLine;
    
    out[i] = {
        /*.matrix=*/util::cast<float>(matrix),
        /*.label_plane_matrix=*/util::cast<float>(labelPlaneMatrix),
        /*.coord_matrix=*/util::cast<float>(glCoordMatrix),

        /*.texsize=*/computeUBO.texsize,
        /*.texsize_icon=*/computeUBO.texsize_icon,

        /*.gamma_scale=*/gammaScale,
        /*.rotate_symbol=*/rotateInShader,
        /*.pad=*/{0},
    };
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

    if (propertiesUpdated) {
        textPropertiesUpdated = true;
        iconPropertiesUpdated = true;
        propertiesUpdated = false;
    }

    const auto camDist = state.getCameraToCenterDistance();

    const SymbolDynamicUBO dynamicUBO = {/*.fade_change=*/parameters.symbolFadeChange,
                                         /*.camera_to_center_distance=*/camDist,
                                         /*.aspect_ratio=*/state.getSize().aspectRatio(),
                                         0};

    if (!dynamicBuffer) {
        dynamicBuffer = parameters.context.createUniformBuffer(&dynamicUBO, sizeof(dynamicUBO));
    } else {
        dynamicBuffer->update(&dynamicUBO, sizeof(dynamicUBO));
    }

    int i = 0;
    std::vector<SymbolDrawableUBO> drawableUBOVector(layerGroup.getDrawableCount());
    std::vector<SymbolComputeUBO> computeUBOVector(layerGroup.getDrawableCount());
    
    constexpr bool aligned = false;
    constexpr bool nearClipped = false;
    constexpr bool inViewportPixelUnits = false;
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
            /*.projMatrix=*/projMatrix,
            
            /*.tileIdCanonicalX=*/tileID.canonical.x,
            /*.tileIdCanonicalY=*/tileID.canonical.y,
            /*.tileIdCanonicalZ=*/tileID.canonical.z,
            /*.tileIdWrap=*/tileID.wrap,

            /*.layerIndex=*/layerIndex,
            /*.subLayerIndex=*/subLayerIndex,
            
            /*.translation=*/translate,
            
            /*.scale=*/state.getScale(),
            /*.bearing=*/state.getBearing(),
            /*.zoom=*/state.getZoom(),
            /*.width=*/state.getSize().width,
            /*.height=*/state.getSize().height,
            /*.camDist=*/camDist,
            /*.pitch=*/state.getPitch(),
            
            /*.texsize=*/toArray(getTexSize(drawable, idSymbolImageTexture)),
            /*.texsize_icon=*/toArray(getTexSize(drawable, idSymbolImageIconTexture)),

            /*.isAnchorMap=*/(anchor == TranslateAnchorType::Map),
            /*.inViewportPixelUnits=*/inViewportPixelUnits,
            /*.inViewportPixelUnits=*/pitchWithMap,
            /*.inViewportPixelUnits=*/rotateWithMap,
            /*.inViewportPixelUnits=*/alongLine,
            /*.inViewportPixelUnits=*/hasVariablePlacement,
        };
        
        computeDrawableUBO(drawableUBOVector, computeUBOVector, i);
        
        i++;
    });
    
    if (!drawableBuffer || drawableBuffer->getSize() < sizeof(SymbolDrawableUBO) * drawableUBOVector.size()) {
        drawableBuffer = parameters.context.createUniformBuffer(drawableUBOVector.data(), sizeof(SymbolDrawableUBO) * drawableUBOVector.size());
    } else {
        drawableBuffer->update(drawableUBOVector.data(), sizeof(SymbolDrawableUBO) * drawableUBOVector.size());
    }
    
    i = 0;
    visitLayerGroupDrawables(layerGroup, [&](gfx::Drawable& drawable) {
        if (!drawable.getTileID() || !drawable.getData()) {
            return;
        }

        const auto tileID = drawable.getTileID()->toUnwrapped();
        const auto& symbolData = static_cast<gfx::SymbolDrawableData&>(*drawable.getData());
        const auto isText = (symbolData.symbolType == SymbolType::Text);

        if (isText && (!textPaintBuffer || textPropertiesUpdated)) {
            const auto props = buildPaintUBO(true, evaluated);
            textPaintBuffer = parameters.context.createUniformBuffer(&props, sizeof(props));
            textPropertiesUpdated = false;
        } else if (!isText && (!iconPaintBuffer || iconPropertiesUpdated)) {
            const auto props = buildPaintUBO(false, evaluated);
            iconPaintBuffer = parameters.context.createUniformBuffer(&props, sizeof(props));
            iconPropertiesUpdated = false;
        }
        
        auto& uniforms = drawable.mutableUniformBuffers();
        uniforms.set(idSymbolDrawableUBO, drawableBuffer, i * sizeof(SymbolDrawableUBO));
        uniforms.set(idSymbolDynamicUBO, dynamicBuffer);
        uniforms.set(idSymbolDrawablePaintUBO, isText ? textPaintBuffer : iconPaintBuffer);
        
        i++;
    });
}

} // namespace mbgl
