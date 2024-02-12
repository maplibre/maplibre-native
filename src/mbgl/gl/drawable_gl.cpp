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
#include <mbgl/util/string_indexer.hpp>

namespace mbgl {
namespace gl {

DrawableGL::DrawableGL(std::string name_)
    : Drawable(std::move(name_)),
      impl(std::make_unique<Impl>()) {}

DrawableGL::~DrawableGL() {
    impl->indexBuffer = {0, nullptr};
    impl->attributeBuffers.clear();
}

void DrawableGL::draw(PaintParameters& parameters) const {
    if (isCustom) {
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
        mbgl::Log::Warning(Event::General, "Missing shader for drawable " + util::toString(getID()) + "/" + getName());
        assert(false);
        return;
    }

    if (enableDepth) {
        context.setDepthMode(getIs3D() ? parameters.depthModeFor3D()
                                       : parameters.depthModeForSublayer(getSubLayerIndex(), getDepthType()));
    } else {
        context.setDepthMode(gfx::DepthMode::disabled());
    }

    // force disable depth test for debugging
    // context.setDepthMode({gfx::DepthFunctionType::Always, gfx::DepthMaskType::ReadOnly, {0,1}});

    // For 3D mode, stenciling is handled by the layer group
    if (!is3D) {
        context.setStencilMode(makeStencilMode(parameters));
    }

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

#ifndef NDEBUG
    context.bindVertexArray = value::BindVertexArray::Default;

    unbindTextures();
    unbindUniformBuffers();
#endif
}

void DrawableGL::setIndexData(gfx::IndexVectorBasePtr indexes, std::vector<UniqueDrawSegment> segments) {
    impl->indexes = std::move(indexes);
    impl->segments = std::move(segments);
}

void DrawableGL::setVertices(std::vector<uint8_t>&& data, std::size_t count, gfx::AttributeDataType type_) {
    impl->vertexData = std::move(data);
    impl->vertexCount = count;
    impl->vertexType = type_;
}

const gfx::UniformBufferArray& DrawableGL::getUniformBuffers() const {
    return impl->uniformBuffers;
}

gfx::UniformBufferArray& DrawableGL::mutableUniformBuffers() {
    return impl->uniformBuffers;
}

void DrawableGL::setVertexAttrNameId(const StringIdentity id) {
    impl->idVertexAttrName = id;
}

void DrawableGL::bindUniformBuffers() const {
    if (shader) {
        const auto& uniformBlocks = shader->getUniformBlocks();
        for (size_t id = 0; id < uniformBlocks.allocatedSize(); id++) {
            const auto& block = uniformBlocks.get(id);
            if (!block) continue;
            const auto& uniformBuffer = getUniformBuffers().get(id);
            assert(uniformBuffer && "UBO missing, drawable skipped");
            if (!uniformBuffer) {
                using namespace std::string_literals;
                const auto tileIDStr = getTileID() ? util::toString(*getTileID()) : "<no tile>";
                Log::Error(Event::General,
                           "bindUniformBuffers: UBO "s + util::toString(block->getIndex()) + " not found for " +
                               util::toString(getID()) + " / " + getName() + " / " + tileIDStr + ". skipping.");
                assert(false);
                continue;
            }
            block->bindBuffer(*uniformBuffer);
        }
    }
}

void DrawableGL::unbindUniformBuffers() const {
    if (shader) {
        const auto& uniformBlocks = shader->getUniformBlocks();
        for (size_t id = 0; id < uniformBlocks.allocatedSize(); id++) {
            const auto& block = uniformBlocks.get(id);
            if (block) {
                block->unbindBuffer();
            }
        }
    }
}

struct IndexBufferGL : public gfx::IndexBufferBase {
    IndexBufferGL(std::unique_ptr<gfx::IndexBuffer>&& buffer_)
        : buffer(std::move(buffer_)) {}
    ~IndexBufferGL() override = default;

    std::unique_ptr<mbgl::gfx::IndexBuffer> buffer;
};

void DrawableGL::upload(gfx::UploadPass& uploadPass) {
    if (!shader) {
        return;
    }

    const bool build = vertexAttributes &&
                       (vertexAttributes->isDirty() ||
                        std::any_of(impl->segments.begin(), impl->segments.end(), [](const auto& seg) {
                            return !static_cast<const DrawSegmentGL&>(*seg).getVertexArray().isValid();
                        }));

    if (build) {
        auto& context = uploadPass.getContext();
        auto& glContext = static_cast<gl::Context&>(context);
        constexpr auto usage = gfx::BufferUsageType::StaticDraw;

        // Apply drawable values to shader defaults
        const auto& defaults = shader->getVertexAttributes();
        const auto& overrides = *vertexAttributes;

        const auto& indexAttribute = defaults.get(impl->idVertexAttrName);
        const auto vertexAttributeIndex = static_cast<std::size_t>(indexAttribute ? indexAttribute->getIndex() : -1);

        std::vector<std::unique_ptr<gfx::VertexBufferResource>> vertexBuffers;
        auto bindings = uploadPass.buildAttributeBindings(impl->vertexCount,
                                                          impl->vertexType,
                                                          vertexAttributeIndex,
                                                          impl->vertexData,
                                                          defaults,
                                                          overrides,
                                                          usage,
                                                          vertexBuffers);

        impl->attributeBuffers = std::move(vertexBuffers);

        if (impl->indexes->getDirty()) {
            auto indexBufferResource{
                uploadPass.createIndexBufferResource(impl->indexes->data(), impl->indexes->bytes(), usage)};
            auto indexBuffer = std::make_unique<gfx::IndexBuffer>(impl->indexes->elements(),
                                                                  std::move(indexBufferResource));
            auto buffer = std::make_unique<IndexBufferGL>(std::move(indexBuffer));

            impl->indexes->setBuffer(std::move(buffer));
            impl->indexes->setDirty(false);
        }

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

            if (!glSeg.getVertexArray().isValid()) {
                auto vertexArray = glContext.createVertexArray();
                const auto& indexBuffer = static_cast<IndexBufferGL&>(*impl->indexes->getBuffer());
                vertexArray.bind(glContext, *indexBuffer.buffer, bindings);
                assert(vertexArray.isValid());
                if (vertexArray.isValid()) {
                    glSeg.setVertexArray(std::move(vertexArray));
                }
            }
        };
    }

    const bool texturesNeedUpload = std::any_of(
        textures.begin(), textures.end(), [](const auto& texture) { return texture && texture->needsUpload(); });

    if (texturesNeedUpload) {
        uploadTextures();
    }
}

gfx::ColorMode DrawableGL::makeColorMode(PaintParameters& parameters) const {
    return enableColor ? parameters.colorModeForRenderPass() : gfx::ColorMode::disabled();
}

gfx::StencilMode DrawableGL::makeStencilMode(PaintParameters& parameters) const {
    if (enableStencil) {
        if (!is3D && tileID) {
            return parameters.stencilModeForClipping(tileID->toUnwrapped());
        }
        assert(false);
    }
    return gfx::StencilMode::disabled();
}

void DrawableGL::uploadTextures() const {
    for (const auto& texture : textures) {
        if (texture) {
            static_cast<gl::Texture2D&>(*texture).upload();
        }
    }
}

void DrawableGL::bindTextures() const {
    int32_t unit = 0;
    for (size_t id = 0; id < textures.size(); id++) {
        if (const auto& texture = textures[id]) {
            if (const auto& location = shader->getSamplerLocation(id)) {
                static_cast<gl::Texture2D&>(*texture).bind(static_cast<int32_t>(*location), unit++);
            }
        }
    }
}

void DrawableGL::unbindTextures() const {
    for (const auto& texture : textures) {
        if (texture) {
            static_cast<gl::Texture2D&>(*texture).unbind();
        }
    }
}

} // namespace gl
} // namespace mbgl
