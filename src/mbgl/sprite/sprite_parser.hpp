#pragma once

#include <mbgl/style/image.hpp>

#include <memory>

namespace mbgl {

// Extracts an individual image from a spritesheet from the given location.
std::unique_ptr<style::Image> createStyleImage(const std::string& id,
                                               const PremultipliedImage&,
                                               int32_t srcX,
                                               int32_t srcY,
                                               int32_t srcWidth,
                                               int32_t srcHeight,
                                               double ratio,
                                               bool sdf,
                                               style::ImageStretches&& stretchX = {},
                                               style::ImageStretches&& stretchY = {},
                                               const std::optional<style::ImageContent>& content = std::nullopt,
                                               const std::optional<style::TextFit>& textFitWidth = std::nullopt,
                                               const std::optional<style::TextFit>& textFitHeight = std::nullopt);

// Parses an image and an associated JSON file and returns the sprite objects.
std::vector<Immutable<style::Image::Impl>> parseSprite(const std::string& id,
                                                       const std::string& image,
                                                       const std::string& json);

} // namespace mbgl
