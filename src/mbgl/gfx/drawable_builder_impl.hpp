#include <mbgl/gfx/attribute.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/index_vector.hpp>
#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/util/color.hpp>

#include <cstdint>

namespace mbgl {
namespace gfx {

struct DrawableBuilder::Impl {
    using VT = gfx::detail::VertexType<gfx::AttributeType<std::int16_t, 2>>;
    gfx::VertexVector<VT> vertices;
    gfx::IndexVector<gfx::Triangles> triangleIndexes;
    gfx::IndexVector<gfx::Lines> lineIndexes;
    Color currentColor = Color::white();
    std::vector<Color> colors;
    gfx::CullFaceMode cullFaceMode = gfx::CullFaceMode::disabled();
};

} // namespace gfx
} // namespace mbgl
