#pragma once

#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/gfx/renderbuffer.hpp>
#include <mbgl/gfx/texture.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>

#include <vector>

namespace mbgl {
namespace gfx {

class CommandEncoder;
class VertexVectorBase;
using VertexVectorBasePtr = std::shared_ptr<VertexVectorBase>;

} // namespace gfx

namespace mtl {

class BufferResource;
class CommandEncoder;
class Context;
class VertexArray;
class Texture2D;

class TextureResource : public gfx::TextureResource {
public:
    TextureResource() = default;
};
class RenderbufferResource : public gfx::RenderbufferResource {
public:
    RenderbufferResource() = default;
};

class UploadPass final : public gfx::UploadPass {
public:
    UploadPass(CommandEncoder&, const char* name);

private:
    void pushDebugGroup(const char* name) override;
    void popDebugGroup() override;

public:
#if MLN_DRAWABLE_RENDERER
    gfx::Context& getContext() override;
    const gfx::Context& getContext() const override;
#endif

    std::unique_ptr<gfx::VertexBufferResource> createVertexBufferResource(const void* data,
                                                                          std::size_t size,
                                                                          gfx::BufferUsageType) override;
    void updateVertexBufferResource(gfx::VertexBufferResource&, const void* data, std::size_t size) override;

    std::unique_ptr<gfx::IndexBufferResource> createIndexBufferResource(const void* data,
                                                                        std::size_t size,
                                                                        gfx::BufferUsageType) override;
    void updateIndexBufferResource(gfx::IndexBufferResource&, const void* data, std::size_t size) override;

    void updateResource(BufferResource&, const void* data, std::size_t size);

#if MLN_DRAWABLE_RENDERER
    const gfx::UniqueVertexBufferResource& getBuffer(const gfx::VertexVectorBasePtr&, gfx::BufferUsageType);

    gfx::AttributeBindingArray buildAttributeBindings(
        const std::size_t vertexCount,
        const gfx::AttributeDataType vertexType,
        const std::size_t vertexAttributeIndex,
        const std::vector<std::uint8_t>& vertexData,
        const gfx::VertexAttributeArray& defaults,
        const gfx::VertexAttributeArray& overrides,
        gfx::BufferUsageType,
        /*out*/ std::vector<std::unique_ptr<gfx::VertexBufferResource>>& outBuffers) override;
#endif

public:
    std::unique_ptr<gfx::TextureResource> createTextureResource(Size,
                                                                const void* data,
                                                                gfx::TexturePixelType,
                                                                gfx::TextureChannelDataType) override;
    void updateTextureResource(
        gfx::TextureResource&, Size, const void* data, gfx::TexturePixelType, gfx::TextureChannelDataType) override;

    void updateTextureResourceSub(gfx::TextureResource&,
                                  uint16_t xOffset,
                                  uint16_t yOffset,
                                  Size,
                                  const void* data,
                                  gfx::TexturePixelType,
                                  gfx::TextureChannelDataType) override;

private:
    CommandEncoder& commandEncoder;
    std::vector<gfx::DebugGroup<gfx::CommandEncoder>> debugGroups;
};

} // namespace mtl
} // namespace mbgl
