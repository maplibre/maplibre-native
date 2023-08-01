#pragma once

#include <mbgl/gfx/drawable_data.hpp>
#include <mbgl/style/types.hpp>

#include <memory>

namespace mbgl {
namespace gfx {

struct CollisionDrawableData : public DrawableData {
    CollisionDrawableData(const std::array<float, 2> translate_, const style::TranslateAnchorType translateAnchor_)
        : translate(translate_),
          translateAnchor(translateAnchor_) {}
    ~CollisionDrawableData() override = default;

    std::array<float, 2> translate;
    style::TranslateAnchorType translateAnchor;
};

using UniqueCollisionDrawableData = std::unique_ptr<CollisionDrawableData>;

} // namespace gfx
} // namespace mbgl
