#include <mbgl/mtl/upload_pass.hpp>

#include <mbgl/mtl/context.hpp>
#include <mbgl/mtl/command_encoder.hpp>
#include <mbgl/mtl/index_buffer_resource.hpp>
#include <mbgl/mtl/renderable_resource.hpp>
#include <mbgl/mtl/vertex_attribute.hpp>
#include <mbgl/mtl/vertex_buffer_resource.hpp>
#include <mbgl/util/logging.hpp>

#include <Metal/Metal.hpp>

#include <algorithm>

namespace mbgl {
namespace mtl {

UploadPass::UploadPass(gfx::Renderable& renderable, CommandEncoder& commandEncoder_, const char* name)
    : commandEncoder(commandEncoder_) {
    auto& resource = renderable.getResource<RenderableResource>();

    if (!resource.getCommandBuffer()) {
        resource.bind();
    }

    if (const auto& buffer_ = resource.getCommandBuffer()) {
        buffer = buffer_;
        if (auto upd = resource.getUploadPassDescriptor()) {
            encoder = NS::RetainPtr(buffer->blitCommandEncoder(upd.get()));
        }
    }

    assert(encoder);

    // Push the groups already accumulated by the encoder
    commandEncoder.visitDebugGroups([this](const auto& group) {
        debugGroups.emplace_back(gfx::DebugGroup<gfx::UploadPass>{*this, group.c_str()});
    });

    // Push the group for the name provided
    debugGroups.emplace_back(gfx::DebugGroup<gfx::UploadPass>{*this, name});

    // Let the encoder pass along any groups pushed to it after this
    commandEncoder.trackUploadPass(this);
}

UploadPass::~UploadPass() {
    commandEncoder.forgetUploadPass(this);
    endEncoding();
}

void UploadPass::endEncoding() {
    debugGroups.clear();

    if (encoder) {
        encoder->endEncoding();
        encoder.reset();
    }
}

std::unique_ptr<gfx::VertexBufferResource> UploadPass::createVertexBufferResource(const void* data,
                                                                                  const std::size_t size,
                                                                                  const gfx::BufferUsageType usage) {
    return std::make_unique<VertexBufferResource>(commandEncoder.context.createBuffer(data, size, usage));
}

void UploadPass::updateVertexBufferResource(gfx::VertexBufferResource& resource, const void* data, std::size_t size) {
    static_cast<VertexBufferResource&>(resource).get().update(data, size, /*offset=*/0);
}

std::unique_ptr<gfx::IndexBufferResource> UploadPass::createIndexBufferResource(const void* data,
                                                                                const std::size_t size,
                                                                                const gfx::BufferUsageType usage) {
    return std::make_unique<IndexBufferResource>(commandEncoder.context.createBuffer(data, size, usage));
}

void UploadPass::updateIndexBufferResource(gfx::IndexBufferResource& resource, const void* data, std::size_t size) {
    static_cast<IndexBufferResource&>(resource).get().update(data, size, /*offset=*/0);
}

std::unique_ptr<gfx::TextureResource> UploadPass::createTextureResource(const Size size,
                                                                        const void* data,
                                                                        gfx::TexturePixelType format,
                                                                        gfx::TextureChannelDataType type) {
    assert(false);
    throw std::runtime_error("UploadPass::createTextureResource not implemented on Metal!");
}

void UploadPass::updateTextureResource(gfx::TextureResource& resource,
                                       const Size size,
                                       const void* data,
                                       gfx::TexturePixelType format,
                                       gfx::TextureChannelDataType type) {
    assert(false);
    throw std::runtime_error("UploadPass::updateTextureResource not implemented on Metal!");
}

void UploadPass::updateTextureResourceSub(gfx::TextureResource& resource,
                                          const uint16_t xOffset,
                                          const uint16_t yOffset,
                                          const Size size,
                                          const void* data,
                                          gfx::TexturePixelType format,
                                          gfx::TextureChannelDataType type) {
    assert(false);
    throw std::runtime_error("UploadPass::updateTextureResourceSub not implemented on Metal!");
}

struct VertexBuffer : public gfx::VertexBufferBase {
    ~VertexBuffer() override = default;

    std::unique_ptr<gfx::VertexBufferResource> resource;
};

namespace {
const std::unique_ptr<gfx::VertexBufferResource> noBuffer;
}
const gfx::UniqueVertexBufferResource& UploadPass::getBuffer(const gfx::VertexVectorBasePtr& vec,
                                                             const gfx::BufferUsageType usage) {
    if (vec) {
        const auto* rawBufPtr = vec->getRawData();
        const auto rawBufSize = vec->getRawCount() * vec->getRawSize();

        // If we already have a buffer...
        if (auto* rawData = static_cast<VertexBuffer*>(vec->getBuffer()); rawData && rawData->resource) {
            auto& resource = static_cast<VertexBufferResource&>(*rawData->resource);

            // If it's changed, update it
            if (rawBufSize <= resource.getSizeInBytes()) {
                if (vec->getDirty()) {
                    updateVertexBufferResource(resource, rawBufPtr, rawBufSize);
                    vec->setDirty(false);
                }
                return rawData->resource;
            }
        }
        // Otherwise, create a new one
        if (rawBufSize > 0) {
            auto buffer = std::make_unique<VertexBuffer>();
            buffer->resource = createVertexBufferResource(rawBufPtr, rawBufSize, usage);
            vec->setBuffer(std::move(buffer));
            vec->setDirty(false);
            return static_cast<VertexBuffer*>(vec->getBuffer())->resource;
        }
    }
    return noBuffer;
}

gfx::AttributeBindingArray UploadPass::buildAttributeBindings(
    const std::size_t vertexCount,
    const gfx::AttributeDataType vertexType,
    const std::size_t vertexAttributeIndex,
    const std::vector<std::uint8_t>& vertexData,
    const gfx::VertexAttributeArray& defaults,
    const gfx::VertexAttributeArray& overrides,
    const gfx::BufferUsageType usage,
    /*out*/ std::vector<std::unique_ptr<gfx::VertexBufferResource>>& outBuffers) {
    gfx::AttributeBindingArray bindings;
    bindings.resize(defaults.size());

    constexpr std::size_t align = 16;
    constexpr std::uint8_t padding = 0;

    std::vector<std::uint8_t> allData;
    allData.reserve((defaults.getTotalSize() + align) * vertexCount);

    uint32_t vertexStride = 0;

    // For each attribute in the program, with the corresponding default and optional override...
    const auto resolveAttr = [&](const std::string& name, auto& default_, auto& override_) -> void {
        auto& effectiveAttr = override_ ? *override_ : default_;
        const auto& defaultAttr = static_cast<const VertexAttribute&>(default_);
        const auto stride = defaultAttr.getStride();
        const auto offset = static_cast<uint32_t>(allData.size());
        const auto index = static_cast<std::size_t>(defaultAttr.getIndex());

        bindings.resize(std::max(bindings.size(), index + 1));

        if (!override_) {
            // No attribute was provided, this value will be used from a uniform, but we have to set
            // up a valid binding or the validation will complain when the shader code references the
            // attribute at compile time, regardless of whether it's ever used at runtime.
            bindings[index] = {
                /*.attribute = */ {defaultAttr.getDataType(), /*offset=*/0},
                /*.vertexStride = */ static_cast<uint32_t>(VertexAttribute::getStrideOf(defaultAttr.getDataType())),
                /*.vertexBufferResource = */ nullptr,
                /*.vertexOffset = */ 0,
            };
            return;
        }

        // If the attribute references data shared with a bucket, get the corresponding buffer.
        if (const auto& buffer = getBuffer(effectiveAttr.getSharedRawData(), usage)) {
            assert(effectiveAttr.getSharedStride() * effectiveAttr.getSharedVertexOffset() <
                   effectiveAttr.getSharedRawData()->getRawSize() * effectiveAttr.getSharedRawData()->getRawCount());

            bindings[index] = {
                /*.attribute = */ {effectiveAttr.getSharedType(), effectiveAttr.getSharedOffset()},
                /*.vertexStride = */ effectiveAttr.getSharedStride(),
                /*.vertexBufferResource = */ buffer.get(),
                /*.vertexOffset = */ effectiveAttr.getSharedVertexOffset(),
            };
            return;
        }

        assert(effectiveAttr.getStride() > 0);

        // Otherwise, turn the data managed by the attribute into a buffer.
        if (const auto& buffer = VertexAttribute::getBuffer(effectiveAttr, *this, gfx::BufferUsageType::StaticDraw)) {
            bindings[index] = {
                /*.attribute = */ {effectiveAttr.getDataType(), /*offset=*/0},
                /*.vertexStride = */ static_cast<uint32_t>(effectiveAttr.getStride()),
                /*.vertexBufferResource = */ buffer.get(),
                /*.vertexOffset = */ 0,
            };
            return;
        }

        assert(false);
    };
    defaults.resolve(overrides, resolveAttr);

    assert(vertexStride * vertexCount <= allData.size());

    if (!allData.empty()) {
        if (auto vertBuf = createVertexBufferResource(allData.data(), allData.size(), usage)) {
            // Fill in the buffer in each binding that was generated without its own buffer
            std::for_each(bindings.begin(), bindings.end(), [&](auto& b) {
                if (b && !b->vertexBufferResource) {
                    b->vertexBufferResource = vertBuf.get();
                }
            });

            outBuffers.emplace_back(std::move(vertBuf));
        } else {
            assert(false);
            return {};
        }
    }

    return bindings;
}

namespace {
constexpr auto missing = "<none>";
NS::String* toNSString(const char* str) {
    return NS::String::string(str ? str : missing, NS::UTF8StringEncoding);
}
} // namespace

void UploadPass::pushDebugGroup(const char* name) {
    assert(encoder);
    if (encoder) {
        encoder->pushDebugGroup(toNSString(name));
    }
}

void UploadPass::popDebugGroup() {
    assert(encoder);
    if (encoder) {
        encoder->popDebugGroup();
    }
}

gfx::Context& UploadPass::getContext() {
    return commandEncoder.context;
}

const gfx::Context& UploadPass::getContext() const {
    return commandEncoder.context;
}

} // namespace mtl
} // namespace mbgl
