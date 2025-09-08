#pragma once

#include <mbgl/map/mode.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/style/light.hpp>
#include <mbgl/style/image.hpp>
#include <mbgl/style/source.hpp>
#include <mbgl/style/layer.hpp>
#include <mbgl/text/glyph.hpp>
#include <mbgl/util/chrono.hpp>
#include <mbgl/util/immutable.hpp>

#include <numbers>
#include <vector>

#include <mapbox/std/weak.hpp>

namespace mbgl {

class AnnotationManager;
class FileSource;

class UpdateParameters {
public:
    const bool styleLoaded;
    const MapMode mode;
    const float pixelRatio;
    const MapDebugOptions debugOptions;
    const TimePoint timePoint;
    const TransformState transformState;

    const std::string glyphURL;
    std::shared_ptr<FontFaces> fontFaces;
    const bool spriteLoaded;
    const style::TransitionOptions transitionOptions;
    const Immutable<style::Light::Impl> light;
    const Immutable<std::vector<Immutable<style::Image::Impl>>> images;
    const Immutable<std::vector<Immutable<style::Source::Impl>>> sources;
    const Immutable<std::vector<Immutable<style::Layer::Impl>>> layers;

    mapbox::base::WeakPtr<AnnotationManager> annotationManager;
    std::shared_ptr<FileSource> fileSource;

    const uint8_t prefetchZoomDelta;

    // For still image requests, render requested
    const bool stillImageRequest;

    const bool crossSourceCollisions;

    double tileLodMinRadius = 3;
    double tileLodScale = 1;
    double tileLodPitchThreshold = (60.0 / 180.0) * std::numbers::pi;
    double tileLodZoomShift = 0;
};

} // namespace mbgl
