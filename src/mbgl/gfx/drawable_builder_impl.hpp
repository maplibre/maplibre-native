#include <mbgl/gfx/attribute.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/index_vector.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/programs/segment.hpp>

#include <cstdint>
#include <memory>

namespace mbgl {
namespace gfx {

struct DrawableBuilder::Impl {
    using VT = gfx::detail::VertexType<gfx::AttributeType<std::int16_t, 2>>;
    gfx::VertexVector<VT> vertices;

    std::vector<uint8_t> rawVertices;
    std::size_t rawVerticesCount = 0;

    std::vector<uint16_t> buildIndexes;
    std::shared_ptr<gfx::IndexVectorBase> sharedIndexes;
    std::vector<std::unique_ptr<Drawable::DrawSegment>> segments;

    AttributeDataType rawVerticesType = static_cast<AttributeDataType>(-1);
    gfx::ColorMode colorMode = gfx::ColorMode::disabled();
    gfx::CullFaceMode cullFaceMode = gfx::CullFaceMode::disabled();

    VertexAttributeArray vertexAttrs;
};

} // namespace gfx
} // namespace mbgl
