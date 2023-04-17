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

void DrawableGL::setVertData(std::vector<std::uint8_t> vertexData_,
                             std::vector<std::uint16_t> indexes_) {
    impl->vertData = std::move(vertexData_);
    impl->indexes = std::move(indexes_);
}

std::vector<std::uint16_t>& DrawableGL::getIndexData() const {
    return impl->indexes;
}

const gl::VertexArray& DrawableGL::getVertexArray() const {
    return impl->vertexArray;
}

void DrawableGL::setVertexArray(gl::VertexArray&& vertexArray_,
                                gfx::UniqueVertexBufferResource&& attributeBuffer_,
                                gfx::IndexBuffer&& indexBuffer_) {
    impl->vertexArray = std::move(vertexArray_);
    impl->attributeBuffer = std::move(attributeBuffer_);
    impl->indexBuffer = std::move(indexBuffer_);
}

} // namespace gl
} // namespace mbgl
