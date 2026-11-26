#pragma once

#include <mln/style/image.hpp>
#include <mln/util/immutable.hpp>
#include <mln/style/sprite.hpp>

#include <exception>
#include <vector>

namespace mln {

namespace style {
class Image;
} // namespace style

class SpriteLoaderObserver {
public:
    virtual ~SpriteLoaderObserver() = default;

    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    virtual void onSpriteLoaded(std::optional<style::Sprite>, std::vector<Immutable<style::Image::Impl>>) {}

    virtual void onSpriteError(std::optional<style::Sprite>, std::exception_ptr) {}

    virtual void onSpriteRequested(const std::optional<style::Sprite>&) {}
};

} // namespace mln
