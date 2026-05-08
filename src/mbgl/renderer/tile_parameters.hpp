#pragma once

#include <mbgl/map/mode.hpp>
#include <mbgl/actor/scheduler.hpp>

#include <functional>
#include <memory>
#include <numbers>
#include <string>

#include <mapbox/std/weak.hpp>

namespace mbgl {

class RenderSource;
class TransformState;
class FileSource;
class AnnotationManager;
class ImageManager;
class GlyphManager;

namespace gfx {
class DynamicTextureAtlas;
using DynamicTextureAtlasPtr = std::shared_ptr<gfx::DynamicTextureAtlas>;
} // namespace gfx

class TileParameters {
public:
    const float pixelRatio;
    const MapDebugOptions debugOptions;
    const TransformState& transformState;
    std::shared_ptr<FileSource> fileSource;
    const MapMode mode;
    mapbox::base::WeakPtr<AnnotationManager> annotationManager;
    std::shared_ptr<ImageManager> imageManager;
    std::shared_ptr<GlyphManager> glyphManager;
    const uint8_t prefetchZoomDelta;
    TaggedScheduler threadPool;
    double tileLodMinRadius = 3;
    double tileLodScale = 1;
    double tileLodPitchThreshold = (60.0 / 180.0) * std::numbers::pi;
    double tileLodZoomShift = 0;
    TileLodMode tileLodMode = TileLodMode::Default;
    gfx::DynamicTextureAtlasPtr dynamicTextureAtlas;
    bool isUpdateSynchronous = false;
    // Cross-source lookup: a render source can resolve another source by ID
    // during update(). Used by ContourSource to find its upstream
    // raster-dem source and subscribe to tile-load events. Empty / returning
    // nullptr means "not available" (e.g. when constructing tile parameters
    // outside the orchestrator's update loop, as in tests).
    std::function<RenderSource*(const std::string&)> getRenderSource;
};

} // namespace mbgl
