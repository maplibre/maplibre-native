#pragma once

#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>

#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/gfx/renderbuffer.hpp>

namespace mbgl {
namespace gfx {
class CommandEncoder;
class Renderable;
} // namespace gfx

namespace webgpu {

class CommandEncoder;

class RenderbufferResource : public gfx::RenderbufferResource {
public:
    RenderbufferResource() = default;
};

class UploadPass final : public gfx::UploadPass {
public:
    UploadPass(gfx::Renderable& renderable, CommandEncoder& commandEncoder, const char* name);
    ~UploadPass() override;

private:
    void pushDebugGroup(const char* name) override;
    void popDebugGroup() override;

public:
    gfx::Context& getContext() override;
    const gfx::Context& getContext() const override;

    std::unique_ptr<gfx::VertexBufferResource> createVertexBufferResource(const void* data,
                                                                          std::size_t size,
                                                                          gfx::BufferUsageType,
                                                                          bool persistent) override;
    void updateVertexBufferResource(gfx::VertexBufferResource&, const void* data, std::size_t size) override;

    std::unique_ptr<gfx::IndexBufferResource> createIndexBufferResource(const void* data,
                                                                        std::size_t size,
                                                                        gfx::BufferUsageType,
                                                                        bool persistent) override;
    void updateIndexBufferResource(gfx::IndexBufferResource&, const void* data, std::size_t size) override;

    gfx::AttributeBindingArray buildAttributeBindings(
        const std::size_t vertexCount,
        const gfx::AttributeDataType vertexType,
        const std::size_t vertexAttributeIndex,
        const std::vector<std::uint8_t>& vertexData,
        const gfx::VertexAttributeArray& defaults,
        const gfx::VertexAttributeArray& overrides,
        gfx::BufferUsageType,
        const std::optional<std::chrono::duration<double>> lastUpdate,
        /*out*/ std::vector<std::unique_ptr<gfx::VertexBufferResource>>& outBuffers) override;

    const gfx::UniqueVertexBufferResource& getBuffer(const gfx::VertexVectorBasePtr&,
                                                     gfx::BufferUsageType,
                                                     bool forceUpdate);

private:
    CommandEncoder& commandEncoder;
    std::vector<gfx::DebugGroup<gfx::UploadPass>> debugGroups;
};

} // namespace webgpu
} // namespace mbgl
