#include <mbgl/mtl/drawable.hpp>

#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/mtl/command_encoder.hpp>
#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/drawable_impl.hpp>
#include <mbgl/mtl/index_buffer_resource.hpp>
#include <mbgl/mtl/render_pass.hpp>
#include <mbgl/mtl/renderable_resource.hpp>
#include <mbgl/mtl/texture2d.hpp>
#include <mbgl/mtl/upload_pass.hpp>
#include <mbgl/mtl/uniform_buffer.hpp>
#include <mbgl/mtl/vertex_buffer_resource.hpp>
#include <mbgl/mtl/vertex_attribute.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/shaders/mtl/shader_program.hpp>
#include <mbgl/util/logging.hpp>

#include <Metal/Metal.hpp>

#include <simd/simd.h>

#include <cassert>

namespace mbgl {
namespace mtl {

Drawable::Drawable(std::string name_)
    : gfx::Drawable(std::move(name_)),
      impl(std::make_unique<Impl>()) {}

Drawable::~Drawable() {}

namespace {
// Downcast a generic vertex buffer resource and extract the Metal buffer
MTL::Buffer* getMetalBuffer(const gfx::VertexBufferResource* resource_) {
    if (const auto* resource = static_cast<const VertexBufferResource*>(resource_)) {
        if (const auto& bufferResource = resource->get()) {
            return bufferResource.getMetalBuffer().get();
        }
    }
    return nullptr;
}

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

MTL::PrimitiveType getPrimitiveType(const gfx::DrawModeType type) {
    switch (type) {
        default:
            assert(false);
            [[fallthrough]];
        case gfx::DrawModeType::Points:
            return MTL::PrimitiveType::PrimitiveTypePoint;
        case gfx::DrawModeType::Lines:
            return MTL::PrimitiveType::PrimitiveTypeLine;
        case gfx::DrawModeType::Triangles:
            return MTL::PrimitiveType::PrimitiveTypeTriangle;
    }
}

#if !defined(NDEBUG)
std::string debugLabel(const gfx::Drawable& drawable) {
    std::string result = drawable.getName();
    if (const auto& tileID = drawable.getTileID()) {
        result.append("/tile=").append(util::toString(*tileID));
    }
    return result;
}
#endif // !defined(NDEBUG)
} // namespace

void Drawable::draw(PaintParameters& parameters) const {
    if (isCustom) {
        return;
    }

    const auto& context = static_cast<Context&>(parameters.context);
    const auto& renderPass = static_cast<RenderPass&>(*parameters.renderPass);
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

    const auto& shaderMTL = static_cast<const ShaderProgram&>(*shader);

#if !defined(NDEBUG)
    const auto debugGroup = parameters.encoder->createDebugGroup(debugLabel(*this));
#endif

    /*
     context.setDepthMode(getIs3D() ? parameters.depthModeFor3D()
                                    : parameters.depthModeForSublayer(getSubLayerIndex(), getDepthType()));

     // force disable depth test for debugging
     // context.setDepthMode({gfx::DepthFunctionType::Always, gfx::DepthMaskType::ReadOnly, {0,1}});

     // For 3D mode, stenciling is handled by the layer group
     if (!is3D) {
         context.setStencilMode(makeStencilMode(parameters));
     }

     context.setColorMode(getColorMode());
     context.setCullFaceMode(getCullFaceMode());

     */

    bindAttributes(renderPass);
    bindUniformBuffers(renderPass);
    bindTextures(renderPass);

    const auto& renderPassDescriptor = renderPass.getDescriptor();

    const auto& indexResource = impl->indexBuffer.getResource<IndexBufferResource>().get();
    const auto& indexBuffer = indexResource.getMetalBuffer().get();
    if (!indexBuffer) {
        assert(!"Index buffer missing");
        return;
    }

    if (!impl->vertexDesc) {
        assert(!"Vertex descriptor missing");
    }

    for (const auto& seg_ : impl->segments) {
        const auto& segment = static_cast<DrawSegment&>(*seg_);
        const auto& mlSegment = segment.getSegment();
        if (mlSegment.indexLength > 0) {
            const auto& mode = segment.getMode();
            if (auto state = shaderMTL.getRenderPipelineState(
                    renderPassDescriptor, impl->vertexDesc, getColorMode())) {
                encoder->setRenderPipelineState(state.get());
            } else {
                assert(!"Failed to create render pipeline state");
                continue;
            }

            const auto primitiveType = getPrimitiveType(mode.type);
            constexpr auto indexType = MTL::IndexType::IndexTypeUInt16;
            constexpr auto indexSize = sizeof(std::uint16_t);
            constexpr NS::UInteger instanceCount = 1;
            constexpr NS::UInteger baseInstance = 0;
            const NS::UInteger indexOffset = static_cast<NS::UInteger>(indexSize *
                                                                       mlSegment.indexOffset); // in bytes, not indexes
            const NS::Integer baseVertex = static_cast<NS::Integer>(mlSegment.vertexOffset);
            encoder->drawIndexedPrimitives(primitiveType,
                                           mlSegment.indexLength,
                                           indexType,
                                           indexBuffer,
                                           indexOffset,
                                           instanceCount,
                                           baseVertex,
                                           baseInstance);
        }
    }

    unbindTextures(renderPass);
    unbindUniformBuffers(renderPass);
    unbindAttributes(renderPass);
}

void Drawable::setIndexData(gfx::IndexVectorBasePtr indexes, std::vector<UniqueDrawSegment> segments) {
    impl->indexes = std::move(indexes);
    impl->segments = std::move(segments);
}

void Drawable::setVertices(std::vector<uint8_t>&& data, std::size_t count, gfx::AttributeDataType type) {
    impl->vertexCount = count;
    impl->vertexType = type;

    if (count && type != gfx::AttributeDataType::Invalid && !data.empty() && !impl->vertexAttrName.empty()) {
        if (auto& attrib = impl->vertexAttributes.getOrAdd(impl->vertexAttrName, /*index=*/-1, type)) {
            attrib->setRawData(std::move(data));
            attrib->setStride(VertexAttribute::getStrideOf(type));
        } else {
            Log::Warning(Event::General, "Vertex attribute type mismatch: " + name + " / " + impl->vertexAttrName);
            assert(false);
        }
    }
}

const gfx::VertexAttributeArray& Drawable::getVertexAttributes() const {
    return impl->vertexAttributes;
}

gfx::VertexAttributeArray& Drawable::mutableVertexAttributes() {
    return impl->vertexAttributes;
}

void Drawable::setVertexAttributes(const gfx::VertexAttributeArray& value) {
    impl->vertexAttributes = static_cast<const VertexAttributeArray&>(value);
}
void Drawable::setVertexAttributes(gfx::VertexAttributeArray&& value) {
    impl->vertexAttributes = std::move(static_cast<VertexAttributeArray&&>(value));
}

const gfx::UniformBufferArray& Drawable::getUniformBuffers() const {
    return impl->uniformBuffers;
}

gfx::UniformBufferArray& Drawable::mutableUniformBuffers() {
    return impl->uniformBuffers;
}

void Drawable::setVertexAttrName(std::string value) {
    impl->vertexAttrName = std::move(value);
}

void Drawable::bindAttributes(const RenderPass& renderPass) const {
    const auto& encoder = renderPass.getMetalEncoder();

    NS::UInteger attributeIndex = 0;
    for (const auto& binding : attributeBindings) {
        if (const auto buffer = getMetalBuffer(binding ? binding->vertexBufferResource : nullptr)) {
            assert(getBufferSize(binding->vertexBufferResource) == binding->vertexStride * impl->vertexCount);
            encoder->setVertexBuffer(buffer, /*offset=*/0, attributeIndex);
        } else if (const auto buffer = getMetalBuffer(impl->noBindingBuffer.get())) {
            encoder->setVertexBuffer(buffer, /*offset=*/0, attributeIndex);
        }
        attributeIndex += 1;
    }
}

void Drawable::unbindAttributes(const RenderPass& renderPass) const {
    const auto& encoder = renderPass.getMetalEncoder();

    NS::UInteger attributeIndex = 0;
    for (const auto& binding : attributeBindings) {
        encoder->setVertexBuffer(nullptr, /*offset=*/0, attributeIndex);
        attributeIndex += 1;
    }
}

void Drawable::bindUniformBuffers(const RenderPass& renderPass) const {
    if (shader) {
        const auto& shaderMTL = static_cast<const ShaderProgram&>(*shader);
        const auto& encoder = renderPass.getMetalEncoder();
        for (const auto& element : shaderMTL.getUniformBlocks().getMap()) {
            const auto& uniformBuffer = getUniformBuffers().get(element.first);
            if (!uniformBuffer) {
                using namespace std::string_literals;
                Log::Error(Event::General,
                           "Drawable::bindUniformBuffers: UBO "s + element.first + " not found. skipping.");
                assert(false);
                continue;
            }

            const auto& buffer = static_cast<UniformBuffer&>(*uniformBuffer.get());
            const auto& bufferMTL = buffer.getBufferResource().getMetalBuffer().get();

            const auto& block = static_cast<const UniformBlock&>(*element.second);
            const auto index = block.getIndex();

            if (block.getBindVertex()) {
                encoder->setVertexBuffer(bufferMTL, /*offset=*/0, index);
            }
            if (block.getBindFragment()) {
                encoder->setFragmentBuffer(bufferMTL, /*offset=*/0, index);
            }
        }
    }
}

void Drawable::unbindUniformBuffers(const RenderPass& renderPass) const {
    if (shader) {
        const auto& shaderMTL = static_cast<const ShaderProgram&>(*shader);
        const auto& encoder = renderPass.getMetalEncoder();
        for (const auto& element : shaderMTL.getUniformBlocks().getMap()) {
            const auto index = element.second->getIndex();
            encoder->setVertexBuffer(nullptr, 0, index);
        }
    }
}

void Drawable::bindTextures(const RenderPass& renderPass) const {
    for (const auto& pair : textures) {
        if (const auto& tex = pair.second) {
            const auto& location = pair.first;
            std::static_pointer_cast<mtl::Texture2D>(tex)->bind(renderPass, location);
        }
    }
}

void Drawable::unbindTextures(const RenderPass& renderPass) const {
    for (const auto& pair : textures) {
        if (const auto& tex = pair.second) {
            const auto& location = pair.first;
            std::static_pointer_cast<mtl::Texture2D>(tex)->unbind(renderPass, location);
        }
    }
}

void Drawable::uploadTextures() const {
    for (const auto& pair : textures) {
        if (const auto& tex = pair.second) {
            std::static_pointer_cast<mtl::Texture2D>(tex)->upload();
        }
    }
}

namespace {
MTL::VertexFormat mtlVertexTypeOf(gfx::AttributeDataType type) {
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
    if (!shader) {
        Log::Warning(Event::General, "Missing shader for drawable " + util::toString(getID()) + "/" + getName());
        assert(false);
        return;
    }
    const auto& shaderMTL = static_cast<const ShaderProgram&>(*shader);
    const auto& shaderUniforms = shaderMTL.getUniformBlocks();

    auto& uploadPass = static_cast<UploadPass&>(uploadPass_);
    auto& contextBase = uploadPass.getContext();
    auto& context = static_cast<Context&>(contextBase);

    const bool buildAttribs = impl->vertexAttributes.isDirty() || !impl->vertexDesc;
    ;

    if (buildAttribs) {
#if !defined(NDEBUG)
        const auto debugGroup = uploadPass.createDebugGroup(debugLabel(*this));
#endif

        constexpr auto usage = gfx::BufferUsageType::StaticDraw;

        const auto indexBytes = impl->indexes->elements() * sizeof(gfx::IndexVectorBase::value_type);
        auto indexBufferResource = uploadPass.createIndexBufferResource(impl->indexes->data(), indexBytes, usage);
        impl->indexBuffer = gfx::IndexBuffer{impl->indexes->elements(), std::move(indexBufferResource)};

        // Apply drawable values to shader defaults
        std::vector<std::unique_ptr<gfx::VertexBufferResource>> vertexBuffers;
        attributeBindings = uploadPass.buildAttributeBindings(impl->vertexCount,
                                                              impl->vertexType,
                                                              /*vertexAttributeIndex=*/-1,
                                                              /*vertexData=*/{},
                                                              shader->getVertexAttributes(),
                                                              impl->vertexAttributes,
                                                              usage,
                                                              vertexBuffers);
        impl->attributeBuffers = std::move(vertexBuffers);

        impl->vertexAttributes.visitAttributes(
            [](const auto&, gfx::VertexAttribute& attrib) { attrib.setDirty(false); });

        // Create a layout descriptor for each attribute
        auto vertDesc = NS::RetainPtr(MTL::VertexDescriptor::vertexDescriptor());

        NS::UInteger index = 0;
        for (auto& binding : attributeBindings) {
            if (!binding) {
                assert("Missing attribute binding");
                continue;
            }

            if (!binding->vertexBufferResource && !impl->noBindingBuffer) {
                impl->noBindingBuffer = uploadPass.createVertexBufferResource(
                    nullptr, 64, gfx::BufferUsageType::StaticDraw);
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

            index += 1;
        }

        impl->vertexDesc = std::move(vertDesc);
    }

    const bool texturesNeedUpload = std::any_of(
        textures.begin(), textures.end(), [](const auto& pair) { return pair.second && pair.second->needsUpload(); });

    if (texturesNeedUpload) {
        uploadTextures();
    }
}
/*
gfx::ColorMode Drawable::makeColorMode(PaintParameters& parameters) const {
    return enableColor ? parameters.colorModeForRenderPass() : gfx::ColorMode::disabled();
}

gfx::StencilMode Drawable::makeStencilMode(PaintParameters& parameters) const {
    if (enableStencil) {
        if (!is3D && tileID) {
            return parameters.stencilModeForClipping(tileID->toUnwrapped());
        }
        assert(false);
    }
    return gfx::StencilMode::disabled();
}
*/
} // namespace mtl
} // namespace mbgl
