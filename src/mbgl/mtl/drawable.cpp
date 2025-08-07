#include <mbgl/mtl/drawable.hpp>

#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/depth_mode.hpp>
#include <mbgl/mtl/command_encoder.hpp>
#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/drawable_impl.hpp>
#include <mbgl/mtl/index_buffer_resource.hpp>
#include <mbgl/mtl/render_pass.hpp>
#include <mbgl/mtl/renderable_resource.hpp>
#include <mbgl/mtl/renderer_backend.hpp>
#include <mbgl/mtl/texture2d.hpp>
#include <mbgl/mtl/upload_pass.hpp>
#include <mbgl/mtl/uniform_buffer.hpp>
#include <mbgl/mtl/vertex_buffer_resource.hpp>
#include <mbgl/mtl/vertex_attribute.hpp>
#include <mbgl/shaders/segment.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/variant.hpp>
#include <mbgl/util/hash.hpp>

#include <Metal/Metal.hpp>

#include <simd/simd.h>

#include <cassert>
#if !defined(NDEBUG)
#include <sstream>
#endif

namespace mbgl {
namespace mtl {

struct IndexBuffer : public gfx::IndexBufferBase {
    IndexBuffer(std::unique_ptr<gfx::IndexBuffer>&& buffer_)
        : buffer(std::move(buffer_)) {}
    ~IndexBuffer() override = default;

    std::unique_ptr<mbgl::gfx::IndexBuffer> buffer;
};

Drawable::Drawable(std::string name_)
    : gfx::Drawable(std::move(name_)),
      impl(std::make_unique<Impl>()) {}

Drawable::~Drawable() {}

namespace {
#if !defined(NDEBUG)
std::size_t getBufferSize(const gfx::VertexBufferResource* resource_) {
    if (const auto* resource = static_cast<const VertexBufferResource*>(resource_)) {
        if (const auto& bufferResource = resource->get()) {
            return bufferResource.getSizeInBytes();
        }
    }
    return 0UL;
}
#endif // !defined(NDEBUG)

MTL::PrimitiveType getPrimitiveType(const gfx::DrawModeType type) noexcept {
    switch (type) {
        default:
            assert(false);
            [[fallthrough]];
        case gfx::DrawModeType::Points:
            return MTL::PrimitiveType::PrimitiveTypePoint;
        case gfx::DrawModeType::Lines:
            return MTL::PrimitiveType::PrimitiveTypeLine;
        case gfx::DrawModeType::LineStrip:
            return MTL::PrimitiveType::PrimitiveTypeLineStrip;
        case gfx::DrawModeType::Triangles:
            return MTL::PrimitiveType::PrimitiveTypeTriangle;
    }
}

#if !defined(NDEBUG)
std::string debugLabel(const gfx::Drawable& drawable) {
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

MTL::Buffer* getMetalBuffer(const gfx::IndexVectorBasePtr& indexes) noexcept {
    if (const auto* buf0 = indexes->getBuffer()) {
        if (const auto* buf1 = static_cast<const IndexBuffer*>(buf0)->buffer.get()) {
            const auto& buf2 = buf1->getResource<IndexBufferResource>().get();
            return buf2.getMetalBuffer().get();
        }
    }
    return nullptr;
}

MTL::CullMode mapCullMode(const gfx::CullFaceSideType mode) noexcept {
    switch (mode) {
        case gfx::CullFaceSideType::Front:
            return MTL::CullModeFront;
        case gfx::CullFaceSideType::Back:
            return MTL::CullModeBack;
        default:
        case gfx::CullFaceSideType::FrontAndBack:
            return MTL::CullModeNone;
    }
}

MTL::Winding mapWindingMode(const gfx::CullFaceWindingType mode) noexcept {
    switch (mode) {
        case gfx::CullFaceWindingType::Clockwise:
            return MTL::Winding::WindingClockwise;
        default:
        case gfx::CullFaceWindingType::CounterClockwise:
            return MTL::Winding::WindingCounterClockwise;
    }
}

MTL::ScissorRect getMetalScissorRect(gfx::ScissorRect rect) noexcept {
    return {static_cast<uint32_t>(rect.x), static_cast<uint32_t>(rect.y), rect.width, rect.height};
}

} // namespace

void Drawable::setColorMode(const gfx::ColorMode& value) {
    impl->pipelineState.reset();
    gfx::Drawable::setColorMode(value);
}

void Drawable::setEnableStencil(bool value) {
    gfx::Drawable::setEnableStencil(value);
    impl->depthStencilState.reset();
}

void Drawable::setEnableDepth(bool value) {
    gfx::Drawable::setEnableDepth(value);
    impl->depthStencilState.reset();
}

void Drawable::setSubLayerIndex(int32_t value) {
    gfx::Drawable::setSubLayerIndex(value);
    impl->depthStencilState.reset();
}

void Drawable::setDepthType(gfx::DepthMaskType value) {
    gfx::Drawable::setDepthType(value);
    impl->depthStencilState.reset();
}

void Drawable::setShader(gfx::ShaderProgramBasePtr value) {
    impl->pipelineState.reset();
    gfx::Drawable::setShader(value);
}

void Drawable::draw(PaintParameters& parameters) const {
    if (isCustom) {
        return;
    }

    auto& context = static_cast<Context&>(parameters.context);
    auto& renderPass = static_cast<RenderPass&>(*parameters.renderPass);
    const auto& encoder = renderPass.getMetalEncoder();
    if (!encoder) {
        assert(false);
        return;
    }

    if (!shader) {
        Log::Warning(Event::General, "Missing shader for drawable " + util::toString(getID()) + "/" + getName());
        assert(false);
        return;
    }

    const auto& descriptor = renderPass.getDescriptor();
    const auto& renderable = descriptor.renderable;
    if (impl->renderPassDescriptor && descriptor != *impl->renderPassDescriptor) {
        impl->pipelineState.reset();
    }
    impl->renderPassDescriptor.emplace(gfx::RenderPassDescriptor{
        descriptor.renderable, descriptor.clearColor, descriptor.clearDepth, descriptor.clearStencil});

    const auto& shaderMTL = static_cast<const ShaderProgram&>(*shader);

#if !defined(NDEBUG)
    const auto debugGroup = parameters.encoder->createDebugGroup(debugLabel(*this));
#endif

    renderPass.unbindVertex(shaders::idGlobalUBOIndex);
    renderPass.unbindFragment(shaders::idGlobalUBOIndex);
    encoder->setVertexBytes(&uboIndex, sizeof(uboIndex), shaders::idGlobalUBOIndex);
    encoder->setFragmentBytes(&uboIndex, sizeof(uboIndex), shaders::idGlobalUBOIndex);

    bindAttributes(renderPass);
    bindInstanceAttributes(renderPass);
    bindTextures(renderPass);
    impl->uniformBuffers.bind(renderPass);

    if (!impl->indexes->getBuffer() || impl->indexes->getDirty()) {
        assert(!"Index buffer not uploaded");
        return;
    }

    const auto* indexBuffer = getMetalBuffer(impl->indexes);
    if (!indexBuffer || impl->indexes->getDirty()) {
        assert(!"Index buffer not uploaded");
        return;
    }

    if (!impl->vertexDesc) {
        assert(!"Vertex descriptor missing");
    }

    const auto& cullMode = getCullFaceMode();
    renderPass.setCullMode(cullMode.enabled ? mapCullMode(cullMode.side) : MTL::CullModeNone);
    renderPass.setFrontFacingWinding(mapWindingMode(cullMode.winding));

    renderPass.setScissorRect(getMetalScissorRect(parameters.scissorRect));

    if (!impl->pipelineState) {
        impl->pipelineState = shaderMTL.getRenderPipelineState(
            renderable,
            impl->vertexDesc,
            getColorMode(),
            mbgl::util::hash(getColorMode().hash(), impl->vertexDescHash));
    }
    if (impl->pipelineState) {
        renderPass.setRenderPipelineState(impl->pipelineState);
    } else {
        assert(!"Failed to create render pipeline state");
        return;
    }

    // For 3D mode, stenciling is handled by the layer group
    if (!is3D) {
        std::optional<gfx::StencilMode> newStencilMode;

        // If we have a stencil state, we may be able to reuse it, if the tile masks haven't changed.
        // We assume that only the reference value needs to be compared.
        if (impl->depthStencilState && enableStencil) {
            newStencilMode = parameters.stencilModeForClipping(tileID->toUnwrapped());
            if (newStencilMode->ref != impl->previousStencilMode.ref) {
                // No, need to rebuild it
                impl->depthStencilState.reset();
            }
        }
        if (!impl->depthStencilState) {
            if (enableStencil && !newStencilMode) {
                newStencilMode = parameters.stencilModeForClipping(tileID->toUnwrapped());
            }
            const auto depthMode = getEnableDepth()
                                       ? parameters.depthModeForSublayer(getSubLayerIndex(), getDepthType())
                                       : gfx::DepthMode::disabled();
            const auto stencilMode = enableStencil ? parameters.stencilModeForClipping(tileID->toUnwrapped())
                                                   : gfx::StencilMode::disabled();
            impl->depthStencilState = context.makeDepthStencilState(depthMode, stencilMode, renderable);
            // FIXME: https://github.com/maplibre/maplibre-native/issues/3248
            if (newStencilMode) impl->previousStencilMode = *newStencilMode;
        }
        renderPass.setDepthStencilState(impl->depthStencilState);
        renderPass.setStencilReference(impl->previousStencilMode.ref);
    }

    for (const auto& seg_ : impl->segments) {
        const auto& segment = static_cast<DrawSegment&>(*seg_);
        const auto& mlSegment = segment.getSegment();
        if (mlSegment.indexLength > 0) {
            const auto& mode = segment.getMode();

            const auto primitiveType = getPrimitiveType(mode.type);
            constexpr auto indexType = MTL::IndexType::IndexTypeUInt16;
            constexpr auto indexSize = sizeof(std::uint16_t);
            const NS::UInteger instanceCount = instanceAttributes ? instanceAttributes->getMaxCount() : 1;
            constexpr NS::UInteger baseInstance = 0;
            const NS::UInteger indexOffset = static_cast<NS::UInteger>(indexSize *
                                                                       mlSegment.indexOffset); // in bytes, not indexes
            const NS::Integer baseVertex = static_cast<NS::Integer>(mlSegment.vertexOffset);

#if !defined(NDEBUG)
            const auto indexBufferLength = indexBuffer->length() / indexSize;
            const auto* indexes = static_cast<const std::uint16_t*>(const_cast<MTL::Buffer*>(indexBuffer)->contents());
            const auto maxIndex = *std::max_element(indexes + mlSegment.indexOffset,
                                                    indexes + mlSegment.indexOffset + mlSegment.indexLength);

            // Uncomment for a detailed accounting of each draw call
            // Log::Warning(Event::General,
            //             util::toString(getID()) + "/" + getName() +
            //             " => " + util::toString(mlSegment.indexLength) +
            //             " idxs @ " + util::toString(mlSegment.indexOffset) +
            //             " (=" + util::toString(mlSegment.indexLength + mlSegment.indexOffset) +
            //             " of " + util::toString(indexBufferLength) +
            //             ") max index " + util::toString(maxIndex) +
            //             " on base vertex " + util::toString(baseVertex) +
            //             " (" + util::toString(baseVertex + maxIndex) +
            //             " of " + util::toString(impl->vertexCount) +
            //             ") indexBuf=" + util::toString((uint64_t)indexBuffer) +
            //             "/" + util::toString(indexBuffer->gpuAddress()));

            assert(mlSegment.indexOffset + mlSegment.indexLength <= indexBufferLength);
            assert(static_cast<std::size_t>(maxIndex) < mlSegment.vertexLength);
#endif

            if (context.getBackend().isBaseVertexInstanceDrawingSupported()) {
                encoder->drawIndexedPrimitives(primitiveType,
                                               mlSegment.indexLength,
                                               indexType,
                                               indexBuffer,
                                               indexOffset,
                                               instanceCount,
                                               baseVertex,
                                               baseInstance);
            } else {
                encoder->drawIndexedPrimitives(
                    primitiveType, mlSegment.indexLength, indexType, indexBuffer, indexOffset, instanceCount);
            }

            context.renderingStats().numDrawCalls++;
        }
    }

    unbindTextures(renderPass);
    unbindAttributes(renderPass);
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

void Drawable::updateVertexAttributes(gfx::VertexAttributeArrayPtr vertices,
                                      std::size_t vertexCount,
                                      gfx::DrawMode mode,
                                      gfx::IndexVectorBasePtr indexes,
                                      const SegmentBase* segments,
                                      std::size_t segmentCount) {
    gfx::Drawable::setVertexAttributes(std::move(vertices));
    impl->vertexCount = vertexCount;

    std::vector<UniqueDrawSegment> drawSegs;
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

const gfx::UniformBufferArray& Drawable::getUniformBuffers() const {
    return impl->uniformBuffers;
}

gfx::UniformBufferArray& Drawable::mutableUniformBuffers() {
    return impl->uniformBuffers;
}

void Drawable::setVertexAttrId(const size_t id) {
    impl->vertexAttrId = id;
}

void Drawable::bindAttributes(RenderPass& renderPass) const noexcept {
    NS::UInteger attributeIndex = 0;
    for (const auto& binding : impl->attributeBindings) {
        const auto* buffer = static_cast<const mtl::VertexBufferResource*>(binding ? binding->vertexBufferResource
                                                                                   : nullptr);
        if (buffer && buffer->get()) {
            assert(binding->vertexStride * impl->vertexCount <= getBufferSize(binding->vertexBufferResource));
            renderPass.bindVertex(buffer->get(), /*offset=*/0, attributeIndex);
        }
        attributeIndex += 1;
    }
}

void Drawable::bindInstanceAttributes(RenderPass& renderPass) const noexcept {
    NS::UInteger attributeIndex = 0;
    for (const auto& binding : impl->instanceBindings) {
        if (binding.has_value()) {
            const auto* buffer = static_cast<const mtl::VertexBufferResource*>(binding->vertexBufferResource);
            if (buffer && buffer->get()) {
                renderPass.bindVertex(buffer->get(), /*offset=*/0, attributeIndex);
            }
        }
        attributeIndex += 1;
    }
}

void Drawable::bindTextures(RenderPass& renderPass) const noexcept {
    for (size_t id = 0; id < textures.size(); id++) {
        if (const auto& texture = textures[id]) {
            if (const auto& location = shader->getSamplerLocation(id)) {
                static_cast<mtl::Texture2D&>(*texture).bind(renderPass, static_cast<int32_t>(*location));
            }
        }
    }
}

void Drawable::unbindTextures(RenderPass& renderPass) const noexcept {
    for (size_t id = 0; id < textures.size(); id++) {
        if (const auto& texture = textures[id]) {
            if (const auto& location = shader->getSamplerLocation(id)) {
                static_cast<mtl::Texture2D&>(*texture).unbind(renderPass, static_cast<int32_t>(*location));
            }
        }
    }
}

void Drawable::uploadTextures(UploadPass&) const noexcept {
    for (const auto& texture : textures) {
        if (texture) {
            texture->upload();
        }
    }
}

namespace {
MTL::VertexFormat mtlVertexTypeOf(gfx::AttributeDataType type) noexcept {
    switch (type) {
        case gfx::AttributeDataType::Byte:
            return MTL::VertexFormatChar;
        case gfx::AttributeDataType::Byte2:
            return MTL::VertexFormatChar2;
        case gfx::AttributeDataType::Byte3:
            return MTL::VertexFormatChar3;
        case gfx::AttributeDataType::Byte4:
            return MTL::VertexFormatChar4;
        case gfx::AttributeDataType::UByte:
            return MTL::VertexFormatUChar;
        case gfx::AttributeDataType::UByte2:
            return MTL::VertexFormatUChar2;
        case gfx::AttributeDataType::UByte3:
            return MTL::VertexFormatUChar3;
        case gfx::AttributeDataType::UByte4:
            return MTL::VertexFormatUChar4;
        case gfx::AttributeDataType::Short:
            return MTL::VertexFormatShort;
        case gfx::AttributeDataType::Short2:
            return MTL::VertexFormatShort2;
        case gfx::AttributeDataType::Short3:
            return MTL::VertexFormatShort3;
        case gfx::AttributeDataType::Short4:
            return MTL::VertexFormatShort4;
        case gfx::AttributeDataType::UShort:
            return MTL::VertexFormatUShort;
        case gfx::AttributeDataType::UShort2:
            return MTL::VertexFormatUShort2;
        case gfx::AttributeDataType::UShort3:
            return MTL::VertexFormatUShort3;
        case gfx::AttributeDataType::UShort4:
            return MTL::VertexFormatUShort4;
        // case gfx::AttributeDataType::UShort8: return MTL::VertexFormatUShort8;
        case gfx::AttributeDataType::Int:
            return MTL::VertexFormatInt;
        case gfx::AttributeDataType::Int2:
            return MTL::VertexFormatInt2;
        case gfx::AttributeDataType::Int3:
            return MTL::VertexFormatInt3;
        case gfx::AttributeDataType::Int4:
            return MTL::VertexFormatInt4;
        case gfx::AttributeDataType::UInt:
            return MTL::VertexFormatUInt;
        case gfx::AttributeDataType::UInt2:
            return MTL::VertexFormatUInt2;
        case gfx::AttributeDataType::UInt3:
            return MTL::VertexFormatUInt3;
        case gfx::AttributeDataType::UInt4:
            return MTL::VertexFormatUInt4;
        case gfx::AttributeDataType::Float:
            return MTL::VertexFormatFloat;
        case gfx::AttributeDataType::Float2:
            return MTL::VertexFormatFloat2;
        case gfx::AttributeDataType::Float3:
            return MTL::VertexFormatFloat3;
        case gfx::AttributeDataType::Float4:
            return MTL::VertexFormatFloat4;
        default:
            assert(!"Unsupported vertex attribute format");
            return MTL::VertexFormatInvalid;
    }
}
} // namespace

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
    auto& contextBase = uploadPass.getContext();
    auto& context = static_cast<Context&>(contextBase);
    constexpr auto usage = gfx::BufferUsageType::StaticDraw;

    // We need either raw index data or a buffer already created from them.
    // We can have a buffer and no indexes, but only if it's not marked dirty.
    if (!impl->indexes || (impl->indexes->empty() && (!impl->indexes->getBuffer() || impl->indexes->getDirty()))) {
        assert(!"Missing index data");
        return;
    }

    if (!impl->indexes->getBuffer() || impl->indexes->getDirty()) {
        // Create or update a buffer for the index data.  We don't update any
        // existing buffer because it may still be in use by the previous frame.
        auto indexBufferResource{uploadPass.createIndexBufferResource(
            impl->indexes->data(), impl->indexes->bytes(), usage, /*persistent=*/false)};
        auto indexBuffer = std::make_unique<gfx::IndexBuffer>(impl->indexes->elements(),
                                                              std::move(indexBufferResource));
        auto buffer = std::make_unique<IndexBuffer>(std::move(indexBuffer));

        impl->indexes->setBuffer(std::move(buffer));
        impl->indexes->setDirty(false);
    }

    const bool buildAttribs = !impl->vertexDesc || !vertexAttributes || !attributeUpdateTime ||
                              vertexAttributes->isModifiedAfter(*attributeUpdateTime);

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

            // hash
            std::size_t hash{0};

            // Create a layout descriptor for each attribute
            auto vertDesc = NS::RetainPtr(MTL::VertexDescriptor::vertexDescriptor());

            NS::UInteger index = 0;
            for (auto& binding : impl->attributeBindings) {
                if (!binding) {
                    assert("Missing attribute binding");
                    index += 1;
                    continue;
                }

                if (!binding->vertexBufferResource && !impl->noBindingBuffer) {
                    if (const auto& buf = context.getEmptyVertexBuffer()) {
                        impl->noBindingBuffer = buf.get();
                    }
                }

                auto attribDesc = NS::TransferPtr(MTL::VertexAttributeDescriptor::alloc()->init());
                attribDesc->setBufferIndex(index);
                attribDesc->setOffset(static_cast<NS::UInteger>(binding->attribute.offset));
                attribDesc->setFormat(mtlVertexTypeOf(binding->attribute.dataType));
                assert(binding->vertexStride > 0);

                auto layoutDesc = NS::TransferPtr(MTL::VertexBufferLayoutDescriptor::alloc()->init());
                layoutDesc->setStride(static_cast<NS::UInteger>(binding->vertexStride));
                layoutDesc->setStepFunction(binding->vertexBufferResource ? MTL::VertexStepFunctionPerVertex
                                                                          : MTL::VertexStepFunctionConstant);
                layoutDesc->setStepRate(binding->vertexBufferResource ? 1 : 0);

                vertDesc->attributes()->setObject(attribDesc.get(), index);
                vertDesc->layouts()->setObject(layoutDesc.get(), index);

                mbgl::util::hash_combine(hash,
                                         mbgl::util::hash(index,
                                                          binding->attribute.offset,
                                                          binding->attribute.dataType,
                                                          binding->vertexStride,
                                                          static_cast<bool>(binding->vertexBufferResource)));

                index += 1;
            }

            impl->vertexDesc = std::move(vertDesc);
            impl->vertexDescHash = hash;
            impl->pipelineState.reset();
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

    const bool texturesNeedUpload = std::any_of(
        textures.begin(), textures.end(), [](const auto& texture) { return texture && texture->needsUpload(); });

    if (texturesNeedUpload) {
        uploadTextures(uploadPass);
    }

    attributeUpdateTime = util::MonotonicTimer::now();
}

} // namespace mtl
} // namespace mbgl
