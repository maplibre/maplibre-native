#include <mbgl/vulkan/drawable.hpp>

#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/depth_mode.hpp>
#include <mbgl/vulkan/command_encoder.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/drawable_impl.hpp>
#include <mbgl/vulkan/render_pass.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>
#include <mbgl/vulkan/upload_pass.hpp>
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

Drawable::Drawable(std::string name_)
    : gfx::Drawable(std::move(name_)),
      impl(std::make_unique<Impl>()) {}

Drawable::~Drawable() {}

void Drawable::setColorMode(const gfx::ColorMode& value) {
    //impl->pipelineState.reset();
    gfx::Drawable::setColorMode(value);
}

void Drawable::setEnableStencil(bool value) {
    gfx::Drawable::setEnableStencil(value);
    //impl->depthStencilState.reset();
}

void Drawable::setEnableDepth(bool value) {
    gfx::Drawable::setEnableDepth(value);
    //impl->depthStencilState.reset();
}

void Drawable::setSubLayerIndex(int32_t value) {
    gfx::Drawable::setSubLayerIndex(value);
    //impl->depthStencilState.reset();
}

void Drawable::setDepthType(gfx::DepthMaskType value) {
    gfx::Drawable::setDepthType(value);
    //impl->depthStencilState.reset();
}

void Drawable::setShader(gfx::ShaderProgramBasePtr value) {
    //impl->pipelineState.reset();
    gfx::Drawable::setShader(value);
}

void Drawable::draw(PaintParameters& parameters) const {
    if (isCustom) {
        return;
    }

    auto& context = static_cast<Context&>(parameters.context);
    auto& renderPass = static_cast<RenderPass&>(*parameters.renderPass);
//    const auto& encoder = renderPass.getMetalEncoder();
//    if (!encoder) {
//        assert(false);
//        return;
//    }
//
//    if (!shader) {
//        Log::Warning(Event::General, "Missing shader for drawable " + util::toString(getID()) + "/" + getName());
//        assert(false);
//        return;
//    }
//
//    const auto& descriptor = renderPass.getDescriptor();
//    const auto& renderable = descriptor.renderable;
//    if (impl->renderPassDescriptor && descriptor != *impl->renderPassDescriptor) {
//        impl->pipelineState.reset();
//    }
//    impl->renderPassDescriptor.emplace(gfx::RenderPassDescriptor{
//        descriptor.renderable, descriptor.clearColor, descriptor.clearDepth, descriptor.clearStencil});
//
//    const auto& shaderMTL = static_cast<const ShaderProgram&>(*shader);
//
//#if !defined(NDEBUG)
//    const auto debugGroup = parameters.encoder->createDebugGroup(debugLabel(*this));
//#endif
//
//    bindAttributes(renderPass);
//    bindInstanceAttributes(renderPass);
//    bindUniformBuffers(renderPass);
//    bindTextures(renderPass);
//
//    if (!impl->indexes->getBuffer() || impl->indexes->getDirty()) {
//        assert(!"Index buffer not uploaded");
//        return;
//    }
//
//    const auto* indexBuffer = getMetalBuffer(impl->indexes);
//    if (!indexBuffer || impl->indexes->getDirty()) {
//        assert(!"Index buffer not uploaded");
//        return;
//    }
//
//    if (!impl->vertexDesc) {
//        assert(!"Vertex descriptor missing");
//    }
//
//    const auto& cullMode = getCullFaceMode();
//    encoder->setCullMode(cullMode.enabled ? mapCullMode(cullMode.side) : MTL::CullModeNone);
//    encoder->setFrontFacingWinding(mapWindingMode(cullMode.winding));
//
//    if (!impl->pipelineState) {
//        impl->pipelineState = shaderMTL.getRenderPipelineState(
//            renderable,
//            impl->vertexDesc,
//            getColorMode(),
//            mbgl::util::hash(getColorMode().hash(), impl->vertexDescHash));
//    }
//    if (impl->pipelineState) {
//        encoder->setRenderPipelineState(impl->pipelineState.get());
//    } else {
//        assert(!"Failed to create render pipeline state");
//        return;
//    }
//
//    // For 3D mode, stenciling is handled by the layer group
//    if (!is3D) {
//        std::optional<gfx::StencilMode> newStencilMode;
//
//        // If we have a stencil state, we may be able to reuse it, if the tile masks haven't changed.
//        // We assume that only the reference value needs to be compared.
//        if (impl->depthStencilState && enableStencil) {
//            newStencilMode = parameters.stencilModeForClipping(tileID->toUnwrapped());
//            if (newStencilMode->ref != impl->previousStencilMode.ref) {
//                // No, need to rebuild it
//                impl->depthStencilState.reset();
//            }
//        }
//        if (!impl->depthStencilState) {
//            if (enableStencil && !newStencilMode) {
//                newStencilMode = parameters.stencilModeForClipping(tileID->toUnwrapped());
//            }
//            const auto depthMode = getEnableDepth()
//                                       ? parameters.depthModeForSublayer(getSubLayerIndex(), getDepthType())
//                                       : gfx::DepthMode::disabled();
//            const auto stencilMode = enableStencil ? parameters.stencilModeForClipping(tileID->toUnwrapped())
//                                                   : gfx::StencilMode::disabled();
//            impl->depthStencilState = context.makeDepthStencilState(depthMode, stencilMode, renderable);
//            impl->previousStencilMode = *newStencilMode;
//        }
//        renderPass.setDepthStencilState(impl->depthStencilState);
//        renderPass.setStencilReference(impl->previousStencilMode.ref);
//    }
//
//    for (const auto& seg_ : impl->segments) {
//        const auto& segment = static_cast<DrawSegment&>(*seg_);
//        const auto& mlSegment = segment.getSegment();
//        if (mlSegment.indexLength > 0) {
//            const auto& mode = segment.getMode();
//
//            const auto primitiveType = getPrimitiveType(mode.type);
//            constexpr auto indexType = MTL::IndexType::IndexTypeUInt16;
//            constexpr auto indexSize = sizeof(std::uint16_t);
//            const NS::UInteger instanceCount = instanceAttributes ? instanceAttributes->getMaxCount() : 1;
//            constexpr NS::UInteger baseInstance = 0;
//            const NS::UInteger indexOffset = static_cast<NS::UInteger>(indexSize *
//                                                                       mlSegment.indexOffset); // in bytes, not indexes
//            const NS::Integer baseVertex = static_cast<NS::Integer>(mlSegment.vertexOffset);
//
//#if !defined(NDEBUG)
//            const auto indexBufferLength = indexBuffer->length() / indexSize;
//            const auto* indexes = static_cast<const std::uint16_t*>(const_cast<MTL::Buffer*>(indexBuffer)->contents());
//            const auto maxIndex = *std::max_element(indexes + mlSegment.indexOffset,
//                                                    indexes + mlSegment.indexOffset + mlSegment.indexLength);
//
//            // Uncomment for a detailed accounting of each draw call
//            // Log::Warning(Event::General,
//            //             util::toString(getID()) + "/" + getName() +
//            //             " => " + util::toString(mlSegment.indexLength) +
//            //             " idxs @ " + util::toString(mlSegment.indexOffset) +
//            //             " (=" + util::toString(mlSegment.indexLength + mlSegment.indexOffset) +
//            //             " of " + util::toString(indexBufferLength) +
//            //             ") max index " + util::toString(maxIndex) +
//            //             " on base vertex " + util::toString(baseVertex) +
//            //             " (" + util::toString(baseVertex + maxIndex) +
//            //             " of " + util::toString(impl->vertexCount) +
//            //             ") indexBuf=" + util::toString((uint64_t)indexBuffer) +
//            //             "/" + util::toString(indexBuffer->gpuAddress()));
//
//            assert(mlSegment.indexOffset + mlSegment.indexLength <= indexBufferLength);
//            assert(static_cast<std::size_t>(maxIndex) < mlSegment.vertexLength);
//#endif
//
//            if (context.getBackend().isBaseVertexInstanceDrawingSupported()) {
//                encoder->drawIndexedPrimitives(primitiveType,
//                                               mlSegment.indexLength,
//                                               indexType,
//                                               indexBuffer,
//                                               indexOffset,
//                                               instanceCount,
//                                               baseVertex,
//                                               baseInstance);
//            } else {
//                encoder->drawIndexedPrimitives(
//                    primitiveType, mlSegment.indexLength, indexType, indexBuffer, indexOffset, instanceCount);
//            }
//
//            context.renderingStats().numDrawCalls++;
//        }
//    }
//
//    unbindTextures(renderPass);
//    unbindUniformBuffers(renderPass);
//    unbindAttributes(renderPass);
}

void Drawable::setIndexData(gfx::IndexVectorBasePtr indexes, std::vector<UniqueDrawSegment> segments) {
    assert(indexes && indexes->elements());
    impl->indexes = std::move(indexes);
    impl->segments = std::move(segments);
}

void Drawable::setVertices(std::vector<uint8_t>&& data, std::size_t count, gfx::AttributeDataType type) {
    impl->vertexCount = count;
    impl->vertexType = type;

    //if (count && type != gfx::AttributeDataType::Invalid && !data.empty()) {
    //    if (!vertexAttributes) {
    //        vertexAttributes = std::make_shared<VertexAttributeArray>();
    //    }
    //    if (auto& attrib = vertexAttributes->set(impl->vertexAttrId, /*index=*/-1, type)) {
    //        attrib->setRawData(std::move(data));
    //        attrib->setStride(VertexAttribute::getStrideOf(type));
    //    } else {
    //        using namespace std::string_literals;
    //        Log::Warning(Event::General,
    //                     "Vertex attribute type mismatch: "s + name + " / " + util::toString(impl->vertexAttrId));
    //        assert(false);
    //    }
    //}
}

namespace {

class UniformBufferArray : public gfx::UniformBufferArray {
public:
    UniformBufferArray() = default;
    UniformBufferArray(UniformBufferArray&& other)
        : gfx::UniformBufferArray(std::move(other)) {}
    UniformBufferArray(const UniformBufferArray&) = delete;

    UniformBufferArray& operator=(UniformBufferArray&& other) {
        gfx::UniformBufferArray::operator=(std::move(other));
        return *this;
    }
    UniformBufferArray& operator=(const UniformBufferArray& other) {
        gfx::UniformBufferArray::operator=(other);
        return *this;
    }

private:
    gfx::UniqueUniformBuffer copy(const gfx::UniformBuffer& buffer) override {
        return std::make_unique<UniformBuffer>(static_cast<const UniformBuffer&>(buffer).clone());
    }
};


    UniformBufferArray placeholder;
}

const gfx::UniformBufferArray& Drawable::getUniformBuffers() const {
    return placeholder;
}

gfx::UniformBufferArray& Drawable::mutableUniformBuffers() {
    return placeholder;
}

void Drawable::bindAttributes(RenderPass& renderPass) const noexcept {
    //const auto& encoder = renderPass.getMetalEncoder();

    //NS::UInteger attributeIndex = 0;
    //for (const auto& binding : impl->attributeBindings) {
    //    const auto* buffer = static_cast<const mtl::VertexBufferResource*>(binding ? binding->vertexBufferResource
    //                                                                               : nullptr);
    //    if (buffer && buffer->get()) {
    //        assert(binding->vertexStride * impl->vertexCount <= getBufferSize(binding->vertexBufferResource));
    //        renderPass.bindVertex(buffer->get(), /*offset=*/0, attributeIndex);
    //    } else {
    //        const auto* shaderMTL = static_cast<const ShaderProgram*>(shader.get());
    //        auto& context = renderPass.getCommandEncoder().getContext();
    //        renderPass.bindVertex(context.getEmptyBuffer(), /*offset=*/0, attributeIndex);
    //    }
    //    attributeIndex += 1;
    //}
}

void Drawable::bindInstanceAttributes(RenderPass& renderPass) const noexcept {
    //const auto& encoder = renderPass.getMetalEncoder();

    //NS::UInteger attributeIndex = 0;
    //for (const auto& binding : impl->instanceBindings) {
    //    if (binding.has_value()) {
    //        const auto* buffer = static_cast<const mtl::VertexBufferResource*>(binding->vertexBufferResource);
    //        if (buffer && buffer->get()) {
    //            renderPass.bindVertex(buffer->get(), /*offset=*/0, attributeIndex);
    //        } else {
    //            const auto* shaderMTL = static_cast<const ShaderProgram*>(shader.get());
    //            auto& context = renderPass.getCommandEncoder().getContext();
    //            renderPass.bindVertex(context.getEmptyBuffer(), /*offset=*/0, attributeIndex);
    //        }
    //    }
    //    attributeIndex += 1;
    //}
}

void Drawable::bindUniformBuffers(RenderPass& renderPass) const noexcept {
    //if (shader) {
    //    const auto& uniformBlocks = shader->getUniformBlocks();
    //    for (size_t id = 0; id < uniformBlocks.allocatedSize(); id++) {
    //        const auto& block = uniformBlocks.get(id);
    //        if (!block) continue;
    //        const auto& uniformBuffer = getUniformBuffers().get(id);
    //        if (uniformBuffer) {
    //            const auto& buffer = static_cast<UniformBuffer&>(*uniformBuffer.get());
    //            const auto& resource = buffer.getBufferResource();
    //            const auto& mtlBlock = static_cast<const UniformBlock&>(*block);

    //            if (mtlBlock.getBindVertex()) {
    //                renderPass.bindVertex(resource, /*offset=*/0, block->getIndex());
    //            }
    //            if (mtlBlock.getBindFragment()) {
    //                renderPass.bindFragment(resource, /*offset=*/0, block->getIndex());
    //            }
    //        }
    //    }
    //}
}

void Drawable::bindTextures(RenderPass& renderPass) const noexcept {
    /* for (size_t id = 0; id < textures.size(); id++) {
        if (const auto& texture = textures[id]) {
            if (const auto& location = shader->getSamplerLocation(id)) {
                static_cast<mtl::Texture2D&>(*texture).bind(renderPass, static_cast<int32_t>(*location));
            }
        }
    }*/
}

void Drawable::unbindTextures(RenderPass& renderPass) const noexcept {
    /*for (size_t id = 0; id < textures.size(); id++) {
        if (const auto& texture = textures[id]) {
            if (const auto& location = shader->getSamplerLocation(id)) {
                static_cast<mtl::Texture2D&>(*texture).unbind(renderPass, static_cast<int32_t>(*location));
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
