#include <mbgl/gfx/drawable.hpp>
#include <mbgl/gfx/types.hpp>
#include <mbgl/util/mat4.hpp>

namespace mbgl {
namespace gfx {

Drawable::Drawable() :
    matrix(matrix::identity4()),
    depthType(DepthMaskType::ReadOnly)
{
}

} // namespace gfx
} // namespace mbgl

