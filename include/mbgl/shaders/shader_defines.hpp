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
    idColorReliefDrawableUBO = idDrawableReservedVertexOnlyUBO, // SSBO
    idColorReliefTilePropsUBO = drawableReservedUBOCount,       // SSBO
    colorReliefLayerSSBOCount
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
    idHillshadeDrawableUBO = idDrawableReservedVertexOnlyUBO,    // SSBO
    idHillshadeTilePropsUBO = idDrawableReservedFragmentOnlyUBO, // SSBO
    hillshadeLayerSSBOCount = drawableReservedUBOCount
};

enum {
    idLineDrawableUBO = idDrawableReservedVertexOnlyUBO,    // SSBO
    idLineTilePropsUBO = idDrawableReservedFragmentOnlyUBO, // SSBO
    lineLayerSSBOCount = drawableReservedUBOCount
};

enum {
    idRasterDrawableUBO = idDrawableReservedVertexOnlyUBO, // SSBO
    rasterLayerSSBOCount = drawableReservedUBOCount
};

enum {
    idSymbolDrawableUBO = idDrawableReservedVertexOnlyUBO,    // SSBO
    idSymbolTilePropsUBO = idDrawableReservedFragmentOnlyUBO, // SSBO
    symbolLayerSSBOCount = drawableReservedUBOCount
};

enum {
    idTerrainDrawableUBO = idDrawableReservedVertexOnlyUBO,    // SSBO
    idTerrainTilePropsUBO = idDrawableReservedFragmentOnlyUBO, // SSBO
    terrainLayerSSBOCount = drawableReservedUBOCount
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
                                                      static_cast<uint32_t>(symbolLayerSSBOCount),
                                                      static_cast<uint32_t>(terrainLayerSSBOCount)});

#if MLN_RENDER_BACKEND_VULKAN
#define getEnumValue(packed, unpacked) unpacked
#else
#define getEnumValue(packed, unpacked) packed
#endif

enum {
    idBackgroundPropsUBO = getEnumValue(backgroundLayerSSBOCount, layerUBOStartId),
    backgroundLayerUBOCount
};

enum {
    idCircleEvaluatedPropsUBO = getEnumValue(circleLayerSSBOCount, layerUBOStartId),
    circleLayerUBOCount
};

enum {
    idColorReliefEvaluatedPropsUBO = getEnumValue(colorReliefLayerSSBOCount, layerUBOStartId),
    colorReliefLayerUBOCount
};

enum {
    idFillEvaluatedPropsUBO = getEnumValue(fillLayerSSBOCount, layerUBOStartId),
    fillLayerUBOCount
};

enum {
    idFillExtrusionPropsUBO = getEnumValue(fillExtrusionLayerSSBOCount, layerUBOStartId),
    fillExtrusionLayerUBOCount
};

enum {
    idHeatmapEvaluatedPropsUBO = getEnumValue(heatmapLayerSSBOCount, layerUBOStartId),
    heatmapLayerUBOCount
};

enum {
    idHeatmapTexturePropsUBO = getEnumValue(drawableReservedUBOCount, layerUBOStartId),
    heatmapTextureUBOCount
};

enum {
    idHillshadeEvaluatedPropsUBO = getEnumValue(hillshadeLayerSSBOCount, layerUBOStartId),
    hillshadeLayerUBOCount
};

enum {
    idLineEvaluatedPropsUBO = getEnumValue(lineLayerSSBOCount, layerUBOStartId),
    idLineExpressionUBO,
    lineLayerUBOCount
};

enum {
    idRasterEvaluatedPropsUBO = getEnumValue(rasterLayerSSBOCount, layerUBOStartId),
    rasterLayerUBOCount
};

enum {
    idSymbolEvaluatedPropsUBO = getEnumValue(symbolLayerSSBOCount, layerUBOStartId),
    symbolLayerUBOCount
};

enum {
    idTerrainEvaluatedPropsUBO = getEnumValue(terrainLayerSSBOCount, layerUBOStartId),
    terrainLayerUBOCount
};

// drawable SSBOs

static constexpr uint32_t drawableSSBOStartId = std::max({static_cast<uint32_t>(backgroundLayerUBOCount),
                                                          static_cast<uint32_t>(circleLayerUBOCount),
                                                          static_cast<uint32_t>(colorReliefLayerUBOCount),
                                                          static_cast<uint32_t>(fillLayerUBOCount),
                                                          static_cast<uint32_t>(fillExtrusionLayerUBOCount),
                                                          static_cast<uint32_t>(heatmapLayerUBOCount),
                                                          static_cast<uint32_t>(hillshadeLayerUBOCount),
                                                          static_cast<uint32_t>(lineLayerUBOCount),
                                                          static_cast<uint32_t>(rasterLayerUBOCount),
                                                          static_cast<uint32_t>(symbolLayerUBOCount),
                                                          static_cast<uint32_t>(terrainLayerUBOCount)});

enum {
#if MLN_USE_FILL_EXTRUSION_INSTANCING
    idFillExtrusionInstanced = getEnumValue(fillExtrusionLayerUBOCount, drawableSSBOStartId),
#endif
    fillExtrusionDrawableSSBOCount
};

// drawable UBOs

static constexpr uint32_t drawableUBOStartId = std::max(
    {static_cast<uint32_t>(drawableSSBOStartId), static_cast<uint32_t>(fillExtrusionDrawableSSBOCount)});

enum {
    backgroundUBOCount = getEnumValue(backgroundLayerUBOCount, drawableUBOStartId)
};

enum {
    circleUBOCount = getEnumValue(circleLayerUBOCount, drawableUBOStartId)
};

enum {
    idCollisionDrawableUBO = getEnumValue(idDrawableReservedVertexOnlyUBO, drawableUBOStartId), // UBO
    idCollisionTilePropsUBO = getEnumValue(drawableReservedUBOCount, idCollisionDrawableUBO + 1),
    collisionUBOCount
};

enum {
    idClippingMaskUBO = getEnumValue(idDrawableReservedVertexOnlyUBO, drawableUBOStartId),
    clippingMaskUBOCount = getEnumValue(drawableReservedUBOCount, idClippingMaskUBO + 1)
};

enum {
    colorReliefUBOCount = getEnumValue(colorReliefLayerUBOCount, drawableUBOStartId)
};

enum {
    idCustomGeometryDrawableUBO = getEnumValue(drawableReservedUBOCount, drawableUBOStartId), // UBO
    customGeometryUBOCount
};

enum {
    idCustomSymbolDrawableUBO = getEnumValue(idDrawableReservedVertexOnlyUBO, drawableUBOStartId), // UBO
    customSymbolUBOCount = getEnumValue(drawableReservedUBOCount, idCustomSymbolDrawableUBO + 1)
};

enum {
    idDebugUBO = getEnumValue(drawableReservedUBOCount, drawableUBOStartId), // UBO
    debugUBOCount
};

enum {
    fillUBOCount = getEnumValue(fillLayerUBOCount, drawableUBOStartId)
};

enum {
    fillExtrusionUBOCount = getEnumValue(fillExtrusionLayerUBOCount, drawableUBOStartId)
};

enum {
    heatmapUBOCount = getEnumValue(heatmapLayerUBOCount, drawableUBOStartId)
};

enum {
    idHillshadePrepareDrawableUBO = getEnumValue(idDrawableReservedVertexOnlyUBO, drawableUBOStartId),          // UBO
    idHillshadePrepareTilePropsUBO = getEnumValue(drawableReservedUBOCount, idHillshadePrepareDrawableUBO + 1), // UBO
    hillshadePrepareUBOCount
};

enum {
    hillshadeUBOCount = getEnumValue(hillshadeLayerUBOCount, drawableUBOStartId)
};

enum {
    lineUBOCount = getEnumValue(lineLayerUBOCount, drawableUBOStartId)
};

enum {
    idLocationIndicatorDrawableUBO = getEnumValue(drawableReservedUBOCount, drawableUBOStartId), // UBO
    locationIndicatorUBOCount
};

enum {
    rasterUBOCount = getEnumValue(rasterLayerUBOCount, drawableUBOStartId)
};

enum {
    symbolUBOCount = getEnumValue(symbolLayerUBOCount, drawableUBOStartId)
};

enum {
    terrainUBOCount = getEnumValue(terrainLayerUBOCount, drawableUBOStartId)
};

enum {
    idWideVectorUniformsUBO = getEnumValue(idDrawableReservedVertexOnlyUBO, drawableUBOStartId),         // UBO
    idWideVectorUniformWideVecUBO = getEnumValue(drawableReservedUBOCount, idWideVectorUniformsUBO + 1), // UBO
    wideVectorUBOCount
};

#undef getEnumValue

static constexpr uint32_t maxUBOCountPerShader = std::max({static_cast<uint32_t>(backgroundUBOCount),
                                                           static_cast<uint32_t>(circleUBOCount),
                                                           static_cast<uint32_t>(clippingMaskUBOCount),
                                                           static_cast<uint32_t>(collisionUBOCount),
                                                           static_cast<uint32_t>(colorReliefUBOCount),
                                                           static_cast<uint32_t>(customGeometryUBOCount),
                                                           static_cast<uint32_t>(debugUBOCount),
                                                           static_cast<uint32_t>(fillUBOCount),
                                                           static_cast<uint32_t>(fillExtrusionUBOCount),
                                                           static_cast<uint32_t>(heatmapTextureUBOCount),
                                                           static_cast<uint32_t>(heatmapUBOCount),
                                                           static_cast<uint32_t>(hillshadePrepareUBOCount),
                                                           static_cast<uint32_t>(hillshadeUBOCount),
                                                           static_cast<uint32_t>(lineUBOCount),
                                                           static_cast<uint32_t>(locationIndicatorUBOCount),
                                                           static_cast<uint32_t>(rasterUBOCount),
                                                           static_cast<uint32_t>(symbolUBOCount),
                                                           static_cast<uint32_t>(terrainUBOCount),
                                                           static_cast<uint32_t>(wideVectorUBOCount)});

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
    idCircleDEMTexture,
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

enum {
    idTerrainDEMTexture,
    idTerrainMapTexture,
    terrainTextureCount
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
                                                               static_cast<uint32_t>(symbolTextureCount),
                                                               static_cast<uint32_t>(terrainTextureCount)});

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
    idTerrainPosVertexAttribute,
    idTerrainTexturePosVertexAttribute,
    terrainVertexAttributeCount
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
    static_cast<uint32_t>(terrainVertexAttributeCount),
    static_cast<uint32_t>(wideVectorAttributeCount),
    static_cast<uint32_t>(wideVectorInstanceAttributeCount),
});

} // namespace shaders
} // namespace mbgl
