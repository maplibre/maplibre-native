#pragma once

#include <mbgl/util/image.hpp>
#include <mbgl/util/immutable.hpp>

#include <string>
#include <utility>
#include <vector>
#include <optional>

namespace mbgl {
namespace style {

using ImageStretch = std::pair<float, float>;
using ImageStretches = std::vector<ImageStretch>;

enum class TextFit : uint8_t {
    stretchOrShrink,
    stretchOnly,
    proportional
};

class ImageContent {
public:
    float left;
    float top;
    float right;
    float bottom;

    bool operator==(const ImageContent& rhs) const {
        return left == rhs.left && top == rhs.top && right == rhs.right && bottom == rhs.bottom;
    }
};

class Image {
public:
    Image(std::string id,
          PremultipliedImage&&,
          float pixelRatio,
          bool sdf,
          ImageStretches stretchX = {},
          ImageStretches stretchY = {},
          const std::optional<ImageContent>& content = std::nullopt,
          const std::optional<TextFit>& textFitWidth = std::nullopt,
          const std::optional<TextFit>& textFitHeight = std::nullopt);
    Image(std::string id,
          PremultipliedImage&& image,
          float pixelRatio,
          ImageStretches stretchX = {},
          ImageStretches stretchY = {},
          const std::optional<ImageContent>& content = std::nullopt,
          const std::optional<TextFit>& textFitWidth = std::nullopt,
          const std::optional<TextFit>& textFitHeight = std::nullopt)
        : Image(std::move(id),
                std::move(image),
                pixelRatio,
                false,
                std::move(stretchX),
                std::move(stretchY),
                content,
                textFitWidth,
                textFitHeight) {}
    Image(const Image&);

    std::string getID() const;

    const PremultipliedImage& getImage() const;

    /// Pixel ratio of the sprite image.
    float getPixelRatio() const;

    /// Whether this image should be interpreted as a signed distance field icon.
    bool isSdf() const;

    /// Stretch area of this image
    const ImageStretches& getStretchX() const;
    /// Stretch area of this image
    const ImageStretches& getStretchY() const;

    /// The space where text can be fit into this image.
    const std::optional<ImageContent>& getContent() const;

    /// The constraints on the horizontal scaling of the image when `icon-text-fit` is used
    const std::optional<TextFit>& getTextFitWidth() const;

    /// The constraints on the vertical scaling of the image when `icon-text-fit` is used
    const std::optional<TextFit>& getTextFitHeight() const;

    class Impl;
    Immutable<Impl> baseImpl;
    explicit Image(Immutable<Impl> baseImpl_)
        : baseImpl(std::move(baseImpl_)) {}
};

} // namespace style
} // namespace mbgl
