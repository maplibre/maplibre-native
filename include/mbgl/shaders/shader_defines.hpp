#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

#include <algorithm>

namespace mbgl {
namespace shaders {

// drawable UBOs

enum {
    idBackgroundDrawableUBO = idDrawableReservedVertexOnlyUBO,
    backgroundDrawableUBOCount = drawableReservedUBOCount
};

enum {
    idCircleDrawableUBO = idDrawableReservedVertexOnlyUBO,
    circleDrawableUBOCount = drawableReservedUBOCount
};

enum {
    idClippingMaskUBO = idDrawableReservedVertexOnlyUBO,
    clippingMaskDrawableUBOCount = drawableReservedUBOCount
};

enum {
    idCollisionDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idCollisionTilePropsUBO = drawableReservedUBOCount,
    collisionDrawableUBOCount
};

enum {
    idCustomGeometryDrawableUBO = drawableReservedUBOCount,
    customGeometryDrawableUBOCount
};

enum {
    idCustomSymbolDrawableUBO = idDrawableReservedVertexOnlyUBO,
    customSymbolDrawableUBOCount = drawableReservedUBOCount
};

enum {
    idDebugUBO = drawableReservedUBOCount,
    debugDrawableUBOCount
};

enum {
    idFillDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idFillTilePropsUBO = drawableReservedUBOCount,
    fillDrawableUBOCount
};

enum {
    idFillExtrusionDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idFillExtrusionTilePropsUBO = drawableReservedUBOCount,
    fillExtrusionDrawableUBOCount
};

enum {
    idHeatmapDrawableUBO = idDrawableReservedVertexOnlyUBO,
    heatmapDrawableUBOCount = drawableReservedUBOCount
};

enum {
    heatmapTextureDrawableUBOCount = drawableReservedUBOCount
};

enum {
    idHillshadeDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idHillshadeTilePropsUBO = idDrawableReservedFragmentOnlyUBO,
    hillshadeDrawableUBOCount = drawableReservedUBOCount
};

enum {
    idHillshadePrepareDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idHillshadePrepareTilePropsUBO = drawableReservedUBOCount,
    hillshadePrepareDrawableUBOCount
};

enum {
    idColorReliefDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idColorReliefTilePropsUBO = drawableReservedUBOCount,
    colorReliefDrawableUBOCount
};

enum {
    idLineDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idLineTilePropsUBO = idDrawableReservedFragmentOnlyUBO,
    lineDrawableUBOCount = drawableReservedUBOCount
};

enum {
    idLocationIndicatorDrawableUBO = drawableReservedUBOCount,
    locationIndicatorDrawableUBOCount
};

enum {
    idRasterDrawableUBO = idDrawableReservedVertexOnlyUBO,
    rasterDrawableUBOCount = drawableReservedUBOCount
};

enum {
    idSymbolDrawableUBO = idDrawableReservedVertexOnlyUBO,
    idSymbolTilePropsUBO = idDrawableReservedFragmentOnlyUBO,
    symbolDrawableUBOCount = drawableReservedUBOCount
};

enum {
    idWideVectorUniformsUBO = idDrawableReservedVertexOnlyUBO,
    idWideVectorUniformWideVecUBO = drawableReservedUBOCount,
    wideVectorDrawableUBOCount
};

static constexpr uint32_t layerSSBOStartId = globalUBOCount;
static constexpr uint32_t layerUBOStartId = std::max({static_cast<uint32_t>(backgroundDrawableUBOCount),
                                                      static_cast<uint32_t>(circleDrawableUBOCount),
                                                      static_cast<uint32_t>(clippingMaskDrawableUBOCount),
                                                      static_cast<uint32_t>(collisionDrawableUBOCount),
                                                      static_cast<uint32_t>(customGeometryDrawableUBOCount),
                                                      static_cast<uint32_t>(customSymbolDrawableUBOCount),
                                                      static_cast<uint32_t>(debugDrawableUBOCount),
                                                      static_cast<uint32_t>(fillDrawableUBOCount),
                                                      static_cast<uint32_t>(fillExtrusionDrawableUBOCount),
                                                      static_cast<uint32_t>(heatmapDrawableUBOCount),
                                                      static_cast<uint32_t>(heatmapTextureDrawableUBOCount),
                                                      static_cast<uint32_t>(hillshadeDrawableUBOCount),
                                                      static_cast<uint32_t>(hillshadePrepareDrawableUBOCount),
                                                      static_cast<uint32_t>(colorReliefDrawableUBOCount),
                                                      static_cast<uint32_t>(lineDrawableUBOCount),
                                                      static_cast<uint32_t>(locationIndicatorDrawableUBOCount),
                                                      static_cast<uint32_t>(rasterDrawableUBOCount),
                                                      static_cast<uint32_t>(symbolDrawableUBOCount),
                                                      static_cast<uint32_t>(wideVectorDrawableUBOCount)});

static constexpr uint32_t maxUBOCountPerDrawable = layerUBOStartId - globalUBOCount;

// layer UBOs

#if MLN_RENDER_BACKEND_VULKAN
#define getLayerStartValue(packedValue) layerUBOStartId
#else
#define getLayerStartValue(packedValue) packedValue
#endif

enum {
    idBackgroundPropsUBO = getLayerStartValue(backgroundDrawableUBOCount),
    backgroundUBOCount
};

enum {
    idCircleEvaluatedPropsUBO = getLayerStartValue(circleDrawableUBOCount),
    circleUBOCount
};

enum {
    clippingMaskUBOCount = getLayerStartValue(clippingMaskDrawableUBOCount)
};

enum {
    collisionUBOCount = getLayerStartValue(collisionDrawableUBOCount)
};

enum {
    customGeometryUBOCount = getLayerStartValue(customGeometryDrawableUBOCount)
};

enum {
    customSymbolUBOCount = getLayerStartValue(customSymbolDrawableUBOCount)
};

enum {
    debugUBOCount = getLayerStartValue(debugDrawableUBOCount)
};

enum {
    idFillEvaluatedPropsUBO = getLayerStartValue(fillDrawableUBOCount),
    fillUBOCount
};

enum {
    idFillExtrusionPropsUBO = getLayerStartValue(fillExtrusionDrawableUBOCount),
    fillExtrusionUBOCount
};

enum {
    idHeatmapEvaluatedPropsUBO = getLayerStartValue(heatmapDrawableUBOCount),
    heatmapUBOCount
};

enum {
    idHeatmapTexturePropsUBO = getLayerStartValue(heatmapTextureDrawableUBOCount),
    heatmapTextureUBOCount
};

enum {
    idHillshadeEvaluatedPropsUBO = getLayerStartValue(hillshadeDrawableUBOCount),
    hillshadeUBOCount
};

enum {
    hillshadePrepareUBOCount = getLayerStartValue(hillshadePrepareDrawableUBOCount)
};

enum {
    idColorReliefEvaluatedPropsUBO = getLayerStartValue(colorReliefDrawableUBOCount),
    colorReliefUBOCount
};

enum {
    idLineEvaluatedPropsUBO = getLayerStartValue(lineDrawableUBOCount),
    idLineExpressionUBO,
    lineUBOCount
};

enum {
    locationIndicatorUBOCount = getLayerStartValue(locationIndicatorDrawableUBOCount)
};

enum {
    idRasterEvaluatedPropsUBO = getLayerStartValue(rasterDrawableUBOCount),
    rasterUBOCount
};

enum {
    idSymbolEvaluatedPropsUBO = getLayerStartValue(symbolDrawableUBOCount),
    symbolUBOCount
};

enum {
    wideVectorUBOCount = getLayerStartValue(wideVectorDrawableUBOCount)
};

#undef getLayerStartValue

static constexpr uint32_t maxUBOCountPerShader = std::max({static_cast<uint32_t>(backgroundUBOCount),
                                                           static_cast<uint32_t>(circleUBOCount),
                                                           static_cast<uint32_t>(clippingMaskUBOCount),
                                                           static_cast<uint32_t>(collisionUBOCount),
                                                           static_cast<uint32_t>(customSymbolUBOCount),
                                                           static_cast<uint32_t>(debugUBOCount),
                                                           static_cast<uint32_t>(fillUBOCount),
                                                           static_cast<uint32_t>(fillExtrusionUBOCount),
                                                           static_cast<uint32_t>(heatmapUBOCount),
                                                           static_cast<uint32_t>(heatmapTextureUBOCount),
                                                           static_cast<uint32_t>(hillshadeUBOCount),
                                                           static_cast<uint32_t>(hillshadePrepareUBOCount),
                                                           static_cast<uint32_t>(colorReliefUBOCount),
                                                           static_cast<uint32_t>(lineUBOCount),
                                                           static_cast<uint32_t>(locationIndicatorUBOCount),
                                                           static_cast<uint32_t>(rasterUBOCount),
                                                           static_cast<uint32_t>(symbolUBOCount),
                                                           static_cast<uint32_t>(wideVectorUBOCount)});

static constexpr uint32_t maxSSBOCountPerLayer = maxUBOCountPerDrawable;
static constexpr uint32_t maxUBOCountPerLayer = maxUBOCountPerShader - layerUBOStartId;

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
    idFillExtrusionNormalEdVertexAttribute,

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
