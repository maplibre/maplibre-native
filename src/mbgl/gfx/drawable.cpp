#include <mbgl/gfx/drawable.hpp>
#include <mbgl/util/mat4.hpp>

namespace mbgl {
namespace gfx {

Drawable::Drawable() :
    matrix(matrix::identity4())
{
}

} // namespace gfx
} // namespace mbgl

