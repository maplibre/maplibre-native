#include <mbgl/vulkan/upload_pass.hpp>

#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/command_encoder.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/vulkan/vertex_attribute.hpp>
#include <mbgl/vulkan/vertex_buffer_resource.hpp>
#include <mbgl/vulkan/index_buffer_resource.hpp>

#include <algorithm>

namespace mbgl {
namespace vulkan {

UploadPass::UploadPass(gfx::Renderable&, CommandEncoder& commandEncoder_, const char* name)
    : commandEncoder(commandEncoder_) {
    // Push the group for the name provided
    debugGroups.emplace_back(gfx::DebugGroup<gfx::UploadPass>{*this, name});
}

UploadPass::~UploadPass() {
    endEncoding();
}

void UploadPass::endEncoding() {
    debugGroups.clear();
}

std::unique_ptr<gfx::VertexBufferResource> UploadPass::createVertexBufferResource(const void* data,
                                                                                  const std::size_t size,
                                                                                  const gfx::BufferUsageType,
                                                                                  bool persistent) {
    return std::make_unique<VertexBufferResource>(
        commandEncoder.context.createBuffer(data, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, persistent));
}

void UploadPass::updateVertexBufferResource(gfx::VertexBufferResource& resource, const void* data, std::size_t size) {
    static_cast<VertexBufferResource&>(resource).get().update(data, size, /*offset=*/0);
}

std::unique_ptr<gfx::IndexBufferResource> UploadPass::createIndexBufferResource(const void* data,
                                                                                const std::size_t size,
                                                                                const gfx::BufferUsageType,
                                                                                bool persistent) {
    return std::make_unique<IndexBufferResource>(
        commandEncoder.context.createBuffer(data, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, persistent));
}

void UploadPass::updateIndexBufferResource(gfx::IndexBufferResource& resource, const void* data, std::size_t size) {
    static_cast<IndexBufferResource&>(resource).get().update(data, size, /*offset=*/0);
}

struct VertexBuffer : public gfx::VertexBufferBase {
    ~VertexBuffer() override = default;

    std::unique_ptr<gfx::VertexBufferResource> resource;
};

static const std::unique_ptr<gfx::VertexBufferResource> noBuffer;

const gfx::UniqueVertexBufferResource& UploadPass::getBuffer(const gfx::VertexVectorBasePtr& vec,
                                                             const gfx::BufferUsageType usage,
                                                             [[maybe_unused]] bool forceUpdate) {
    if (vec) {
        const auto* rawBufPtr = vec->getRawData();
        const auto rawBufSize = vec->getRawCount() * vec->getRawSize();

        // If we already have a buffer...
        if (auto* rawData = static_cast<VertexBuffer*>(vec->getBuffer()); rawData && rawData->resource) {
            auto& resource = static_cast<VertexBufferResource&>(*rawData->resource);

            // If it's changed, update it
            if (rawBufSize <= resource.getSizeInBytes()) {
                if (vec->isModifiedAfter(resource.getLastUpdated())) {
                    // updateVertexBufferResource(resource, rawBufPtr, rawBufSize);
                } else {
                    return rawData->resource;
                }
            }
        }
        // Otherwise, create a new one
        if (rawBufSize > 0) {
            auto buffer = std::make_unique<VertexBuffer>();
            buffer->resource = createVertexBufferResource(rawBufPtr, rawBufSize, usage, /*persistent=*/false);
            vec->setBuffer(std::move(buffer));

            auto* rawData = static_cast<VertexBuffer*>(vec->getBuffer());
            auto& resource = static_cast<VertexBufferResource&>(*rawData->resource);
            resource.setLastUpdated(vec->getLastModified());
            return static_cast<VertexBuffer*>(vec->getBuffer())->resource;
        }
    }
    return noBuffer;
}

gfx::AttributeBindingArray UploadPass::buildAttributeBindings(
    const std::size_t,
    const gfx::AttributeDataType,
    const std::size_t,
    const std::vector<std::uint8_t>&,
    const gfx::VertexAttributeArray& defaults,
    const gfx::VertexAttributeArray& overrides,
    const gfx::BufferUsageType usage,
    const std::optional<std::chrono::duration<double>> lastUpdate,
    /*out*/ std::vector<std::unique_ptr<gfx::VertexBufferResource>>&) {
    gfx::AttributeBindingArray bindings;

    // For each attribute in the program, with the corresponding default and optional override...
    const auto resolveAttr = [&](const size_t, auto& default_, auto& override_) -> void {
        auto& effectiveAttr = override_ ? *override_ : default_;
        const auto& defaultAttr = static_cast<const VertexAttribute&>(default_);
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
        if (const auto& buffer = getBuffer(effectiveAttr.getSharedRawData(), usage, !lastUpdate)) {
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
        if (const auto& buffer = VertexAttribute::getBuffer(
                effectiveAttr, *this, gfx::BufferUsageType::StaticDraw, !lastUpdate)) {
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
    // This version is called when the attribute is available, but isn't being used by the shader
    const auto missingAttr = [&](const size_t, auto& attr_) -> void {
        attr_->setDirty(false);
    };
    defaults.resolve(overrides, resolveAttr, missingAttr);

    return bindings;
}

void UploadPass::pushDebugGroup(const char* name) {
    commandEncoder.pushDebugGroup(name);
}

void UploadPass::popDebugGroup() {
    commandEncoder.popDebugGroup();
}

gfx::Context& UploadPass::getContext() {
    return commandEncoder.context;
}

const gfx::Context& UploadPass::getContext() const {
    return commandEncoder.context;
}

} // namespace vulkan
} // namespace mbgl
