#include <mbgl/gl/drawable_gl.hpp>
#include <mbgl/gl/drawable_gl_impl.hpp>
#include <mbgl/gl/renderer_backend.hpp>
#include <mbgl/gl/texture2d.hpp>
#include <mbgl/gl/upload_pass.hpp>
#include <mbgl/gl/vertex_array.hpp>
#include <mbgl/gl/vertex_attribute_gl.hpp>
#include <mbgl/gl/vertex_buffer_resource.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/shaders/gl/shader_program_gl.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/logging.hpp>

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
    MLN_TRACE_FUNC();

    if (isCustom) {
        return;
    }

    auto& context = static_cast<gl::Context&>(parameters.context);

    impl->createVAOs(context);

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

void DrawableGL::updateVertexAttributes(gfx::VertexAttributeArrayPtr vertices,
                                        std::size_t vertexCount,
                                        gfx::DrawMode mode,
                                        gfx::IndexVectorBasePtr indexes,
                                        const SegmentBase* segments,
                                        std::size_t segmentCount) {
    gfx::Drawable::setVertexAttributes(std::move(vertices));
    impl->vertexCount = vertexCount;

    std::vector<std::unique_ptr<Drawable::DrawSegment>> drawSegs;
    drawSegs.reserve(segmentCount);
    for (std::size_t i = 0; i < segmentCount; ++i) {
        const auto& seg = segments[i];
        auto segCopy = SegmentBase{
            // no copy constructor
            seg.vertexOffset,
            seg.indexOffset,
            seg.vertexLength,
            seg.indexLength,
            seg.sortKey,
        };
        auto drawSeg = std::make_unique<DrawableGL::DrawSegmentGL>(
            mode, std::move(segCopy), VertexArray{{nullptr, false}});
        drawSegs.push_back(std::move(drawSeg));
    }

    impl->indexes = std::move(indexes);
    impl->segments = std::move(drawSegs);
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

void DrawableGL::setVertexAttrId(const size_t id) {
    impl->vertexAttrId = id;
}

void DrawableGL::bindUniformBuffers() const {
    if (shader) {
        const auto& uniformBlocks = shader->getUniformBlocks();
        for (size_t id = 0; id < uniformBlocks.allocatedSize(); id++) {
            const auto& block = uniformBlocks.get(id);
            if (!block) continue;
            const auto& uniformBuffer = getUniformBuffers().get(id);
            if (uniformBuffer) {
                block->bindBuffer(*uniformBuffer);
            }
        }
    }
}

void DrawableGL::unbindUniformBuffers() const {
    if (shader) {
        const auto& uniformBlocks = shader->getUniformBlocks();
        for (size_t id = 0; id < uniformBlocks.allocatedSize(); id++) {
            const auto& block = uniformBlocks.get(id);
            if (!block) continue;
            const auto& uniformBuffer = getUniformBuffers().get(id);
            if (uniformBuffer) {
                block->unbindBuffer();
            }
        }
    }
}

void DrawableGL::issueUpload(gfx::UploadPass& uploadPass) {
    if (isCustom) {
        return;
    }
    if (!shader) {
        Log::Warning(Event::General, "Missing shader for drawable " + util::toString(getID()) + "/" + getName());
        assert(false);
        return;
    }

    MLN_TRACE_FUNC();

    constexpr auto usage = gfx::BufferUsageType::StaticDraw;
    assert(impl);

    // Create an index buffer if necessary
    if (impl->indexes) {
        impl->indexes->updateModified();
    }
    if (impl->indexes &&
        (!impl->indexes->getBuffer() || !attributeUpdateTime || impl->indexes->isModifiedAfter(*attributeUpdateTime))) {
        MLN_TRACE_ZONE(build indexes);
        auto indexBufferResource{
            uploadPass.createIndexBufferResource(impl->indexes->data(), impl->indexes->bytes(), usage)};
        auto indexBuffer = std::make_unique<gfx::IndexBuffer>(impl->indexes->elements(),
                                                              std::move(indexBufferResource));
        auto buffer = std::make_unique<IndexBufferGL>(std::move(indexBuffer));
        impl->indexes->setBuffer(std::move(buffer));
    }

    // Build the vertex attributes and bindings, if necessary
    if (impl->attributeBindings.empty() ||
        (vertexAttributes && (!attributeUpdateTime || vertexAttributes->isModifiedAfter(*attributeUpdateTime)))) {
        MLN_TRACE_ZONE(build attributes);

        // Apply drawable values to shader defaults
        const auto& defaults = shader->getVertexAttributes();
        const auto& overrides = *vertexAttributes;

        const auto& indexAttribute = defaults.get(impl->vertexAttrId);
        const auto vertexAttributeIndex = static_cast<std::size_t>(indexAttribute ? indexAttribute->getIndex() : -1);

        std::vector<std::unique_ptr<gfx::VertexBufferResource>> vertexBuffers;
        impl->attributeBindings = uploadPass.buildAttributeBindings(impl->vertexCount,
                                                                    impl->vertexType,
                                                                    vertexAttributeIndex,
                                                                    impl->vertexData,
                                                                    defaults,
                                                                    overrides,
                                                                    usage,
                                                                    attributeUpdateTime,
                                                                    vertexBuffers);

        impl->attributeBuffers = std::move(vertexBuffers);
    }

    const auto needsUpload = [](const auto& texture) {
        return texture && texture->needsUpload();
    };
    if (std::any_of(textures.begin(), textures.end(), needsUpload)) {
        uploadTextures();
    }

    attributeUpdateTime = util::MonotonicTimer::now();
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
    MLN_TRACE_FUNC();

    for (const auto& texture : textures) {
        if (texture) {
            texture->upload();
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
