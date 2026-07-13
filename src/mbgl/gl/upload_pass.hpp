#pragma once

#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/gl/types.hpp>

namespace mbgl {
namespace gfx {

class CommandEncoder;
class VertexVectorBase;
using VertexVectorBasePtr = std::shared_ptr<VertexVectorBase>;

} // namespace gfx

namespace gl {

class CommandEncoder;
class Context;
class VertexArray;
class Texture2D;

class UploadPass final : public gfx::UploadPass {
public:
    UploadPass(gl::CommandEncoder&, const char* name);

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

    const gfx::UniqueVertexBufferResource& getBuffer(const gfx::VertexVectorBasePtr&, gfx::BufferUsageType);

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

private:
    gl::CommandEncoder& commandEncoder;
    const gfx::DebugGroup<gfx::CommandEncoder> debugGroup;
};

} // namespace gl
} // namespace mbgl
