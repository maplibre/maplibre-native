#include <mbgl/gl/drawable_gl.hpp>
#include <mbgl/gl/drawable_gl_impl.hpp>
#include <mbgl/gl/vertex_buffer_resource.hpp>

#include <mbgl/shaders/gl/shader_program_gl.hpp>

namespace mbgl {
namespace gl {

DrawableGL::DrawableGL(std::string name_)
    : Drawable(std::move(name_)),
      impl(std::make_unique<Impl>()) {}

DrawableGL::~DrawableGL() {
    impl->vertexArray = {{nullptr, false}};
    impl->indexBuffer = {0, nullptr};
    impl->attributeBuffer.reset();
}

void DrawableGL::draw(const PaintParameters& parameters) const {
    bindUniformBuffers();
    impl->draw(parameters);
    unbindUniformBuffers();
}

void DrawableGL::setIndexData(std::vector<std::uint16_t> indexes,
                              const std::size_t indexOffset,
                              const std::size_t indexLength) {
    impl->indexes = std::move(indexes);
    impl->indexOffset = indexOffset;
    impl->indexLength = indexLength;
}

std::vector<std::uint16_t>& DrawableGL::getIndexData() const {
    return impl->indexes;
}

const gl::VertexArray& DrawableGL::getVertexArray() const {
    return impl->vertexArray;
}

const gfx::IndexBuffer& DrawableGL::getIndexBuffer() const {
    return impl->indexBuffer;
}

const gfx::UniqueVertexBufferResource& DrawableGL::getBuffer() const {
    return impl->attributeBuffer;
}

const gfx::VertexAttributeArray& DrawableGL::getVertexAttributes() const {
    return impl->vertexAttributes;
}

gfx::VertexAttributeArray& DrawableGL::mutableVertexAttributes() {
    return impl->vertexAttributes;
}

void DrawableGL::setVertexAttributes(const gfx::VertexAttributeArray& value) {
    impl->vertexAttributes = static_cast<const VertexAttributeArrayGL&>(value);
}
void DrawableGL::setVertexAttributes(gfx::VertexAttributeArray&& value) {
    impl->vertexAttributes = std::move(static_cast<VertexAttributeArrayGL&&>(value));
}

void DrawableGL::setVertexArray(gl::VertexArray&& vertexArray_,
                                gfx::UniqueVertexBufferResource&& attributeBuffer_,
                                gfx::IndexBuffer&& indexBuffer_) {
    impl->vertexArray = std::move(vertexArray_);
    impl->attributeBuffer = std::move(attributeBuffer_);
    impl->attributeOffset = 0;
    impl->indexBuffer = std::move(indexBuffer_);
}

const gfx::UniformBufferArray& DrawableGL::getUniformBuffers() const {
    return impl->uniformBuffers;
}

gfx::UniformBufferArray& DrawableGL::mutableUniformBuffers() {
    return impl->uniformBuffers;
}

void DrawableGL::resetColor(const Color& newColor) {
    if (const auto& colorAttr = impl->vertexAttributes.get("a_color")) {
        colorAttr->clear();
        colorAttr->set(0, colorAttrValue(newColor));
    }
}

void DrawableGL::bindUniformBuffers() const {
    if (shader) {
        const auto& shaderGL = static_cast<const ShaderProgramGL&>(*shader);
        for (const auto& element : shaderGL.getUniformBlocks().getMap()) {
            const auto& uniformBuffer = getUniformBuffers().get(element.first);
            if (!uniformBuffer) {
                continue;
            }
            element.second->bindBuffer(*uniformBuffer);
        }
    }
}

void DrawableGL::unbindUniformBuffers() const {
    if (shader) {
        const auto& shaderGL = static_cast<const ShaderProgramGL&>(*shader);
        for (const auto& element : shaderGL.getUniformBlocks().getMap()) {
            element.second->unbindBuffer();
        }
    }
}

} // namespace gl
} // namespace mbgl
