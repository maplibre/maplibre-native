#pragma once

#include <mbgl/style/image.hpp>
#include <mbgl/util/containers.hpp>

#include <string>
#include <optional>

namespace mbgl {
namespace style {

class Image::Impl {
public:
    Impl(std::string id,
         PremultipliedImage&&,
         float pixelRatio,
         bool sdf = false,
         ImageStretches stretchX = {},
         ImageStretches stretchY = {},
         std::optional<ImageContent> content = std::nullopt,
         std::optional<TextFit> textFitWidth = std::nullopt,
         std::optional<TextFit> textFitHeight = std::nullopt);

    const std::string id;

    PremultipliedImage image;

    // Pixel ratio of the sprite image.
    const float pixelRatio;

    // Whether this image should be interpreted as a signed distance field icon.
    const bool sdf;

    // Stretch areas of this image.
    const ImageStretches stretchX;
    const ImageStretches stretchY;

    // The space where text can be fit into this image.
    const std::optional<ImageContent> content;

    // If `icon-text-fit` is used in a layer with this image, this option defines constraints on the horizontal scaling
    // of the image.
    const std::optional<TextFit> textFitWidth;
    // If `icon-text-fit` is used in a layer with this image, this option defines constraints on the vertical scaling of
    // the image.
    const std::optional<TextFit> textFitHeight;
};

} // namespace style

enum class ImageType : bool {
    Icon,
    Pattern
};

using ImageMap = mbgl::unordered_map<std::string, Immutable<style::Image::Impl>>;
using ImageDependencies = mbgl::unordered_map<std::string, ImageType>;
using ImageRequestPair = std::pair<ImageDependencies, uint64_t>;
using ImageVersionMap = mbgl::unordered_map<std::string, uint32_t>;
inline bool operator<(const Immutable<mbgl::style::Image::Impl>& a, const Immutable<mbgl::style::Image::Impl>& b) {
    return a->id < b->id;
}

} // namespace mbgl
