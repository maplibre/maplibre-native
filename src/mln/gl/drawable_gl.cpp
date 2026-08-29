#include <mln/gl/drawable_gl.hpp>
#include <mln/gl/drawable_gl_impl.hpp>
#include <mln/gl/index_buffer_resource.hpp>
#include <mln/gl/texture2d.hpp>
#include <mln/gl/upload_pass.hpp>
#include <mln/gl/vertex_array.hpp>
#include <mln/gl/vertex_attribute_gl.hpp>
#include <mln/gl/vertex_buffer_resource.hpp>
#include <mln/plugin/plugin_drawable_data.hpp>
#include <mln/shaders/segment.hpp>
#include <mln/shaders/gl/shader_program_gl.hpp>
#include <mln/util/instrumentation.hpp>
#include <mln/util/logging.hpp>

namespace mln {
namespace gl {

DrawableGL::DrawableGL(std::string name_)
    : Drawable(std::move(name_)),
      impl(std::make_unique<Impl>()) {}

DrawableGL::~DrawableGL() {
    impl->attributeBuffers.clear();
}

struct IndexBufferGL : public gfx::IndexBufferBase {
    IndexBufferGL(std::unique_ptr<gfx::IndexBuffer>&& buffer_)
        : buffer(std::move(buffer_)) {}
    ~IndexBufferGL() override = default;

    std::unique_ptr<mln::gfx::IndexBuffer> buffer;
};

namespace {

mln_plugin_attribute_type pluginAttributeType(gfx::AttributeDataType type) {
    switch (type) {
        case gfx::AttributeDataType::Short2:
            return MLN_PLUGIN_ATTRIBUTE_INT16_X2;
        case gfx::AttributeDataType::UShort2:
            return MLN_PLUGIN_ATTRIBUTE_UINT16_X2;
        case gfx::AttributeDataType::Float:
            return MLN_PLUGIN_ATTRIBUTE_FLOAT;
        case gfx::AttributeDataType::Float2:
            return MLN_PLUGIN_ATTRIBUTE_FLOAT_X2;
        default:
            return MLN_PLUGIN_ATTRIBUTE_NONE;
    }
}

mln_plugin_buffer_binding_v1 pluginBinding(const gfx::AttributeBindingArray& bindings,
                                           int8_t location,
                                           std::size_t segmentVertexOffset) {
    mln_plugin_buffer_binding_v1 result{};
    result.struct_size = sizeof(result);
    if (location < 0 || static_cast<std::size_t>(location) >= bindings.size()) return result;
    const auto& binding = bindings[location];
    if (!binding || !binding->vertexBufferResource) return result;
    const auto& resource = static_cast<const VertexBufferResource&>(*binding->vertexBufferResource);
    result.buffer = resource.getBuffer().get();
    result.offset = binding->attribute.offset + segmentVertexOffset * binding->vertexStride;
    result.stride = binding->vertexStride;
    result.type = pluginAttributeType(binding->attribute.dataType);
    return result;
}

} // namespace

void DrawableGL::collectPluginDrawPackets(std::vector<mln_plugin_draw_packet_v1>& packets) const {
    const auto* metadata = drawableData ? drawableData->getPluginData() : nullptr;
    if (!metadata || !impl->indexes || !impl->indexes->getBuffer()) return;
    const auto& indexBuffer = static_cast<const IndexBufferGL&>(*impl->indexes->getBuffer());
    if (!indexBuffer.buffer) return;
    const auto& resource = indexBuffer.buffer->getResource<IndexBufferResource>();

    for (const auto& drawSegment : impl->segments) {
        const auto& segment = drawSegment->getSegment();
        if (!segment.indexLength) continue;
        auto packet = metadata->packet;
        packet.index_buffer = resource.buffer.get();
        packet.index_offset = segment.indexOffset * sizeof(std::uint16_t);
        packet.index_count = static_cast<uint32_t>(segment.indexLength);
        packet.instance_count = 1;
        packet.base_vertex = 0;
        packet.wall_vertex = pluginBinding(impl->attributeBindings, metadata->wallVertexLocation, segment.vertexOffset);
        packet.position = pluginBinding(impl->attributeBindings, metadata->positionLocation, segment.vertexOffset);
        packet.decimals_edge = pluginBinding(impl->attributeBindings, metadata->decimalsLocation, segment.vertexOffset);
        packet.normal = pluginBinding(impl->attributeBindings, metadata->normalLocation, segment.vertexOffset);
        packet.base = pluginBinding(impl->attributeBindings, metadata->baseLocation, segment.vertexOffset);
        packet.height = pluginBinding(impl->attributeBindings, metadata->heightLocation, segment.vertexOffset);
        packets.push_back(packet);
    }
}

void DrawableGL::draw(PaintParameters& parameters) const {
    MLN_TRACE_FUNC();

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
        mln::Log::Warning(Event::General, "Missing shader for drawable " + util::toString(getID()) + "/" + getName());
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

    context.setScissorTest(parameters.scissorRect);

    impl->uniformBuffers.bind();
    bindTextures();

    for (const auto& seg : impl->segments) {
        const auto& glSeg = static_cast<DrawSegmentGL&>(*seg);
        const auto& mlSeg = glSeg.getSegment();
        if (mlSeg.indexLength > 0 && glSeg.getVertexArray().isValid()) {
            context.bindVertexArray = glSeg.getVertexArray().getID();
            context.draw(glSeg.getMode(), mlSeg.indexOffset, mlSeg.indexLength);
        }
    }
    // Unbind the VAO so that future buffer commands outside Drawable do not change the current VAO state
    context.bindVertexArray = value::BindVertexArray::Default;

    unbindTextures();
    impl->uniformBuffers.unbind();
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
            seg.baseInstance,
            seg.instanceCount,
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

void DrawableGL::upload(gfx::UploadPass& uploadPass) {
    if (isCustom) {
        return;
    }
    if (!shader) {
        Log::Warning(Event::General, "Missing shader for drawable " + util::toString(getID()) + "/" + getName());
        assert(false);
        return;
    }

    MLN_TRACE_FUNC();
#ifdef MLN_TRACY_ENABLE
    {
        auto str = name + "/" + (tileID ? util::toString(*tileID) : std::string());
        MLN_ZONE_STR(str);
    }
#endif

    auto& context = uploadPass.getContext();
    auto& glContext = static_cast<gl::Context&>(context);
    constexpr auto usage = gfx::BufferUsageType::StaticDraw;

    // Create an index buffer if necessary}
    if (impl->indexes && (!impl->indexes->getBuffer() || impl->indexes->getDirty())) {
        MLN_TRACE_ZONE(build indexes);
        auto indexBufferResource{
            uploadPass.createIndexBufferResource(impl->indexes->data(), impl->indexes->bytes(), usage)};
        auto indexBuffer = std::make_unique<gfx::IndexBuffer>(impl->indexes->elements(),
                                                              std::move(indexBufferResource));
        auto buffer = std::make_unique<IndexBufferGL>(std::move(indexBuffer));
        impl->indexes->setBuffer(std::move(buffer));
        impl->indexes->setDirty(false);
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

    // Bind a VAO for each group of vertexes described by a segment
    for (const auto& seg : impl->segments) {
        MLN_TRACE_ZONE(segment);
        auto& glSeg = static_cast<DrawSegmentGL&>(*seg);
        const auto& mlSeg = glSeg.getSegment();

        if (mlSeg.indexLength == 0) {
            continue;
        }

        for (auto& binding : impl->attributeBindings) {
            if (binding) {
                binding->vertexOffset = static_cast<uint32_t>(mlSeg.vertexOffset);
            }
        }

        if (!glSeg.getVertexArray().isValid() && impl->indexes) {
            auto vertexArray = glContext.createVertexArray();
            const auto& indexBuffer = static_cast<IndexBufferGL&>(*impl->indexes->getBuffer());
            vertexArray.bind(glContext, *indexBuffer.buffer, impl->attributeBindings);
            assert(vertexArray.isValid());
            if (vertexArray.isValid()) {
                glSeg.setVertexArray(std::move(vertexArray));
            }
        }
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
    if (enableStencil && parameters.stencilClippingAvailable) {
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
} // namespace mln
