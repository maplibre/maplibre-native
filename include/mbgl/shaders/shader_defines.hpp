#pragma once

#include <mbgl/shaders/background_layer_ubo.hpp>
#include <mbgl/shaders/circle_layer_ubo.hpp>
#include <mbgl/shaders/collision_layer_ubo.hpp>
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

static constexpr auto maxUBOCountPerShader = std::max({static_cast<size_t>(backgroundUBOCount),
                                                       static_cast<size_t>(circleUBOCount),
                                                       static_cast<size_t>(collisionUBOCount),
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

enum {
    idBackgroundImageTexture,
    backgroundTextureCount
};

enum {
    circleTextureCount
};

enum {
    collisionTextureCount
};

enum {
    idDebugOverlayTexture,
    debugTextureCount
};

enum {
    idFillExtrusionImageTexture,
    fillExtrusionTextureCount
};

enum {
    idFillImageTexture,
    fillTextureCount
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
                                                           static_cast<size_t>(collisionTextureCount),
                                                           static_cast<size_t>(debugTextureCount),
                                                           static_cast<size_t>(fillTextureCount),
                                                           static_cast<size_t>(fillExtrusionTextureCount),
                                                           static_cast<size_t>(heatmapTextureCount),
                                                           static_cast<size_t>(hillshadeTextureCount),
                                                           static_cast<size_t>(lineTextureCount),
                                                           static_cast<size_t>(rasterTextureCount),
                                                           static_cast<size_t>(symbolTextureCount)});

} // namespace shaders
} // namespace mbgl
