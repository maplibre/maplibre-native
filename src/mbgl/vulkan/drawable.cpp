#include <mbgl/vulkan/drawable.hpp>

#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/depth_mode.hpp>
#include <mbgl/vulkan/command_encoder.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/drawable_impl.hpp>
#include <mbgl/vulkan/render_pass.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>
#include <mbgl/vulkan/upload_pass.hpp>
#include <mbgl/vulkan/index_buffer_resource.hpp>
#include <mbgl/vulkan/vertex_buffer_resource.hpp>
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

    if (value)
        impl->pipelineInfo.colorMask = vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags;
    else
        impl->pipelineInfo.colorMask = vk::ColorComponentFlags();
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

void Drawable::setEnableStencil(bool value) {
    gfx::Drawable::setEnableStencil(value);
    impl->pipelineInfo.depthTest = value;
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

    const auto& shaderImpl = static_cast<const ShaderProgram&>(*shader);
    const auto& shaderUniforms = shaderImpl.getUniformBlocks();

    auto& uploadPass = static_cast<UploadPass&>(uploadPass_);
    auto& context = static_cast<Context&>(uploadPass.getContext());
    constexpr auto usage = gfx::BufferUsageType::StaticDraw;

    // We need either raw index data or a buffer already created from them.
    // We can have a buffer and no indexes, but only if it's not marked dirty.
    if (!impl->indexes || (impl->indexes->empty() && (!impl->indexes->getBuffer() || impl->indexes->getDirty()))) {
        assert(!"Missing index data");
        return;
    }

    if (impl->indexes->getDirty()) {
        // Create or update a buffer for the index data.  We don't update any
        // existing buffer because it may still be in use by the previous frame.
        impl->indexes->getBuffer();

        auto indexBufferResource{uploadPass.createIndexBufferResource(
            impl->indexes->data(), impl->indexes->bytes(), usage, /*persistent=*/false)};
        auto indexBuffer = std::make_unique<gfx::IndexBuffer>(impl->indexes->elements(),
                                                              std::move(indexBufferResource));
        auto buffer = std::make_unique<IndexBuffer>(std::move(indexBuffer));

        impl->indexes->setBuffer(std::move(buffer));
        impl->indexes->setDirty(false);
    }

    const bool buildAttribs = !vertexAttributes || vertexAttributes->isDirty();

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
                                                                    vertexBuffers);
        impl->attributeBuffers = std::move(vertexBuffers);

        vertexAttributes->visitAttributes([](gfx::VertexAttribute& attrib) { attrib.setDirty(false); });

        if (impl->attributeBindings != attributeBindings_) {
            impl->attributeBindings = std::move(attributeBindings_);
        }
    }

    // build instance buffer
    const bool buildInstanceBuffer = (instanceAttributes && instanceAttributes->isDirty());

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
                                                                   instanceBuffers);
        impl->instanceBuffers = std::move(instanceBuffers);

        // clear dirty flag
        instanceAttributes->visitAttributes([](gfx::VertexAttribute& attrib) { attrib.setDirty(false); });

        if (impl->instanceBindings != instanceBindings_) {
            impl->instanceBindings = std::move(instanceBindings_);
        }
    }

    const bool texturesNeedUpload = std::any_of(
        textures.begin(), textures.end(), [](const auto& texture) { return texture && texture->needsUpload(); });

    if (texturesNeedUpload) {
        uploadTextures(uploadPass);
    }
}

void Drawable::draw(PaintParameters& parameters) const {
    if (isCustom) {
        return;
    }

    auto& context = static_cast<Context&>(parameters.context);
    auto& renderPass = static_cast<RenderPass&>(*parameters.renderPass);
    auto& encoder = renderPass.getEncoder();
    auto& commandBuffer = encoder.getCommandBuffer();

    auto& shaderImpl = static_cast<mbgl::vulkan::ShaderProgram&>(*shader);
    
    bindAttributes(encoder);
    bindUniformBuffers(encoder);
    bindTextures(encoder);

    const uint32_t instanceCount = instanceAttributes ? instanceAttributes->getMaxCount() : 1;

    for (const auto& seg : impl->segments) {
        const auto& mode = seg->getMode();
        const auto& segment = seg->getSegment();

        // update pipeline info with per segment modifiers
        impl->pipelineInfo.setTopology(mode.type);

        const auto& pipeline = shaderImpl.getPipeline(impl->pipelineInfo);

        commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.get());

        if (impl->pipelineInfo.wideLines) {
            commandBuffer->setLineWidth(impl->pipelineInfo.dynamicValues.lineWidth.value());
        }

        if (impl->pipelineInfo.dynamicValues.blendConstants.has_value()) {
            commandBuffer->setBlendConstants(impl->pipelineInfo.dynamicValues.blendConstants.value().data());
        }

        if (segment.indexLength) {
            commandBuffer->drawIndexed(segment.indexLength, instanceCount, segment.indexOffset, segment.vertexOffset, 0);
        } else {
            commandBuffer->draw(segment.vertexLength, instanceCount, segment.vertexOffset, 0);
        }

        context.renderingStats().numDrawCalls++;
    }
}

void Drawable::setIndexData(gfx::IndexVectorBasePtr indexes, std::vector<UniqueDrawSegment> segments) {
    assert(indexes && indexes->elements());
    impl->indexes = std::move(indexes);
    impl->segments = std::move(segments);
}

void Drawable::setVertices(std::vector<uint8_t>&& data, std::size_t count, gfx::AttributeDataType type) {
    impl->vertexCount = count;
    impl->vertexType = type;

    if (count && type != gfx::AttributeDataType::Invalid && !data.empty()) {
        if (!vertexAttributes) {
            vertexAttributes = std::make_shared<VertexAttributeArray>();
        }
        if (auto& attrib = vertexAttributes->set(impl->vertexAttrId, /*index=*/-1, type)) {
            attrib->setRawData(std::move(data));
            attrib->setStride(VertexAttribute::getStrideOf(type));
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

void Drawable::bindAttributes(CommandEncoder& encoder) const noexcept {
    auto& context = encoder.getContext();
    const auto& commandBuffer = encoder.getCommandBuffer();

    std::vector<vk::Buffer> vertexBuffers;
    std::vector<vk::DeviceSize> vertexOffsets;

    const auto addAttributes = [&](const auto& bindings) {
        for (const auto& binding : bindings) {
            if (binding) {
                const auto& buffer = static_cast<const VertexBufferResource*>(binding->vertexBufferResource);
                vertexBuffers.push_back(buffer->get().getVulkanBuffer());
            } else {
                const auto& buffer = context.getDummyVertexBuffer();
                vertexBuffers.push_back(buffer->getVulkanBuffer());
            }

            vertexOffsets.push_back(0u);
        }
    };

    addAttributes(impl->attributeBindings);
    addAttributes(impl->instanceBindings);

    if (!vertexBuffers.empty())
        commandBuffer->bindVertexBuffers(0, vertexBuffers, vertexOffsets);

    if (impl->indexes) {
        const auto* indexBuffer = static_cast<const IndexBuffer*>(impl->indexes->getBuffer());
        const auto& indexBufferResource = indexBuffer->buffer->getResource<IndexBufferResource>().get();
        commandBuffer->bindIndexBuffer(indexBufferResource.getVulkanBuffer(), 0, vk::IndexType::eUint16);
    }
}

void Drawable::bindUniformBuffers(CommandEncoder& encoder) const noexcept {
    if (!shader)
        return;

    auto& context = encoder.getContext();
    const auto& device = context.getBackend().getDevice();
    const auto& descriptorPool = context.getCurrentDescriptorPool();
    auto* shaderImpl = static_cast<ShaderProgram*>(shader.get());
    const auto& descriptorSetLayouts = shaderImpl->getDescriptorSetLayouts();

    const auto descriptorAllocInfo = vk::DescriptorSetAllocateInfo()
        .setDescriptorPool(*descriptorPool)
        .setSetLayouts(descriptorSetLayouts);

    const auto& drawableDescriptorSets = device->allocateDescriptorSets(descriptorAllocInfo);

    const auto updateDescriptors = [&](const auto& buffer, bool fillGaps) {
        for (size_t id = 0; id < buffer.allocatedSize(); ++id) {
            if (id >= drawableDescriptorSets.size())
                continue;

            auto& descriptorBufferInfo = vk::DescriptorBufferInfo()
                .setOffset(0)
                .setRange(VK_WHOLE_SIZE);

            if (const auto& uniformBuffer = buffer.get(id)) {
                const auto& uniformBufferImpl = static_cast<const UniformBuffer&>(*uniformBuffer);
                const auto& bufferResource = uniformBufferImpl.getBufferResource();
                descriptorBufferInfo.setBuffer(bufferResource.getVulkanBuffer());
            } else if (fillGaps) {
                descriptorBufferInfo.setBuffer(context.getDummyUniformBuffer()->getVulkanBuffer());
            } else {
                continue;
            }

            const auto& writeDescriptorSet = vk::WriteDescriptorSet()
                .setBufferInfo(descriptorBufferInfo)
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setDstBinding(0)
                .setDstSet(drawableDescriptorSets[id]);

            device->updateDescriptorSets(writeDescriptorSet, nullptr);
        }
    };

    updateDescriptors(context.getGlobalUniformBuffers(), false);
    updateDescriptors(getUniformBuffers(), true);

    if (drawableDescriptorSets.empty())
        return;

    // merge this with context.globalUniforms
    const auto& commandBuffer = encoder.getCommandBuffer();
    commandBuffer->bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, 
        shaderImpl->getPipelineLayout().get(),
        0,
        drawableDescriptorSets,
        nullptr
    );
}

void Drawable::bindTextures(CommandEncoder& encoder) const noexcept {
    /* for (size_t id = 0; id < textures.size(); id++) {
        if (const auto& texture = textures[id]) {
            if (const auto& location = shader->getSamplerLocation(id)) {
                static_cast<mtl::Texture2D&>(*texture).bind(renderPass, static_cast<int32_t>(*location));
            }
        }
    }*/
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
