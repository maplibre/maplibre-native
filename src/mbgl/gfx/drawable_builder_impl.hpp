#include <mbgl/gfx/attribute.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/index_vector.hpp>
#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/util/color.hpp>

#include <cstdint>
#include <memory>

namespace mbgl {
namespace gfx {

struct DrawableBuilder::Impl {
    using VT = gfx::detail::VertexType<gfx::AttributeType<std::int16_t, 2>>;
    gfx::VertexVector<VT> vertices;
    std::vector<uint8_t> rawVertices;
    std::size_t rawVerticesCount = 0;
    AttributeDataType rawVerticesType = static_cast<AttributeDataType>(-1);
    std::vector<uint16_t> indexes;
    std::vector<std::unique_ptr<Drawable::DrawSegment>> segments;

    Color currentColor = Color::white();
    std::vector<Color> colors;
    gfx::ColorMode colorMode = gfx::ColorMode::disabled();
    gfx::CullFaceMode cullFaceMode = gfx::CullFaceMode::disabled();
};

} // namespace gfx
} // namespace mbgl
