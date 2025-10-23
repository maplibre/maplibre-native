#include <mbgl/webgpu/upload_pass.hpp>
#include <mbgl/webgpu/command_encoder.hpp>
#include <mbgl/webgpu/context.hpp>
#include <mbgl/webgpu/vertex_buffer_resource.hpp>
#include <mbgl/webgpu/index_buffer_resource.hpp>
#include <mbgl/webgpu/vertex_attribute.hpp>
#include <mbgl/webgpu/renderable_resource.hpp>
#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/gfx/debug_group.hpp>
#include <mbgl/util/logging.hpp>
#include <sstream>

namespace mbgl {
namespace webgpu {

UploadPass::UploadPass([[maybe_unused]] gfx::Renderable& renderable, CommandEncoder& commandEncoder_, const char* name)
    : commandEncoder(commandEncoder_) {
    // Push debug groups (Metal pattern)
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
    debugGroups.clear();
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

std::unique_ptr<gfx::VertexBufferResource> UploadPass::createVertexBufferResource(const void* data,
                                                                                  std::size_t size,
                                                                                  gfx::BufferUsageType /*usage*/,
                                                                                  bool persistent) {
    auto& context = static_cast<Context&>(getContext());
    BufferResource buffer(context, data, size, WGPUBufferUsage_Vertex, /*isIndexBuffer=*/false, persistent);

    if (!buffer.getBuffer()) {
        mbgl::Log::Error(mbgl::Event::Render, "  Failed to create vertex buffer!");
    }

    return std::make_unique<VertexBufferResource>(std::move(buffer));
}

void UploadPass::updateVertexBufferResource(gfx::VertexBufferResource& resource, const void* data, std::size_t size) {
    auto& buffer = static_cast<VertexBufferResource&>(resource);
    buffer.update(data, size);
}

std::unique_ptr<gfx::IndexBufferResource> UploadPass::createIndexBufferResource(const void* data,
                                                                                std::size_t size,
                                                                                gfx::BufferUsageType /*usage*/,
                                                                                bool persistent) {
    auto& context = static_cast<Context&>(getContext());
    BufferResource buffer(context, data, size, WGPUBufferUsage_Index, /*isIndexBuffer=*/true, persistent);
    return std::make_unique<IndexBufferResource>(std::move(buffer));
}

void UploadPass::updateIndexBufferResource(gfx::IndexBufferResource& resource, const void* data, std::size_t size) {
    auto& buffer = static_cast<IndexBufferResource&>(resource);
    buffer.update(data, size);
}

struct VertexBuffer : public gfx::VertexBufferBase {
    ~VertexBuffer() override = default;

    std::unique_ptr<gfx::VertexBufferResource> resource;
};

namespace {
const std::unique_ptr<gfx::VertexBufferResource> noBuffer;
}

const gfx::UniqueVertexBufferResource& UploadPass::getBuffer(const gfx::VertexVectorBasePtr& vec,
                                                             const gfx::BufferUsageType usage,
                                                             bool forceUpdate) {
    if (vec) {
        const auto* rawBufPtr = vec->getRawData();
        const auto rawBufSize = vec->getRawCount() * vec->getRawSize();

        if (auto* rawData = static_cast<VertexBuffer*>(vec->getBuffer()); rawData && rawData->resource) {
            auto& resource = static_cast<VertexBufferResource&>(*rawData->resource);

            if (rawBufSize <= resource.getSizeInBytes()) {
                if (forceUpdate || vec->isModifiedAfter(resource.getLastUpdated())) {
                    updateVertexBufferResource(resource, rawBufPtr, rawBufSize);
                    resource.setLastUpdated(vec->getLastModified());
                }
                return rawData->resource;
            }
        }
        if (rawBufSize > 0) {
            auto buffer_ = std::make_unique<VertexBuffer>();
            buffer_->resource = createVertexBufferResource(rawBufPtr, rawBufSize, usage, /*persistent=*/false);
            vec->setBuffer(std::move(buffer_));
            return static_cast<VertexBuffer*>(vec->getBuffer())->resource;
        }
    }
    return noBuffer;
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
    /*out*/ std::vector<std::unique_ptr<gfx::VertexBufferResource>>& vertexBuffers) {
    gfx::AttributeBindingArray bindings;
    bindings.resize(defaults.allocatedSize());

    const auto resolveAttr = [&]([[maybe_unused]] const size_t id, auto& default_, auto& override_) -> void {
        auto& effectiveAttr = override_ ? *override_ : default_;
        const auto& defaultAttr = static_cast<const VertexAttribute&>(default_);
        const auto index = static_cast<std::size_t>(defaultAttr.getIndex());

        bindings.resize(std::max(bindings.size(), index + 1));

        if (!override_) {
            // No attribute was provided. WebGPU requires all vertex attributes to have buffers bound,
            // so we create a dummy buffer with default values.
            const auto stride = VertexAttribute::getStrideOf(defaultAttr.getDataType());
            const auto bufferSize = stride * vertexCount;

            // Create a buffer with appropriate default values
            std::vector<uint8_t> dummyData(bufferSize);

            // Fill with appropriate default values based on the attribute type
            const auto dataType = defaultAttr.getDataType();
            if (dataType == gfx::AttributeDataType::Float4) {
                // For color attribute - default to white (1,1,1,1)
                float* floatData = reinterpret_cast<float*>(dummyData.data());
                for (size_t i = 0; i < vertexCount; ++i) {
                    floatData[i * 4 + 0] = 1.0f; // R
                    floatData[i * 4 + 1] = 1.0f; // G
                    floatData[i * 4 + 2] = 1.0f; // B
                    floatData[i * 4 + 3] = 1.0f; // A
                }
            } else if (dataType == gfx::AttributeDataType::Float2) {
                // For opacity attribute - default to full opacity (1,1)
                float* floatData = reinterpret_cast<float*>(dummyData.data());
                for (size_t i = 0; i < vertexCount; ++i) {
                    floatData[i * 2 + 0] = 1.0f;
                    floatData[i * 2 + 1] = 1.0f;
                }
            } else {
                // For other types, fill with zeros
                std::fill(dummyData.begin(), dummyData.end(), 0);
            }

            // Create vertex buffer resource for the dummy data
            auto dummyBuffer = createVertexBufferResource(dummyData.data(),
                                                          bufferSize,
                                                          usage,
                                                          /*persistent=*/false);

            bindings[index] = {
                /*.attribute = */ {defaultAttr.getDataType(), /*offset=*/0},
                /*.vertexStride = */ static_cast<uint32_t>(stride),
                /*.vertexBufferResource = */ dummyBuffer.get(),
                /*.vertexOffset = */ 0,
            };

            // Store the buffer so it doesn't get destroyed
            vertexBuffers.push_back(std::move(dummyBuffer));

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

        Log::Error(Event::Render, "Failed to create buffer for attribute");
        assert(false);
    };

    // This version is called when the attribute is available, but isn't being used by the shader
    const auto missingAttr = [&](const size_t, auto& attr) -> void {
        attr->setDirty(false);
    };

    defaults.resolve(overrides, resolveAttr, missingAttr);

    return bindings;
}

} // namespace webgpu
} // namespace mbgl
