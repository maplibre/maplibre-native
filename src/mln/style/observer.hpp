#pragma once

#include <mln/style/source_observer.hpp>
#include <mln/style/sprite.hpp>

#include <exception>
#include <optional>

namespace mln {
namespace style {

class Observer : public SourceObserver {
public:
    virtual void onStyleLoading() {}
    virtual void onStyleLoaded() {}
    virtual void onUpdate() {}
    virtual void onStyleError(std::exception_ptr) {}
    virtual void onResourceError(std::exception_ptr) {}

    virtual void onSpriteLoaded(const std::optional<Sprite>&) {}
    virtual void onSpriteError(const std::optional<Sprite>&, std::exception_ptr) {}
    virtual void onSpriteRequested(const std::optional<Sprite>&) {}
};

} // namespace style
} // namespace mln
