#pragma once

#include <mbgl/gfx/drawable_data.hpp>
#include <mbgl/util/mat4.hpp>

#include <memory>

namespace mbgl {

namespace gfx {

class ImageDrawableData : public DrawableData {
public:
    ImageDrawableData(mat4 matrix_)
    : matrix(matrix_) {}

    mat4 matrix;
};

using UniqueImageDrawableData = std::unique_ptr<ImageDrawableData>;

} // namespace gfx
} // namespace mbgl
