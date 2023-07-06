#include <mbgl/gl/drawable_gl.hpp>
#include <mbgl/gl/drawable_gl_impl.hpp>
#include <mbgl/gl/texture2d.hpp>
#include <mbgl/gl/upload_pass.hpp>
#include <mbgl/gl/vertex_array.hpp>
#include <mbgl/gl/vertex_attribute_gl.hpp>
#include <mbgl/gl/vertex_buffer_resource.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/shaders/gl/shader_program_gl.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace gl {

DrawableGL::DrawableGL(std::string name_)
    : Drawable(std::move(name_)),
      impl(std::make_unique<Impl>()) {}

DrawableGL::~DrawableGL() {
    impl->indexBuffer = {0, nullptr};
    impl->attributeBuffer.reset();
}

void DrawableGL::draw(PaintParameters& parameters) const {
    if(isCustom) {
        return;
    }

    auto& context = static_cast<gl::Context&>(parameters.context);

    if (shader) {
        const auto& shaderGL = static_cast<const ShaderProgramGL&>(*shader);
        if (shaderGL.getGLProgramID() != context.program.getCurrentValue()) {
            context.program = shaderGL.getGLProgramID();
        }
    }
    if (!shader || context.program.getCurrentValue() == 0) {
        mbgl::Log::Warning(Event::General, "Missing shader for drawable " + util::toString(getId()) + "/" + getName());
        assert(false);
        return;
    }

    context.setDepthMode(getIs3D() ? parameters.depthModeFor3D()
                                   : parameters.depthModeForSublayer(getSubLayerIndex(), getDepthType()));

    // force disable depth test for debugging
    // context.setDepthMode({gfx::DepthFunctionType::Always, gfx::DepthMaskType::ReadOnly, {0,1}});

    context.setStencilMode(makeStencilMode(parameters));

    context.setColorMode(getColorMode());
    context.setCullFaceMode(getCullFaceMode());

    bindUniformBuffers();
    bindTextures();

    for (const auto& seg : impl->segments) {
        const auto& glSeg = static_cast<DrawSegmentGL&>(*seg);
        const auto& mlSeg = glSeg.getSegment();
        if (mlSeg.indexLength > 0 && glSeg.getVertexArray().isValid()) {
            context.bindVertexArray = glSeg.getVertexArray().getID();
            context.draw(glSeg.getMode(), mlSeg.indexOffset, mlSeg.indexLength);
        }
    }

    context.bindVertexArray = value::BindVertexArray::Default;

    unbindTextures();
    unbindUniformBuffers();
}

void DrawableGL::setIndexData(std::vector<std::uint16_t> indexes, std::vector<UniqueDrawSegment> segments) {
    impl->indexes = std::move(indexes);
    impl->segments = std::move(segments);
}

void DrawableGL::setVertices(std::vector<uint8_t>&& data, std::size_t count, gfx::AttributeDataType type) {
    impl->vertexData = std::move(data);
    impl->vertexCount = count;
    impl->vertexType = type;
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

const gfx::UniformBufferArray& DrawableGL::getUniformBuffers() const {
    return impl->uniformBuffers;
}

gfx::UniformBufferArray& DrawableGL::mutableUniformBuffers() {
    return impl->uniformBuffers;
}

void DrawableGL::setVertexAttrName(std::string value) {
    impl->vertexAttrName = std::move(value);
}

void DrawableGL::bindUniformBuffers() const {
    if (shader) {
        const auto& shaderGL = static_cast<const ShaderProgramGL&>(*shader);
        for (const auto& element : shaderGL.getUniformBlocks().getMap()) {
            const auto& uniformBuffer = getUniformBuffers().get(element.first);
            if (!uniformBuffer) {
                using namespace std::string_literals;
                Log::Error(Event::General,
                           "DrawableGL::bindUniformBuffers: UBO "s + element.first + " not found. skipping.");
                assert(false);
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

void DrawableGL::upload(gfx::UploadPass& uploadPass) {
    if (!shader) {
        return;
    }

    const bool build = impl->vertexAttributes.isDirty() ||
                       std::any_of(impl->segments.begin(), impl->segments.end(), [](const auto& seg) {
                           return !static_cast<const DrawSegmentGL&>(*seg).getVertexArray().isValid();
                       });

    if (build) {
        auto& context = uploadPass.getContext();
        auto& glContext = static_cast<gl::Context&>(context);
        constexpr auto usage = gfx::BufferUsageType::StaticDraw;

        const auto indexBytes = impl->indexes.size() * sizeof(decltype(impl->indexes)::value_type);
        auto indexBufferResource = uploadPass.createIndexBufferResource(impl->indexes.data(), indexBytes, usage);
        auto indexBuffer = gfx::IndexBuffer{impl->indexes.size(), std::move(indexBufferResource)};

        // Apply drawable values to shader defaults
        const auto& defaults = shader->getVertexAttributes();
        const auto& overrides = impl->vertexAttributes;

        const auto& indexAttribute = defaults.get(impl->vertexAttrName);
        const auto vertexAttributeIndex = static_cast<std::size_t>(indexAttribute ? indexAttribute->getIndex() : -1);

        std::unique_ptr<gfx::VertexBufferResource> vertexBuffer;
        auto bindings = uploadPass.buildAttributeBindings(impl->vertexCount,
                                                          impl->vertexType,
                                                          vertexAttributeIndex,
                                                          impl->vertexData,
                                                          defaults,
                                                          overrides,
                                                          usage,
                                                          vertexBuffer);

        impl->attributeBuffer = std::move(vertexBuffer);
        impl->indexBuffer = std::move(indexBuffer);

        // Create a VAO for each group of vertexes described by a segment
        for (const auto& seg : impl->segments) {
            auto& glSeg = static_cast<DrawSegmentGL&>(*seg);
            const auto& mlSeg = glSeg.getSegment();

            if (mlSeg.indexLength == 0) {
                continue;
            }

            for (auto& binding : bindings) {
                if (binding) {
                    binding->vertexOffset = static_cast<uint32_t>(mlSeg.vertexOffset);
                }
            }

            auto vertexArray = glContext.createVertexArray();

            vertexArray.bind(glContext, impl->indexBuffer, bindings);

            assert(vertexArray.isValid());

            glSeg.setVertexArray(std::move(vertexArray));
        };
    }

    const bool texturesNeedUpload = std::any_of(
        textures.begin(), textures.end(), [](const auto& pair) { return pair.second && pair.second->needsUpload(); });

    if (texturesNeedUpload) {
        uploadTextures();
    }
}

gfx::ColorMode DrawableGL::makeColorMode(PaintParameters& parameters) const {
    return enableColor ? parameters.colorModeForRenderPass() : gfx::ColorMode::disabled();
}

gfx::StencilMode DrawableGL::makeStencilMode(PaintParameters& parameters) const {
    if (enableStencil) {
        if (is3D) {
            return parameters.stencilModeFor3D();
        } else if (tileID) {
            return parameters.stencilModeForClipping(tileID->toUnwrapped());
        }
        assert(false);
    }
    return gfx::StencilMode::disabled();
}

void DrawableGL::uploadTextures() const {
    for (const auto& pair : textures) {
        if (const auto& tex = pair.second) {
            std::static_pointer_cast<gl::Texture2D>(tex)->upload();
        }
    }
}

void DrawableGL::bindTextures() const {
    int32_t unit = 0;
    for (const auto& pair : textures) {
        if (const auto& tex = pair.second) {
            const auto& location = pair.first;
            std::static_pointer_cast<gl::Texture2D>(tex)->bind(location, unit++);
        }
    }
}

void DrawableGL::unbindTextures() const {
    for (const auto& pair : textures) {
        if (const auto& tex = pair.second) {
            std::static_pointer_cast<gl::Texture2D>(tex)->unbind();
        }
    }
}

} // namespace gl
} // namespace mbgl
