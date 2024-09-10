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
#include <mbgl/programs/segment.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/variant.hpp>
#include <mbgl/util/hash.hpp>

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

void Drawable::upload(gfx::UploadPass& uploadPass_) {
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
    if (!impl->indexes || (impl->indexes->empty() &&
                           (!impl->indexes->getBuffer() || impl->indexes->isModifiedAfter(attributeUpdateTime)))) {
        assert(!"Missing index data");
        return;
    }

    if (!impl->indexes->getBuffer() || impl->indexes->isModifiedAfter(attributeUpdateTime)) {
        // Create a buffer for the index data.  We don't update any
        // existing buffer because it may still be in use by the previous frame.
        auto indexBufferResource{uploadPass.createIndexBufferResource(
            impl->indexes->data(), impl->indexes->bytes(), usage, /*persistent=*/false)};
        auto indexBuffer = std::make_unique<gfx::IndexBuffer>(impl->indexes->elements(),
                                                              std::move(indexBufferResource));
        auto buffer = std::make_unique<IndexBuffer>(std::move(indexBuffer));

        impl->indexes->setBuffer(std::move(buffer));
    }

    const bool buildAttribs = !vertexAttributes || vertexAttributes->isModifiedAfter(attributeUpdateTime) ||
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
    const bool buildInstanceBuffer = (instanceAttributes && instanceAttributes->isModifiedAfter(attributeUpdateTime));

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
    if (isCustom) {
        return;
    }

    auto& context = static_cast<Context&>(parameters.context);
    auto& renderPass_ = static_cast<RenderPass&>(*parameters.renderPass);
    auto& encoder = renderPass_.getEncoder();
    auto& commandBuffer = encoder.getCommandBuffer();

    auto& shaderImpl = static_cast<mbgl::vulkan::ShaderProgram&>(*shader);

    if (!bindAttributes(encoder)) return;
    if (!bindDescriptors(encoder)) return;

    if (is3D) {
        impl->pipelineInfo.setDepthMode(impl->depthFor3D);
        impl->pipelineInfo.setStencilMode(impl->stencilFor3D);
    } else {
        if (enableDepth) {
            const auto& depthMode = parameters.depthModeForSublayer(getSubLayerIndex(), getDepthType());
            impl->pipelineInfo.setDepthMode(depthMode);
        } else {
            impl->pipelineInfo.setDepthMode(gfx::DepthMode::disabled());
        }

        if (enableStencil) {
            const auto& stencilMode = parameters.stencilModeForClipping(tileID->toUnwrapped());
            impl->pipelineInfo.setStencilMode(stencilMode);
        } else {
            impl->pipelineInfo.setStencilMode(gfx::StencilMode::disabled());
        }
    }

    impl->pipelineInfo.setRenderable(renderPass_.getDescriptor().renderable);

    const uint32_t instances = instanceAttributes ? instanceAttributes->getMaxCount() : 1;

    for (const auto& seg : impl->segments) {
        const auto& segment = seg->getSegment();

        // update pipeline info with per segment modifiers
        impl->pipelineInfo.setDrawMode(seg->getMode());

        impl->pipelineInfo.setDynamicValues(context.getBackend(), commandBuffer);

        const auto& pipeline = shaderImpl.getPipeline(impl->pipelineInfo);
        commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.get());

        if (segment.indexLength) {
            commandBuffer->drawIndexed(segment.indexLength, instances, segment.indexOffset, segment.vertexOffset, 0);
        } else {
            commandBuffer->draw(segment.vertexLength, instances, segment.vertexOffset, 0);
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
            uint32_t bindingIndex = 0;

            if (buffIt == uniqueBuffers.end()) {
                bindingIndex = impl->pipelineInfo.inputBindings.size();

                uniqueBuffers.push_back(binding->vertexBufferResource);

                // add new buffer binding
                impl->pipelineInfo.inputBindings.push_back(vk::VertexInputBindingDescription()
                                                               .setBinding(bindingIndex)
                                                               .setStride(binding->vertexStride)
                                                               .setInputRate(inputRate));

                impl->vulkanVertexBuffers.push_back(buffer.getVulkanBuffer());
                impl->vulkanVertexOffsets.push_back(0u);
            } else {
                bindingIndex = std::distance(uniqueBuffers.begin(), buffIt);
            }

            impl->pipelineInfo.inputAttributes.push_back(
                vk::VertexInputAttributeDescription()
                    .setBinding(bindingIndex)
                    .setLocation(i)
                    .setFormat(PipelineInfo::vulkanFormat(binding->attribute.dataType))
                    .setOffset(binding->attribute.offset));
        }
    };

    buildBindings(impl->attributeBindings, vk::VertexInputRate::eVertex);
    buildBindings(impl->instanceBindings, vk::VertexInputRate::eInstance);

    impl->pipelineInfo.updateVertexInputHash();
}

bool Drawable::bindAttributes(CommandEncoder& encoder) const noexcept {
    if (impl->vulkanVertexBuffers.empty()) return false;

    const auto& commandBuffer = encoder.getCommandBuffer();

    commandBuffer->bindVertexBuffers(0, impl->vulkanVertexBuffers, impl->vulkanVertexOffsets);

    if (impl->indexes) {
        if (const auto* indexBuffer = static_cast<const IndexBuffer*>(impl->indexes->getBuffer())) {
            const auto& indexBufferResource = indexBuffer->buffer->getResource<IndexBufferResource>().get();
            commandBuffer->bindIndexBuffer(indexBufferResource.getVulkanBuffer(), 0, vk::IndexType::eUint16);
        }
    }

    return true;
}

bool Drawable::bindDescriptors(CommandEncoder& encoder) const noexcept {
    if (!shader) return false;

    auto& context = encoder.getContext();
    const auto& device = context.getBackend().getDevice();
    const auto& descriptorPool = context.getCurrentDescriptorPool();
    const auto& descriptorSetLayouts = context.getDescriptorSetLayouts();

    const auto descriptorAllocInfo =
        vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool).setSetLayouts(descriptorSetLayouts);

    const auto& drawableDescriptorSets = device->allocateDescriptorSets(descriptorAllocInfo);
    const auto& uniformDescriptorSet = drawableDescriptorSets[0];

    const auto updateUniformDescriptors = [&](const auto& buffer, bool fillGaps) {
        for (size_t id = 0; id < buffer.allocatedSize(); ++id) {
            vk::DescriptorBufferInfo descriptorBufferInfo;

            if (const auto& uniformBuffer = buffer.get(id)) {
                const auto& uniformBufferImpl = static_cast<const UniformBuffer&>(*uniformBuffer);
                const auto& bufferResource = uniformBufferImpl.getBufferResource();
                descriptorBufferInfo.setBuffer(bufferResource.getVulkanBuffer())
                    .setOffset(bufferResource.getVulkanBufferOffset())
                    .setRange(bufferResource.getVulkanBufferSize());
            } else if (fillGaps) {
                descriptorBufferInfo.setBuffer(context.getDummyUniformBuffer()->getVulkanBuffer())
                    .setOffset(0)
                    .setRange(VK_WHOLE_SIZE);
            } else {
                continue;
            }

            const auto writeDescriptorSet = vk::WriteDescriptorSet()
                                                .setBufferInfo(descriptorBufferInfo)
                                                .setDescriptorCount(1)
                                                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                                                .setDstBinding(id)
                                                .setDstSet(uniformDescriptorSet);

            device->updateDescriptorSets(writeDescriptorSet, nullptr);
        }
    };
    const auto& globalUniforms = context.getGlobalUniformBuffers();
    for (size_t i = 0; i < globalUniforms.allocatedSize(); ++i) {
        if (globalUniforms.get(i)) impl->uniformBuffers.set(i, globalUniforms.get(i));
    }

    updateUniformDescriptors(getUniformBuffers(), true);

    if (drawableDescriptorSets.size() >= 2) {
        const auto& imageDescriptorSet = drawableDescriptorSets[1];

        for (size_t id = 0; id < shaders::maxTextureCountPerShader; ++id) {
            const auto& texture = id < textures.size() ? textures[id] : nullptr;
            auto& textureImpl = texture ? static_cast<Texture2D&>(*texture) : *context.getDummyTexture();

            const auto descriptorImageInfo = vk::DescriptorImageInfo()
                                                 .setImageLayout(textureImpl.getVulkanImageLayout())
                                                 .setImageView(textureImpl.getVulkanImageView().get())
                                                 .setSampler(textureImpl.getVulkanSampler());

            const auto writeDescriptorSet = vk::WriteDescriptorSet()
                                                .setImageInfo(descriptorImageInfo)
                                                .setDescriptorCount(1)
                                                .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                                                .setDstBinding(id)
                                                .setDstSet(imageDescriptorSet);

            device->updateDescriptorSets(writeDescriptorSet, nullptr);
        }
    }

    if (drawableDescriptorSets.empty()) return true;

    const auto& commandBuffer = encoder.getCommandBuffer();
    commandBuffer->bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, context.getGeneralPipelineLayout().get(), 0, drawableDescriptorSets, nullptr);

    return true;
}

void Drawable::uploadTextures(UploadPass&) const noexcept {
    for (const auto& texture : textures) {
        if (texture) {
            texture->upload();
        }
    }
}

} // namespace vulkan
} // namespace mbgl
