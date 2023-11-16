#pragma once

#include <mbgl/gfx/drawable_data.hpp>
#include <mbgl/style/types.hpp>

#include <memory>

namespace mbgl {
namespace gfx {

enum class FillVariant : uint8_t {
    Fill,
    FillPattern,
    FillOutline,
    FillOutlinePattern,
    Undefined = 255
};

struct FillDrawableData : public DrawableData {
    FillDrawableData(FillVariant type_) : type(type_) {}
    ~FillDrawableData() override = default;
    
    FillVariant type;
};

using UniqueFillDrawableData = std::unique_ptr<FillDrawableData>;

} // namespace gfx
} // namespace mbgl
