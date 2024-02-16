#pragma once

#include <mbgl/shaders/background_layer_ubo.hpp>
#include <mbgl/shaders/circle_layer_ubo.hpp>
#include <mbgl/shaders/collision_layer_ubo.hpp>
#include <mbgl/shaders/custom_drawable_layer_ubo.hpp>
#include <mbgl/shaders/debug_layer_ubo.hpp>
#include <mbgl/shaders/fill_layer_ubo.hpp>
#include <mbgl/shaders/fill_extrusion_layer_ubo.hpp>
#include <mbgl/shaders/heatmap_layer_ubo.hpp>
#include <mbgl/shaders/heatmap_texture_layer_ubo.hpp>
#include <mbgl/shaders/hillshade_layer_ubo.hpp>
#include <mbgl/shaders/hillshade_prepare_layer_ubo.hpp>
#include <mbgl/shaders/line_layer_ubo.hpp>
#include <mbgl/shaders/raster_layer_ubo.hpp>
#include <mbgl/shaders/symbol_layer_ubo.hpp>

#include <algorithm>

namespace mbgl {
namespace shaders {

// UBO defines
enum {
    idClippingMaskUBO,
    clippingMaskUBOCount
};

static constexpr auto maxUBOCountPerShader = std::max({static_cast<size_t>(backgroundUBOCount),
                                                       static_cast<size_t>(circleUBOCount),
                                                       static_cast<size_t>(clippingMaskUBOCount),
                                                       static_cast<size_t>(collisionUBOCount),
                                                       static_cast<size_t>(customDrawableUBOCount),
                                                       static_cast<size_t>(debugUBOCount),
                                                       static_cast<size_t>(fillUBOCount),
                                                       static_cast<size_t>(fillOutlineUBOCount),
                                                       static_cast<size_t>(fillPatternUBOCount),
                                                       static_cast<size_t>(fillOutlinePatternUBOCount),
                                                       static_cast<size_t>(fillExtrusionUBOCount),
                                                       static_cast<size_t>(heatmapUBOCount),
                                                       static_cast<size_t>(heatmapTextureUBOCount),
                                                       static_cast<size_t>(hillshadeUBOCount),
                                                       static_cast<size_t>(hillshadePrepareUBOCount),
                                                       static_cast<size_t>(lineUBOCount),
                                                       static_cast<size_t>(lineGradientUBOCount),
                                                       static_cast<size_t>(linePatternUBOCount),
                                                       static_cast<size_t>(lineSDFUBOCount),
                                                       static_cast<size_t>(lineBasicUBOCount),
                                                       static_cast<size_t>(rasterUBOCount),
                                                       static_cast<size_t>(symbolUBOCount)});

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
    idCustomSymbolIconTexture,
    customSymbolIconTextureCount
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
                                                           static_cast<size_t>(customSymbolIconTextureCount),
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

static constexpr auto maxVertexAttributeCountPerShader = std::max(
    {static_cast<size_t>(backgroundVertexAttributeCount),
     static_cast<size_t>(circleVertexAttributeCount),
     static_cast<size_t>(clippingMaskVertexAttributeCount),
     static_cast<size_t>(collisionVertexAttributeCount),
     static_cast<size_t>(debugVertexAttributeCount),
     static_cast<size_t>(fillVertexAttributeCount),
     static_cast<size_t>(fillExtrusionVertexAttributeCount),
     static_cast<size_t>(heatmapVertexAttributeCount),
     static_cast<size_t>(hillshadeVertexAttributeCount),
     static_cast<size_t>(lineVertexAttributeCount),
     static_cast<size_t>(rasterVertexAttributeCount),
     static_cast<size_t>(symbolVertexAttributeCount)});

} // namespace shaders
} // namespace mbgl
