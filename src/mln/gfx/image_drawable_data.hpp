#pragma once

#include <mln/gfx/drawable_data.hpp>
#include <mln/util/mat4.hpp>

#include <memory>

namespace mln {

namespace gfx {

class ImageDrawableData : public DrawableData {
public:
    ImageDrawableData(mat4 matrix_)
        : matrix(matrix_) {}

    mat4 matrix;
};

using UniqueImageDrawableData = std::unique_ptr<ImageDrawableData>;

} // namespace gfx
} // namespace mln
