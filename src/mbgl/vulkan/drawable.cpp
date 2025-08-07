#include <mbgl/vulkan/drawable.hpp>

#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/depth_mode.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/vulkan/command_encoder.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/drawable_impl.hpp>
#include <mbgl/vulkan/render_pass.hpp>
#include <mbgl/vulkan/renderable_resource.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>
#include <mbgl/vulkan/upload_pass.hpp>
#include <mbgl/vulkan/index_buffer_resource.hpp>
#include <mbgl/vulkan/vertex_buffer_resource.hpp>
#include <mbgl/vulkan/texture2d.hpp>
#include <mbgl/shaders/vulkan/shader_program.hpp>
#include <mbgl/shaders/segment.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/variant.hpp>
#include <mbgl/util/hash.hpp>
#include <mbgl/util/instrumentation.hpp>

#include <cassert>
#if !defined(NDEBUG)
#include <sstream>
#endif

namespace mbgl {
namespace vulkan {

struct IndexBuffer : public gfx::IndexBufferBase {
    IndexBuffer(std::unique_ptr<gfx::IndexBuffer>&& buffer_)
        : buffer(std::move(buffer_)) {}
    ~IndexBuffer() override = default;

    std::unique_ptr<mbgl::gfx::IndexBuffer> buffer;
};

#if !defined(NDEBUG)
static std::string debugLabel(const gfx::Drawable& drawable) {
    std::ostringstream oss;
    oss << drawable.getID().id() << "/" << drawable.getName() << "/tile=";

    if (const auto& tileID = drawable.getTileID()) {
        oss << util::toString(*tileID);
    } else {
        oss << "(none)";
    }

    return oss.str();
}
#endif // !defined(NDEBUG)

Drawable::Drawable(std::string name_)
    : gfx::Drawable(std::move(name_)),
      impl(std::make_unique<Impl>()) {}

Drawable::~Drawable() {}

void Drawable::setEnableColor(bool value) {
    gfx::Drawable::setEnableColor(value);

    if (value) {
        impl->pipelineInfo.colorMask = vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags;
    } else {
        impl->pipelineInfo.colorMask = vk::ColorComponentFlags();
    }
}

void Drawable::setColorMode(const gfx::ColorMode& value) {
    gfx::Drawable::setColorMode(value);
    impl->pipelineInfo.setColorBlend(value);
}

void Drawable::setEnableDepth(bool value) {
    gfx::Drawable::setEnableDepth(value);
    impl->pipelineInfo.depthTest = value;
}

void Drawable::setDepthType(gfx::DepthMaskType value) {
    gfx::Drawable::setDepthType(value);
    impl->pipelineInfo.setDepthWrite(value);
}

void Drawable::setDepthModeFor3D(const gfx::DepthMode& value) {
    impl->depthFor3D = value;
}

void Drawable::setStencilModeFor3D(const gfx::StencilMode& value) {
    impl->stencilFor3D = value;
}

void Drawable::setLineWidth(int32_t value) {
    gfx::Drawable::setLineWidth(value);
    impl->pipelineInfo.wideLines = value != 1;
    impl->pipelineInfo.dynamicValues.lineWidth = static_cast<float>(value);
}

void Drawable::setCullFaceMode(const gfx::CullFaceMode& mode) {
    gfx::Drawable::setCullFaceMode(mode);
    impl->pipelineInfo.setCullMode(mode);
}

void Drawable::updateVertexAttributes(gfx::VertexAttributeArrayPtr vertices,
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
        drawSegs.push_back(std::make_unique<Drawable::DrawSegment>(mode, std::move(segCopy)));
    }

    impl->indexes = std::move(indexes);
    impl->segments = std::move(drawSegs);
}

void Drawable::upload(gfx::UploadPass& uploadPass_) {
    MLN_TRACE_FUNC();

    if (isCustom) {
        return;
    }

    if (!shader) {
        Log::Warning(Event::General, "Missing shader for drawable " + util::toString(getID()) + "/" + getName());
        assert(false);
        return;
    }

    auto& uploadPass = static_cast<UploadPass&>(uploadPass_);
    constexpr auto usage = gfx::BufferUsageType::StaticDraw;

    // We need either raw index data or a buffer already created from them.
    // We can have a buffer and no indexes, but only if it's not marked dirty.
    if (!impl->indexes || (impl->indexes->empty() && (!impl->indexes->getBuffer() || impl->indexes->getDirty()))) {
        assert(!"Missing index data");
        return;
    }

    if (!impl->indexes->getBuffer() || impl->indexes->getDirty()) {
        // Create a buffer for the index data.  We don't update any
        // existing buffer because it may still be in use by the previous frame.
        auto indexBufferResource{uploadPass.createIndexBufferResource(
            impl->indexes->data(), impl->indexes->bytes(), usage, /*persistent=*/false)};
        auto indexBuffer = std::make_unique<gfx::IndexBuffer>(impl->indexes->elements(),
                                                              std::move(indexBufferResource));
        auto buffer = std::make_unique<IndexBuffer>(std::move(indexBuffer));

        impl->indexes->setBuffer(std::move(buffer));
        impl->indexes->setDirty(false);
    }

    const bool buildAttribs = !vertexAttributes || !attributeUpdateTime ||
                              vertexAttributes->isModifiedAfter(*attributeUpdateTime) ||
                              impl->pipelineInfo.inputAttributes.empty();

    if (buildAttribs) {
#if !defined(NDEBUG)
        const auto debugGroup = uploadPass.createDebugGroup(debugLabel(*this));
#endif

        if (!vertexAttributes) {
            vertexAttributes = std::make_shared<VertexAttributeArray>();
        }

        // Apply drawable values to shader defaults
        std::vector<std::unique_ptr<gfx::VertexBufferResource>> vertexBuffers;
        auto attributeBindings_ = uploadPass.buildAttributeBindings(impl->vertexCount,
                                                                    impl->vertexType,
                                                                    /*vertexAttributeIndex=*/-1,
                                                                    /*vertexData=*/{},
                                                                    shader->getVertexAttributes(),
                                                                    *vertexAttributes,
                                                                    usage,
                                                                    attributeUpdateTime,
                                                                    vertexBuffers);

        vertexAttributes->visitAttributes([](gfx::VertexAttribute& attrib) { attrib.setDirty(false); });

        if (impl->attributeBindings != attributeBindings_) {
            impl->attributeBindings = std::move(attributeBindings_);
        }
    }

    // build instance buffer
    const bool buildInstanceBuffer =
        (instanceAttributes && (!attributeUpdateTime || instanceAttributes->isModifiedAfter(*attributeUpdateTime)));

    if (buildInstanceBuffer) {
        // Build instance attribute buffers
        std::vector<std::unique_ptr<gfx::VertexBufferResource>> instanceBuffers;
        auto instanceBindings_ = uploadPass.buildAttributeBindings(instanceAttributes->getMaxCount(),
                                                                   /*vertexType*/ gfx::AttributeDataType::Byte,
                                                                   /*vertexAttributeIndex=*/-1,
                                                                   /*vertexData=*/{},
                                                                   shader->getInstanceAttributes(),
                                                                   *instanceAttributes,
                                                                   usage,
                                                                   attributeUpdateTime,
                                                                   instanceBuffers);

        // clear dirty flag
        instanceAttributes->visitAttributes([](gfx::VertexAttribute& attrib) { attrib.setDirty(false); });

        if (impl->instanceBindings != instanceBindings_) {
            impl->instanceBindings = std::move(instanceBindings_);
        }
    }

    if (buildAttribs || buildInstanceBuffer) {
        buildVulkanInputBindings();
    }

    const bool texturesNeedUpload = std::any_of(
        textures.begin(), textures.end(), [](const auto& texture) { return texture && texture->needsUpload(); });

    if (texturesNeedUpload) {
        uploadTextures(uploadPass);
    }

    attributeUpdateTime = util::MonotonicTimer::now();
}

void Drawable::draw(PaintParameters& parameters) const {
    MLN_TRACE_FUNC();

    if (isCustom) {
        return;
    }

    auto& context = static_cast<Context&>(parameters.context);
    auto& dispatcher = context.getBackend().getDispatcher();
    auto& renderPass_ = static_cast<RenderPass&>(*parameters.renderPass);
    auto& encoder = renderPass_.getEncoder();
    auto& commandBuffer = encoder.getCommandBuffer();

    auto& shaderImpl = static_cast<mbgl::vulkan::ShaderProgram&>(*shader);

    if (!bindAttributes(encoder)) return;
    if (!bindDescriptors(encoder)) return;

    commandBuffer->pushConstants(
        context.getGeneralPipelineLayout().get(),
        vk::ShaderStageFlags() | vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
        0,
        sizeof(uboIndex),
        &uboIndex,
        dispatcher);

    if (enableDepth) {
        if (impl->depthFor3D.has_value()) {
            impl->pipelineInfo.setDepthMode(impl->depthFor3D.value());
        } else if (is3D) {
            impl->pipelineInfo.setDepthMode(parameters.depthModeFor3D());
        } else {
            const auto& depthMode = parameters.depthModeForSublayer(getSubLayerIndex(), getDepthType());
            impl->pipelineInfo.setDepthMode(depthMode);
        }
    } else {
        impl->pipelineInfo.setDepthMode(gfx::DepthMode::disabled());
    }

    if (enableStencil) {
        if (impl->stencilFor3D.has_value()) {
            impl->pipelineInfo.setStencilMode(impl->stencilFor3D.value());
        } else if (is3D) {
            impl->pipelineInfo.setStencilMode(parameters.stencilModeFor3D());
        } else {
            const auto& stencilMode = parameters.stencilModeForClipping(tileID->toUnwrapped());
            impl->pipelineInfo.setStencilMode(stencilMode);
        }
    } else {
        impl->pipelineInfo.setStencilMode(gfx::StencilMode::disabled());
    }

    impl->pipelineInfo.setRenderable(renderPass_.getDescriptor().renderable);

    const auto instances = instanceAttributes ? instanceAttributes->getMaxCount() : 1;

    for (const auto& seg : impl->segments) {
        const auto& segment = seg->getSegment();

        // update pipeline info with per segment modifiers
        impl->pipelineInfo.setDrawMode(seg->getMode());

        impl->pipelineInfo.setScissorRect(parameters.scissorRect);

        impl->pipelineInfo.setDynamicValues(context.getBackend(), commandBuffer);

        const auto& pipeline = shaderImpl.getPipeline(impl->pipelineInfo);
        commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.get(), dispatcher);

        if (segment.indexLength) {
            commandBuffer->drawIndexed(static_cast<uint32_t>(segment.indexLength),
                                       static_cast<uint32_t>(instances),
                                       static_cast<uint32_t>(segment.indexOffset),
                                       static_cast<int32_t>(segment.vertexOffset),
                                       0,
                                       dispatcher);
        } else {
            commandBuffer->draw(static_cast<uint32_t>(segment.vertexLength),
                                static_cast<uint32_t>(instances),
                                static_cast<uint32_t>(segment.vertexOffset),
                                0,
                                dispatcher);
        }

        context.renderingStats().numDrawCalls++;
    }
}

void Drawable::setIndexData(gfx::IndexVectorBasePtr indexes, std::vector<UniqueDrawSegment> segments) {
    assert(indexes && indexes->elements());
    impl->indexes = std::move(indexes);
    impl->segments = std::move(segments);
}

void Drawable::setVertices(std::vector<uint8_t>&& data, std::size_t count, gfx::AttributeDataType type_) {
    impl->vertexCount = count;
    impl->vertexType = type_;

    if (count && type_ != gfx::AttributeDataType::Invalid && !data.empty()) {
        if (!vertexAttributes) {
            vertexAttributes = std::make_shared<VertexAttributeArray>();
        }
        if (auto& attrib = vertexAttributes->set(impl->vertexAttrId, /*index=*/-1, type_)) {
            attrib->setRawData(std::move(data));
            attrib->setStride(VertexAttribute::getStrideOf(type_));
        } else {
            using namespace std::string_literals;
            Log::Warning(Event::General,
                         "Vertex attribute type mismatch: "s + name + " / " + util::toString(impl->vertexAttrId));
            assert(false);
        }
    }
}

const gfx::UniformBufferArray& Drawable::getUniformBuffers() const {
    return impl->uniformBuffers;
}

gfx::UniformBufferArray& Drawable::mutableUniformBuffers() {
    return impl->uniformBuffers;
}

void Drawable::buildVulkanInputBindings() noexcept {
    MLN_TRACE_FUNC();

    impl->vulkanVertexBuffers.clear();
    impl->vulkanVertexOffsets.clear();

    impl->pipelineInfo.inputAttributes.clear();
    impl->pipelineInfo.inputBindings.clear();

    std::vector<const gfx::VertexBufferResource*> uniqueBuffers;

    const auto buildBindings = [&](const gfx::AttributeBindingArray& bindings, vk::VertexInputRate inputRate) {
        for (size_t i = 0; i < bindings.size(); ++i) {
            const auto& binding = bindings[i];
            if (!binding.has_value() || !binding->vertexBufferResource) continue;

            const auto& vertexBuffer = static_cast<const VertexBufferResource*>(binding->vertexBufferResource);
            const auto& buffer = vertexBuffer->get();

            const auto& buffIt = std::find(uniqueBuffers.begin(), uniqueBuffers.end(), binding->vertexBufferResource);
            std::size_t bindingIndex = 0;

            if (buffIt == uniqueBuffers.end()) {
                bindingIndex = impl->pipelineInfo.inputBindings.size();

                uniqueBuffers.push_back(binding->vertexBufferResource);

                // add new buffer binding
                impl->pipelineInfo.inputBindings.push_back(vk::VertexInputBindingDescription()
                                                               .setBinding(static_cast<uint32_t>(bindingIndex))
                                                               .setStride(binding->vertexStride)
                                                               .setInputRate(inputRate));

                impl->vulkanVertexBuffers.push_back(buffer.getVulkanBuffer());
                impl->vulkanVertexOffsets.push_back(0u);
            } else {
                bindingIndex = std::distance(uniqueBuffers.begin(), buffIt);
            }

            impl->pipelineInfo.inputAttributes.push_back(
                vk::VertexInputAttributeDescription()
                    .setBinding(static_cast<uint32_t>(bindingIndex))
                    .setLocation(static_cast<uint32_t>(i))
                    .setFormat(PipelineInfo::vulkanFormat(binding->attribute.dataType))
                    .setOffset(binding->attribute.offset));
        }
    };

    buildBindings(impl->attributeBindings, vk::VertexInputRate::eVertex);
    buildBindings(impl->instanceBindings, vk::VertexInputRate::eInstance);

    impl->pipelineInfo.updateVertexInputHash();
}

bool Drawable::bindAttributes(CommandEncoder& encoder) const noexcept {
    MLN_TRACE_FUNC();

    if (impl->vulkanVertexBuffers.empty()) return false;

    const auto& dispatcher = encoder.getContext().getBackend().getDispatcher();
    const auto& commandBuffer = encoder.getCommandBuffer();

    commandBuffer->bindVertexBuffers(0, impl->vulkanVertexBuffers, impl->vulkanVertexOffsets, dispatcher);

    if (impl->indexes) {
        if (const auto* indexBuffer = static_cast<const IndexBuffer*>(impl->indexes->getBuffer())) {
            const auto& indexBufferResource = indexBuffer->buffer->getResource<IndexBufferResource>().get();
            commandBuffer->bindIndexBuffer(
                indexBufferResource.getVulkanBuffer(), 0, vk::IndexType::eUint16, dispatcher);
        }
    }

    return true;
}

bool Drawable::bindDescriptors(CommandEncoder& encoder) const noexcept {
    MLN_TRACE_FUNC();

    if (!shader) return false;

    // bind uniforms
    impl->uniformBuffers.bindDescriptorSets(encoder);

    const auto& shaderImpl = static_cast<const mbgl::vulkan::ShaderProgram&>(*shader);
    if (shaderImpl.hasTextures()) {
        // update image set
        if (!impl->imageDescriptorSet) {
            impl->imageDescriptorSet = std::make_unique<ImageDescriptorSet>(encoder.getContext());
        }

        for (const auto& texture : textures) {
            if (!texture) continue;
            const auto textureImpl = static_cast<const Texture2D*>(texture.get());
            if (textureImpl->isModifiedAfter(impl->imageDescriptorSet->getLastModified())) {
                impl->imageDescriptorSet->markDirty();
                break;
            }
        }

        impl->imageDescriptorSet->update(textures);
        impl->imageDescriptorSet->bind(encoder);
    }

    return true;
}

void Drawable::uploadTextures(UploadPass&) const noexcept {
    MLN_TRACE_FUNC();
    for (const auto& texture : textures) {
        if (texture) {
            texture->upload();
        }
    }
}

} // namespace vulkan
} // namespace mbgl
