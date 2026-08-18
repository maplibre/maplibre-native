#pragma once

#include <mbgl/map/mode.hpp>
#include <mbgl/actor/scheduler.hpp>

#include <memory>
#include <numbers>

#include <mapbox/std/weak.hpp>

namespace mbgl {

class TransformState;
class FileSource;
class AnnotationManager;
class ImageManager;
class GlyphManager;

namespace gfx {
class DynamicTextureAtlas;
using DynamicTextureAtlasPtr = std::shared_ptr<gfx::DynamicTextureAtlas>;
} // namespace gfx

namespace util {
class TileElevationProvider;
} // namespace util

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
    /// Terrain elevation for the tile cover; null when there is no terrain, which
    /// leaves the cover flat. See util::TileElevationProvider.
    const util::TileElevationProvider* elevationProvider = nullptr;
};

} // namespace mbgl
