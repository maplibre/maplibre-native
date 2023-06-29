#pragma once

#include <mbgl/gfx/drawable_data.hpp>
#include <mbgl/style/types.hpp>

#include <memory>

namespace mbgl {
namespace style {} // namespace style
namespace gfx {

struct FillExtrusionDrawableData : public DrawableData {
    FillExtrusionDrawableData() {}
    ~FillExtrusionDrawableData() = default;
};

using UniqueFillExtrusionDrawableData = std::unique_ptr<FillExtrusionDrawableData>;

} // namespace gfx
} // namespace mbgl
