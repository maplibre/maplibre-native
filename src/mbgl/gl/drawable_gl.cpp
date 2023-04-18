#include <mbgl/gl/drawable_gl.hpp>
#include <mbgl/gl/drawable_gl_impl.hpp>

namespace mbgl {
namespace gl {

DrawableGL::DrawableGL()
    : impl(std::make_unique<Impl>()) {
}

DrawableGL::~DrawableGL() {
}

void DrawableGL::draw(const PaintParameters &parameters) const
{
    impl->draw(parameters);
}

void DrawableGL::setVertexData(std::vector<std::uint8_t> vertexData,
                               std::vector<std::uint16_t> indexes,
                               const std::size_t indexOffset,
                               const std::size_t indexLength) {
    impl->vertData = std::move(vertexData);
    impl->indexes = std::move(indexes);
    impl->indexOffset = indexOffset;
    impl->indexLength = indexLength;
}

std::vector<std::uint16_t>& DrawableGL::getIndexData() const {
    return impl->indexes;
}

std::vector<std::uint8_t>& DrawableGL::getVertexData() const {
    return impl->vertData;
}

const gl::VertexArray& DrawableGL::getVertexArray() const {
    return impl->vertexArray;
}

void DrawableGL::setVertexArray(gl::VertexArray&& vertexArray_,
                                gfx::UniqueVertexBufferResource&& attributeBuffer_,
                                gfx::IndexBuffer&& indexBuffer_,
                                std::size_t attrOffset) {
    impl->vertexArray = std::move(vertexArray_);
    impl->attributeBuffer = std::move(attributeBuffer_);
    impl->attributeOffset = attrOffset;
    impl->indexBuffer = std::move(indexBuffer_);
}

} // namespace gl
} // namespace mbgl
