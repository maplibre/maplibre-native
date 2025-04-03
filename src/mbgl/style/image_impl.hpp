#pragma once

#include <mbgl/style/image.hpp>
#include <mbgl/util/containers.hpp>
#include <mbgl/util/rect.hpp>

#include <string>
#include <optional>
#include <mapbox/shelf-pack.hpp>

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

class ImagePosition {
public:
    ImagePosition(const mapbox::Bin& bin, const style::Image::Impl& image, uint32_t version_ = 0)
        : pixelRatio(image.pixelRatio),
          paddedRect(bin.x, bin.y, bin.w, bin.h),
          version(version_),
          stretchX(image.stretchX),
          stretchY(image.stretchY),
          content(image.content),
          textFitWidth(image.textFitWidth),
          textFitHeight(image.textFitHeight) {}

    static constexpr const uint16_t padding = 1u;
    float pixelRatio;
    Rect<uint16_t> paddedRect;
    uint32_t version;
    style::ImageStretches stretchX;
    style::ImageStretches stretchY;
    std::optional<style::ImageContent> content;
    std::optional<style::TextFit> textFitWidth;
    std::optional<style::TextFit> textFitHeight;

    std::array<uint16_t, 2> tl() const {
        return {{static_cast<uint16_t>(paddedRect.x + padding), static_cast<uint16_t>(paddedRect.y + padding)}};
    }

    std::array<uint16_t, 2> br() const {
        return {{static_cast<uint16_t>(paddedRect.x + paddedRect.w - padding),
                 static_cast<uint16_t>(paddedRect.y + paddedRect.h - padding)}};
    }

    std::array<uint16_t, 4> tlbr() const {
        const auto _tl = tl();
        const auto _br = br();
        return {{_tl[0], _tl[1], _br[0], _br[1]}};
    }

    std::array<float, 2> displaySize() const {
        return {{
            static_cast<float>(paddedRect.w - padding * 2) / pixelRatio,
            static_cast<float>(paddedRect.h - padding * 2) / pixelRatio,
        }};
    }
};

using ImagePositions = mbgl::unordered_map<std::string, ImagePosition>;

} // namespace mbgl
