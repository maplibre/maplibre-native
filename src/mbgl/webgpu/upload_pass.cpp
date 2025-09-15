#include <mbgl/webgpu/upload_pass.hpp>
#include <mbgl/webgpu/command_encoder.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/buffer_resource.hpp>
#include <mbgl/webgpu/vertex_buffer_resource.hpp>
#include <mbgl/webgpu/index_buffer_resource.hpp>
#include <mbgl/webgpu/vertex_attribute.hpp>
#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace webgpu {

UploadPass::UploadPass(CommandEncoder& commandEncoder_, const char* name)
    : commandEncoder(commandEncoder_) {
    pushDebugGroup(name);
}

void UploadPass::pushDebugGroup(const char* /*name*/) {
    // WebGPU debug groups are typically set on command encoders
    // This would require access to the WebGPU command encoder
    // For now, this is a no-op
}

void UploadPass::popDebugGroup() {
    // WebGPU debug groups are typically set on command encoders
    // This would require access to the WebGPU command encoder
    // For now, this is a no-op
}

gfx::Context& UploadPass::getContext() {
    return commandEncoder.getContext();
}

const gfx::Context& UploadPass::getContext() const {
    return commandEncoder.getContext();
}

std::unique_ptr<gfx::VertexBufferResource> UploadPass::createVertexBufferResource(
    const void* data,
    std::size_t size,
    gfx::BufferUsageType /*usage*/,
    bool persistent) {

    auto& context = static_cast<Context&>(getContext());
    BufferResource buffer(context, data, size, WGPUBufferUsage_Vertex, /*isIndexBuffer=*/false, persistent);
    return std::make_unique<VertexBufferResource>(std::move(buffer));
}

void UploadPass::updateVertexBufferResource(gfx::VertexBufferResource& resource, 
                                           const void* data, 
                                           std::size_t size) {
    auto& buffer = static_cast<VertexBufferResource&>(resource);
    buffer.update(data, size);
}

std::unique_ptr<gfx::IndexBufferResource> UploadPass::createIndexBufferResource(
    const void* data,
    std::size_t size,
    gfx::BufferUsageType /*usage*/,
    bool persistent) {

    auto& context = static_cast<Context&>(getContext());
    BufferResource buffer(context, data, size, WGPUBufferUsage_Index, /*isIndexBuffer=*/true, persistent);
    return std::make_unique<IndexBufferResource>(std::move(buffer));
}

void UploadPass::updateIndexBufferResource(gfx::IndexBufferResource& resource, 
                                          const void* data, 
                                          std::size_t size) {
    auto& buffer = static_cast<IndexBufferResource&>(resource);
    buffer.update(data, size);
}

const gfx::UniqueVertexBufferResource& UploadPass::getBuffer(const gfx::VertexVectorBasePtr& vertexVector,
                                                             gfx::BufferUsageType usage,
                                                             bool forceUpdate) {
    static gfx::UniqueVertexBufferResource nullBuffer;

    if (!vertexVector) {
        return nullBuffer;
    }

    // Check if buffer already exists and doesn't need update
    auto& buffer = vertexVector->getBuffer();
    if (buffer && !forceUpdate) {
        return buffer;
    }

    // Create or update the buffer
    const auto data = vertexVector->getRawData();
    const auto size = vertexVector->getRawSize() * vertexVector->getRawCount();

    if (size > 0) {
        if (buffer && forceUpdate) {
            updateVertexBufferResource(*buffer, data, size);
        } else {
            auto newBuffer = createVertexBufferResource(data, size, usage, false);
            vertexVector->setBuffer(std::move(newBuffer));
        }
    }

    return vertexVector->getBuffer();
}

gfx::AttributeBindingArray UploadPass::buildAttributeBindings(
    [[maybe_unused]] const std::size_t vertexCount,
    [[maybe_unused]] const gfx::AttributeDataType vertexType,
    [[maybe_unused]] const std::size_t vertexAttributeIndex,
    [[maybe_unused]] const std::vector<std::uint8_t>& vertexData,
    const gfx::VertexAttributeArray& defaults,
    const gfx::VertexAttributeArray& overrides,
    const gfx::BufferUsageType usage,
    const std::optional<std::chrono::duration<double>> lastUpdate,
    /*out*/ std::vector<std::unique_ptr<gfx::VertexBufferResource>>&) {

    Log::Info(Event::Render, "WebGPU buildAttributeBindings called");
    Log::Info(Event::Render, "  defaults.allocatedSize() = %zu", defaults.allocatedSize());

    gfx::AttributeBindingArray bindings;
    bindings.resize(defaults.allocatedSize());

    // For each attribute in the program, with the corresponding default and optional override...
    const auto resolveAttr = [&]([[maybe_unused]] const size_t id, auto& default_, auto& override_) -> void {
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
        if (const auto& buffer_ = getBuffer(effectiveAttr.getSharedRawData(), usage, !lastUpdate)) {
            assert(effectiveAttr.getSharedStride() * effectiveAttr.getSharedVertexOffset() <
                   effectiveAttr.getSharedRawData()->getRawSize() * effectiveAttr.getSharedRawData()->getRawCount());

            bindings[index] = {
                /*.attribute = */ {effectiveAttr.getSharedType(), effectiveAttr.getSharedOffset()},
                /*.vertexStride = */ effectiveAttr.getSharedStride(),
                /*.vertexBufferResource = */ buffer_.get(),
                /*.vertexOffset = */ effectiveAttr.getSharedVertexOffset(),
            };
            return;
        }

        assert(effectiveAttr.getStride() > 0);

        // Otherwise, turn the data managed by the attribute into a buffer.
        if (const auto& buffer_ = VertexAttribute::getBuffer(
                effectiveAttr, *this, gfx::BufferUsageType::StaticDraw, !lastUpdate)) {
            bindings[index] = {
                /*.attribute = */ {effectiveAttr.getDataType(), /*offset=*/0},
                /*.vertexStride = */ static_cast<uint32_t>(effectiveAttr.getStride()),
                /*.vertexBufferResource = */ buffer_.get(),
                /*.vertexOffset = */ 0,
            };
            return;
        }

        assert(false);
    };

    // This version is called when the attribute is available, but isn't being used by the shader
    const auto missingAttr = [&](const size_t, auto& missingAttr) -> void {
        missingAttr->setDirty(false);
    };

    defaults.resolve(overrides, resolveAttr, missingAttr);

    return bindings;
}

} // namespace webgpu
} // namespace mbgl