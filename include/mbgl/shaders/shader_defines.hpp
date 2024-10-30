#pragma once

#include <mbgl/shaders/layer_ubo.hpp>

#include <algorithm>

namespace mbgl {
namespace shaders {

// drawable UBOs

enum {
    idClippingMaskUBO = globalUBOCount,
    clippingMaskUBOCount
};

enum {
    idCommonUBO = globalUBOCount,
    commonDrawableUBOCount
};

enum {
    idCircleDrawableUBO = globalUBOCount,
    idCircleInterpolateUBO,
    circleDrawableUBOCount
};

enum {
    idBackgroundDrawableUBO = globalUBOCount,
    backgroundDrawableUBOCount
};

enum {
    idCustomSymbolDrawableUBO = globalUBOCount,
    idCustomSymbolParametersUBO,
    customSymbolDrawableUBOCount
};

enum {
    idCollisionUBO = globalUBOCount,
    collisionDrawableUBOCount
};

enum {
    idDebugUBO = globalUBOCount,
    debugDrawableUBOCount
};

enum {
    idFillDrawableUBO = globalUBOCount,
    idFillTilePropsUBO,
    idFillInterpolateUBO,
    fillDrawableUBOCount
};

enum {
    idFillExtrusionDrawableUBO = globalUBOCount,
    idFillExtrusionTilePropsUBO,
    idFillExtrusionInterpolateUBO,
    fillExtrusionDrawableUBOCount
};

enum {
    idHeatmapDrawableUBO = globalUBOCount,
    idHeatmapInterpolateUBO,
    heatmapDrawableUBOCount
};

enum {
    idHillshadeDrawableUBO = globalUBOCount,
    hillshadeDrawableUBOCount
};

enum {
    idHillshadePrepareDrawableUBO = globalUBOCount,
    hillshadePrepareDrawableUBOCount
};

enum {
    idLineDrawableUBO = globalUBOCount,
    idLineInterpolationUBO,
    idLineTilePropertiesUBO,
    lineDrawableUBOCount
};

enum {
    idRasterDrawableUBO = globalUBOCount,
    rasterDrawableUBOCount
};

enum {
    idSymbolDrawableUBO = globalUBOCountWithIndex,
    idSymbolTilePropsUBO,
    idSymbolInterpolateUBO,
    symbolDrawableUBOCount
};

enum {
    idWideVectorUniformsUBO = globalUBOCount,
    idWideVectorUniformWideVecUBO,
    wideVectorDrawableUBOCount
};

static constexpr auto layerUBOStartId = std::max({static_cast<size_t>(circleDrawableUBOCount),
                                                  static_cast<size_t>(backgroundDrawableUBOCount),
                                                  static_cast<size_t>(customSymbolDrawableUBOCount),
                                                  static_cast<size_t>(collisionDrawableUBOCount),
                                                  static_cast<size_t>(commonDrawableUBOCount),
                                                  static_cast<size_t>(debugDrawableUBOCount),
                                                  static_cast<size_t>(fillDrawableUBOCount),
                                                  static_cast<size_t>(fillExtrusionDrawableUBOCount),
                                                  static_cast<size_t>(heatmapDrawableUBOCount),
                                                  static_cast<size_t>(hillshadeDrawableUBOCount),
                                                  static_cast<size_t>(hillshadePrepareDrawableUBOCount),
                                                  static_cast<size_t>(lineDrawableUBOCount),
                                                  static_cast<size_t>(rasterDrawableUBOCount),
                                                  static_cast<size_t>(symbolDrawableUBOCount),
                                                  static_cast<size_t>(wideVectorDrawableUBOCount)});

static constexpr auto maxUBOCountPerDrawable = layerUBOStartId - globalUBOCount;

// layer UBOs

#if MLN_RENDER_BACKEND_VULKAN
#define getLayerStartValue(packedValue) layerUBOStartId
#else
#define getLayerStartValue(packedValue) packedValue
#endif

enum {
    idCircleEvaluatedPropsUBO = getLayerStartValue(circleDrawableUBOCount),
    circleUBOCount
};

enum {
    idBackgroundLayerUBO = getLayerStartValue(backgroundDrawableUBOCount),
    backgroundUBOCount
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
    idHeatmapTexturePropsUBO = getLayerStartValue(globalUBOCount),
    heatmapTextureUBOCount
};

enum {
    idHillshadeEvaluatedPropsUBO = getLayerStartValue(hillshadeDrawableUBOCount),
    hillshadeUBOCount
};

enum {
    idLineEvaluatedPropsUBO = getLayerStartValue(lineDrawableUBOCount),
    idLineExpressionUBO,
    lineUBOCount
};

enum {
    idRasterEvaluatedPropsUBO = getLayerStartValue(rasterDrawableUBOCount),
    rasterUBOCount
};

enum {
    idSymbolEvaluatedPropsUBO = getLayerStartValue(symbolDrawableUBOCount),
    symbolUBOCount
};

#undef getLayerStartValue

static constexpr auto maxUBOCountPerShader = std::max({static_cast<size_t>(circleUBOCount),
                                                       static_cast<size_t>(clippingMaskUBOCount),
                                                       static_cast<size_t>(fillUBOCount),
                                                       static_cast<size_t>(fillExtrusionUBOCount),
                                                       static_cast<size_t>(heatmapUBOCount),
                                                       static_cast<size_t>(heatmapTextureUBOCount),
                                                       static_cast<size_t>(hillshadeUBOCount),
                                                       static_cast<size_t>(lineUBOCount),
                                                       static_cast<size_t>(rasterUBOCount),
                                                       static_cast<size_t>(symbolUBOCount)});

static constexpr auto maxUBOCountPerLayer = maxUBOCountPerShader - layerUBOStartId;

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
    idCommonTexture,
    commonTextureCount
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

static constexpr auto maxTextureCountPerShader = std::max({static_cast<size_t>(backgroundTextureCount),
                                                           static_cast<size_t>(circleTextureCount),
                                                           static_cast<size_t>(clippingMaskTextureCount),
                                                           static_cast<size_t>(collisionTextureCount),
                                                           static_cast<size_t>(commonTextureCount),
                                                           static_cast<size_t>(customSymbolTextureCount),
                                                           static_cast<size_t>(debugTextureCount),
                                                           static_cast<size_t>(fillTextureCount),
                                                           static_cast<size_t>(fillExtrusionTextureCount),
                                                           static_cast<size_t>(heatmapTextureCount),
                                                           static_cast<size_t>(hillshadeTextureCount),
                                                           static_cast<size_t>(lineTextureCount),
                                                           static_cast<size_t>(rasterTextureCount),
                                                           static_cast<size_t>(symbolTextureCount)});

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
    idCommonPosVertexAttribute,
    idCommonTexVertexAttribute,
    commonVertexAttributeCount
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

static constexpr auto maxVertexAttributeCountPerShader = std::max({
    static_cast<size_t>(backgroundVertexAttributeCount),
    static_cast<size_t>(circleVertexAttributeCount),
    static_cast<size_t>(clippingMaskVertexAttributeCount),
    static_cast<size_t>(collisionVertexAttributeCount),
    static_cast<size_t>(commonVertexAttributeCount),
    static_cast<size_t>(customSymbolVertexAttributeCount),
    static_cast<size_t>(debugVertexAttributeCount),
    static_cast<size_t>(fillVertexAttributeCount),
    static_cast<size_t>(fillExtrusionVertexAttributeCount),
    static_cast<size_t>(heatmapVertexAttributeCount),
    static_cast<size_t>(hillshadeVertexAttributeCount),
    static_cast<size_t>(lineVertexAttributeCount),
    static_cast<size_t>(rasterVertexAttributeCount),
    static_cast<size_t>(symbolVertexAttributeCount),
    static_cast<size_t>(wideVectorAttributeCount),
    static_cast<size_t>(wideVectorInstanceAttributeCount),
});

} // namespace shaders
} // namespace mbgl
