#pragma once

#include <mln/gfx/drawable_data.hpp>
#include <mln/style/types.hpp>

#include <memory>

namespace mln {
namespace gfx {

struct CollisionDrawableData : public DrawableData {
    CollisionDrawableData(const std::array<float, 2> translate_,
                          const style::TranslateAnchorType translateAnchor_,
                          const float heightOffset_)
        : translate(translate_),
          translateAnchor(translateAnchor_),
          heightOffset(heightOffset_) {}
    ~CollisionDrawableData() override = default;

    std::array<float, 2> translate;
    style::TranslateAnchorType translateAnchor;
    /// `symbol-height-offset`, in meters
    float heightOffset;
};

using UniqueCollisionDrawableData = std::unique_ptr<CollisionDrawableData>;

} // namespace gfx
} // namespace mln
