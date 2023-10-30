#pragma once

#include <mbgl/map/mode.hpp>

#include <memory>

#include <mapbox/std/weak.hpp>

namespace mbgl {

class TransformState;
class ResourceLoader;
class AnnotationManager;
class ImageManager;
class GlyphManager;

class TileParameters {
public:
    const float pixelRatio;
    const MapDebugOptions debugOptions;
    const TransformState& transformState;
    std::shared_ptr<ResourceLoader> fileSource;
    const MapMode mode;
    mapbox::base::WeakPtr<AnnotationManager> annotationManager;
    ImageManager& imageManager;
    GlyphManager& glyphManager;
    const uint8_t prefetchZoomDelta;
};

} // namespace mbgl
