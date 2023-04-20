#include <mbgl/gfx/attribute.hpp>
#include <mbgl/gfx/index_vector.hpp>
#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/util/color.hpp>

namespace mbgl {
namespace gfx {

struct DrawableBuilder::Impl {
    using VT = gfx::detail::VertexType<gfx::AttributeType<float,2>>;
    gfx::VertexVector<VT> vertices;
    gfx::IndexVector<gfx::Triangles> indexes;
    SegmentVector<TypeList<void>> segments;
    Color currentColor = Color::white();
    std::vector<Color> colors;
};

} // namespace gfx
} // namespace mbgl

