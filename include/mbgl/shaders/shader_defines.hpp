#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

#include <algorithm>

namespace mbgl {
namespace shaders {

// layer SSBOs

static constexpr uint32_t layerSSBOStartId = globalUBOCount;

enum {
    idDrawableReservedVertexOnlyUBO = layerSSBOStartId,
    idDrawableReservedFragmentOnlyUBO,
    drawableReservedUBOCount
};

enum {
    idBackgroundDrawableUBO = idDrawableReservedVertexOnlyUBO, // SSBO
    backgroundLayerSSBOCount = drawableReservedUBOCount
};

enum {
    idCircleDrawableUBO = idDrawableReservedVertexOnlyUBO, // SSBO
    circleLayerSSBOCount = drawableReservedUBOCount
};

enum {
    idFillDrawableUBO = idDrawableReservedVertexOnlyUBO, // SSBO
    idFillTilePropsUBO = drawableReservedUBOCount,       // SSBO
    fillLayerSSBOCount
};

enum {
    idFillExtrusionDrawableUBO = idDrawableReservedVertexOnlyUBO, // SSBO
    idFillExtrusionTilePropsUBO = drawableReservedUBOCount,       // both SSBO and UBO?
    fillExtrusionLayerSSBOCount
};

enum {
    idHeatmapDrawableUBO = idDrawableReservedVertexOnlyUBO, // SSBO
    heatmapLayerSSBOCount = drawableReservedUBOCount
};

enum {
    idHillshadeDrawableUBO = idDrawableReservedVertexOnlyUBO, // SSBO
    idHillshadeTilePropsUBO = idDrawableReservedFragmentOnlyUBO, // SSBO
    hillshadeLayerSSBOCount = drawableReservedUBOCount
};

enum {
    idColorReliefDrawableUBO = idDrawableReservedVertexOnlyUBO, // SSBO
    idColorReliefTilePropsUBO = drawableReservedUBOCount,       // SSBO
    colorReliefLayerSSBOCount
};

enum {
    idLineDrawableUBO = idDrawableReservedVertexOnlyUBO, // SSBO
    idLineTilePropsUBO = idDrawableReservedFragmentOnlyUBO, // SSBO
    lineLayerSSBOCount = drawableReservedUBOCount
};

enum {
    idRasterDrawableUBO = idDrawableReservedVertexOnlyUBO, // SSBO
    rasterLayerSSBOCount = drawableReservedUBOCount
};

enum {
    idSymbolDrawableUBO = idDrawableReservedVertexOnlyUBO, // SSBO
    idSymbolTilePropsUBO = idDrawableReservedFragmentOnlyUBO, // SSBO
    symbolLayerSSBOCount = drawableReservedUBOCount
};


// layer UBOs
static constexpr uint32_t layerUBOStartId = std::max({static_cast<uint32_t>(drawableReservedUBOCount),
                                                      static_cast<uint32_t>(backgroundLayerSSBOCount),
                                                      static_cast<uint32_t>(circleLayerSSBOCount),
                                                      static_cast<uint32_t>(fillLayerSSBOCount),
                                                      static_cast<uint32_t>(fillExtrusionLayerSSBOCount),
                                                      static_cast<uint32_t>(heatmapLayerSSBOCount),
                                                      static_cast<uint32_t>(hillshadeLayerSSBOCount),
                                                      static_cast<uint32_t>(colorReliefLayerSSBOCount),
                                                      static_cast<uint32_t>(lineLayerSSBOCount),
                                                      static_cast<uint32_t>(rasterLayerSSBOCount),
                                                      static_cast<uint32_t>(symbolLayerSSBOCount)});

#if MLN_RENDER_BACKEND_VULKAN
#define getEnumStartValue(packed, unpacked) unpacked
#else
#define getEnumStartValue(packed, unpacked) packed
#endif

enum {
    idBackgroundPropsUBO = getEnumStartValue(backgroundLayerSSBOCount, layerUBOStartId),
    backgroundLayerUBOCount
};

enum {
    idCircleEvaluatedPropsUBO = getEnumStartValue(circleLayerSSBOCount, layerUBOStartId),
    circleLayerUBOCount
};

enum {
    idFillEvaluatedPropsUBO = getEnumStartValue(fillLayerSSBOCount, layerUBOStartId),
    fillLayerUBOCount
};

enum {
    idFillExtrusionPropsUBO = getEnumStartValue(fillExtrusionLayerSSBOCount, layerUBOStartId),
    fillExtrusionUBOCount
};

enum {
    idHeatmapEvaluatedPropsUBO = getEnumStartValue(heatmapLayerSSBOCount, layerUBOStartId),
    heatmapLayerUBOCount
};

enum {
    idHeatmapTexturePropsUBO = getEnumStartValue(globalUBOCount, layerUBOStartId),
    heatmapTextureLayerUBOCount
};

enum {
    idHillshadeEvaluatedPropsUBO = getEnumStartValue(hillshadeLayerSSBOCount, layerUBOStartId),
    hillshadeLayerUBOCount
};

enum {
    idColorReliefEvaluatedPropsUBO = getEnumStartValue(colorReliefLayerSSBOCount, layerUBOStartId),
    colorReliefLayerUBOCount
};

enum {
    idLineEvaluatedPropsUBO = getEnumStartValue(lineLayerSSBOCount, layerUBOStartId),
    idLineExpressionUBO,
    lineLayerUBOCount
};

enum {
    idRasterEvaluatedPropsUBO = getEnumStartValue(rasterLayerSSBOCount, layerUBOStartId),
    rasterLayerUBOCount
};

enum {
    idSymbolEvaluatedPropsUBO = getEnumStartValue(symbolLayerSSBOCount, layerUBOStartId),
    symbolLayerUBOCount
};


// drawable SSBOs

static constexpr uint32_t drawableSSBOStartId = std::max({static_cast<uint32_t>(backgroundLayerUBOCount),
                                                          static_cast<uint32_t>(circleLayerUBOCount),
                                                          static_cast<uint32_t>(fillLayerUBOCount),
                                                          static_cast<uint32_t>(heatmapLayerUBOCount),
                                                          static_cast<uint32_t>(heatmapTextureLayerUBOCount),
                                                          static_cast<uint32_t>(hillshadeLayerUBOCount),
                                                          static_cast<uint32_t>(colorReliefLayerUBOCount),
                                                          static_cast<uint32_t>(lineLayerUBOCount),
                                                          static_cast<uint32_t>(rasterLayerUBOCount),
                                                          static_cast<uint32_t>(symbolLayerUBOCount)});

enum {
    idFillExtrusionInstanced = getEnumStartValue(fillExtrusionUBOCount, drawableSSBOStartId),
    fillExtrusionDrawableSSBOCount
};

// drawable UBOs

static constexpr uint32_t drawableUBOStartId = std::max(
    {static_cast<uint32_t>(drawableSSBOStartId), static_cast<uint32_t>(fillExtrusionDrawableSSBOCount)});

enum {
    idClippingMaskUBO = getEnumStartValue(globalUBOCount, drawableUBOStartId),
    clippingMaskDrawableUBOCount
};

enum {
    idCollisionDrawableUBO = getEnumStartValue(globalUBOCount, drawableUBOStartId), // UBO
    idCollisionTilePropsUBO,
    collisionDrawableUBOCount
};

enum {
    idCustomGeometryDrawableUBO = getEnumStartValue(globalUBOCount, drawableUBOStartId), // UBO
    customGeometryDrawableUBOCount
};

enum {
    idCustomSymbolDrawableUBO = getEnumStartValue(globalUBOCount, drawableUBOStartId), // UBO
    customSymbolDrawableUBOCount
};

enum {
    idDebugUBO = getEnumStartValue(globalUBOCount, drawableUBOStartId), // UBO
    debugDrawableUBOCount
};

enum {
    idHillshadePrepareDrawableUBO = getEnumStartValue(globalUBOCount, drawableUBOStartId), // UBO
    idHillshadePrepareTilePropsUBO, // UBO
    hillshadePrepareDrawableUBOCount
};

enum {
    idLocationIndicatorDrawableUBO = getEnumStartValue(globalUBOCount, drawableUBOStartId), // UBO
    locationIndicatorDrawableUBOCount
};

enum {
    idWideVectorUniformsUBO = getEnumStartValue(globalUBOCount, drawableUBOStartId), // UBO
    idWideVectorUniformWideVecUBO, // UBO
    wideVectorDrawableUBOCount
};

#undef getEnumStartValue

static constexpr uint32_t maxUBOCountPerShader = std::max({static_cast<uint32_t>(idClippingMaskUBO),
                                                           static_cast<uint32_t>(collisionDrawableUBOCount),
                                                           static_cast<uint32_t>(customGeometryDrawableUBOCount),
                                                           static_cast<uint32_t>(debugDrawableUBOCount),
                                                           static_cast<uint32_t>(hillshadePrepareDrawableUBOCount),
                                                           static_cast<uint32_t>(locationIndicatorDrawableUBOCount),
                                                           static_cast<uint32_t>(wideVectorDrawableUBOCount)});

static constexpr uint32_t maxSSBOCountPerLayer = layerUBOStartId - layerSSBOStartId;
static constexpr uint32_t maxUBOCountPerLayer = drawableSSBOStartId - layerUBOStartId;

static constexpr uint32_t maxSSBOCountPerDrawable = drawableUBOStartId - drawableSSBOStartId;
static constexpr uint32_t maxUBOCountPerDrawable = maxUBOCountPerShader - drawableUBOStartId;



// Texture defines
enum {
    idBackgroundImageTexture,
    backgroundTextureCount
};

enum {
    circleTextureCount
};

enum {
    clippingMaskTextureCount
};

enum {
    collisionTextureCount
};

enum {
    idCustomGeometryTexture,
    customGeometryTextureCount
};

enum {
    idCustomSymbolImageTexture,
    customSymbolTextureCount
};

enum {
    idDebugOverlayTexture,
    debugTextureCount
};

enum {
    idFillImageTexture,
    fillTextureCount
};

enum {
    idFillExtrusionImageTexture,
    fillExtrusionTextureCount
};

enum {
    idHeatmapImageTexture,
    idHeatmapColorRampTexture,
    heatmapTextureCount
};

enum {
    idHillshadeImageTexture,
    hillshadeTextureCount
};

enum {
    idColorReliefImageTexture,
    idColorReliefElevationStopsTexture,
    idColorReliefColorStopsTexture,
    colorReliefTextureCount
};

enum {
    idLocationIndicatorTexture,
    locationIndicatorTextureCount
};

enum {
    idLineImageTexture,
    lineTextureCount
};

enum {
    idRasterImage0Texture,
    idRasterImage1Texture,
    rasterTextureCount
};

enum {
    idSymbolImageTexture,
    idSymbolImageIconTexture,
    symbolTextureCount
};

static constexpr uint32_t maxTextureCountPerShader = std::max({static_cast<uint32_t>(backgroundTextureCount),
                                                               static_cast<uint32_t>(circleTextureCount),
                                                               static_cast<uint32_t>(clippingMaskTextureCount),
                                                               static_cast<uint32_t>(collisionTextureCount),
                                                               static_cast<uint32_t>(customGeometryTextureCount),
                                                               static_cast<uint32_t>(customSymbolTextureCount),
                                                               static_cast<uint32_t>(debugTextureCount),
                                                               static_cast<uint32_t>(fillTextureCount),
                                                               static_cast<uint32_t>(fillExtrusionTextureCount),
                                                               static_cast<uint32_t>(heatmapTextureCount),
                                                               static_cast<uint32_t>(hillshadeTextureCount),
                                                               static_cast<uint32_t>(colorReliefTextureCount),
                                                               static_cast<uint32_t>(lineTextureCount),
                                                               static_cast<uint32_t>(locationIndicatorTextureCount),
                                                               static_cast<uint32_t>(rasterTextureCount),
                                                               static_cast<uint32_t>(symbolTextureCount)});

// Vertex attribute defines
enum {
    idBackgroundPosVertexAttribute,
    backgroundVertexAttributeCount
};

enum {
    idCirclePosVertexAttribute,

    // Data driven
    idCircleColorVertexAttribute,
    idCircleRadiusVertexAttribute,
    idCircleBlurVertexAttribute,
    idCircleOpacityVertexAttribute,
    idCircleStrokeColorVertexAttribute,
    idCircleStrokeWidthVertexAttribute,
    idCircleStrokeOpacityVertexAttribute,

    circleVertexAttributeCount
};

enum {
    idClippingMaskPosVertexAttribute,
    clippingMaskVertexAttributeCount
};

enum {
    idCollisionPosVertexAttribute,
    idCollisionAnchorPosVertexAttribute,
    idCollisionExtrudeVertexAttribute,
    idCollisionPlacedVertexAttribute,
    idCollisionShiftVertexAttribute,
    collisionVertexAttributeCount
};

enum {
    idCustomGeometryPosVertexAttribute,
    idCustomGeometryTexVertexAttribute,
    customGeometryVertexAttributeCount
};

enum {
    idCustomSymbolPosVertexAttribute,
    idCustomSymbolTexVertexAttribute,
    customSymbolVertexAttributeCount
};

enum {
    idDebugPosVertexAttribute,
    debugVertexAttributeCount
};

enum {
    idFillPosVertexAttribute,

    // Data driven
    idFillColorVertexAttribute,
    idFillOpacityVertexAttribute,
    idFillOutlineColorVertexAttribute,
    idFillPatternFromVertexAttribute,
    idFillPatternToVertexAttribute,

    fillVertexAttributeCount
};

enum {
    idFillExtrusionPosVertexAttribute,

#if MLN_USE_FILL_EXTRUSION_INSTANCING
    idFillExtrusionOutlinePosAttribute,
    idFillExtrusionEdDiscardAttribute,
#else
    idFillExtrusionNormalEdVertexAttribute,
#endif

    // Data driven
    idFillExtrusionBaseVertexAttribute,
    idFillExtrusionColorVertexAttribute,
    idFillExtrusionHeightVertexAttribute,
    idFillExtrusionPatternFromVertexAttribute,
    idFillExtrusionPatternToVertexAttribute,

    fillExtrusionVertexAttributeCount
};

enum {
    idHeatmapPosVertexAttribute,

    // Data driven
    idHeatmapWeightVertexAttribute,
    idHeatmapRadiusVertexAttribute,

    heatmapVertexAttributeCount
};

enum {
    idHillshadePosVertexAttribute,
    idHillshadeTexturePosVertexAttribute,
    hillshadeVertexAttributeCount
};

enum {
    idColorReliefPosVertexAttribute,
    idColorReliefTexturePosVertexAttribute,
    colorReliefVertexAttributeCount
};

enum {
    idLinePosNormalVertexAttribute,
    idLineDataVertexAttribute,

    // Data driven
    idLineColorVertexAttribute,
    idLineBlurVertexAttribute,
    idLineOpacityVertexAttribute,
    idLineGapWidthVertexAttribute,
    idLineOffsetVertexAttribute,
    idLineWidthVertexAttribute,
    idLineFloorWidthVertexAttribute,
    idLinePatternFromVertexAttribute,
    idLinePatternToVertexAttribute,

    lineVertexAttributeCount
};

enum {
    idLocationIndicatorPosVertexAttribute,
    idLocationIndicatorTexVertexAttribute,
    locationIndicatorVertexAttributeCount
};

enum {
    idRasterPosVertexAttribute,
    idRasterTexturePosVertexAttribute,
    rasterVertexAttributeCount
};

enum {
    idSymbolPosOffsetVertexAttribute,
    idSymbolDataVertexAttribute,
    idSymbolPixelOffsetVertexAttribute,
    idSymbolProjectedPosVertexAttribute,
    idSymbolFadeOpacityVertexAttribute,

    // Data driven
    idSymbolOpacityVertexAttribute,
    idSymbolColorVertexAttribute,
    idSymbolHaloColorVertexAttribute,
    idSymbolHaloWidthVertexAttribute,
    idSymbolHaloBlurVertexAttribute,

    symbolVertexAttributeCount
};

enum {
    idWideVectorScreenPos,
    idWideVectorColor,
    idWideVectorIndex,

    wideVectorAttributeCount
};

enum {
    idWideVectorInstanceCenter,
    idWideVectorInstanceColor,
    idWideVectorInstancePrevious,
    idWideVectorInstanceNext,

    wideVectorInstanceAttributeCount
};

static constexpr uint32_t maxVertexAttributeCountPerShader = std::max({
    static_cast<uint32_t>(backgroundVertexAttributeCount),
    static_cast<uint32_t>(circleVertexAttributeCount),
    static_cast<uint32_t>(clippingMaskVertexAttributeCount),
    static_cast<uint32_t>(collisionVertexAttributeCount),
    static_cast<uint32_t>(customGeometryVertexAttributeCount),
    static_cast<uint32_t>(customSymbolVertexAttributeCount),
    static_cast<uint32_t>(debugVertexAttributeCount),
    static_cast<uint32_t>(fillVertexAttributeCount),
    static_cast<uint32_t>(fillExtrusionVertexAttributeCount),
    static_cast<uint32_t>(heatmapVertexAttributeCount),
    static_cast<uint32_t>(hillshadeVertexAttributeCount),
    static_cast<uint32_t>(colorReliefVertexAttributeCount),
    static_cast<uint32_t>(lineVertexAttributeCount),
    static_cast<uint32_t>(locationIndicatorVertexAttributeCount),
    static_cast<uint32_t>(rasterVertexAttributeCount),
    static_cast<uint32_t>(symbolVertexAttributeCount),
    static_cast<uint32_t>(wideVectorAttributeCount),
    static_cast<uint32_t>(wideVectorInstanceAttributeCount),
});

} // namespace shaders
} // namespace mbgl
