#pragma once

#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/gfx/renderbuffer.hpp>
#include <mbgl/gfx/texture.hpp>
#include <mbgl/gfx/upload_pass.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/mtl/mtl_fwd.hpp>

#include <Foundation/NSSharedPtr.hpp>

#include <vector>

namespace mbgl {
namespace gfx {

class CommandEncoder;
class Renderable;
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

    MTL::Texture* getMetalTexture() const { return nullptr; }
};
class RenderbufferResource : public gfx::RenderbufferResource {
public:
    RenderbufferResource() = default;
};

class UploadPass final : public gfx::UploadPass {
public:
    UploadPass(gfx::Renderable&, CommandEncoder&, const char* name);
    ~UploadPass() override;

    gfx::Context& getContext() override;
    const gfx::Context& getContext() const override;

    std::unique_ptr<gfx::VertexBufferResource> createVertexBufferResource(const void* data,
                                                                          std::size_t size,
                                                                          gfx::BufferUsageType) override;
    void updateVertexBufferResource(gfx::VertexBufferResource&, const void* data, std::size_t size) override;

    std::unique_ptr<gfx::IndexBufferResource> createIndexBufferResource(const void* data,
                                                                        std::size_t size,
                                                                        gfx::BufferUsageType) override;
    void updateIndexBufferResource(gfx::IndexBufferResource&, const void* data, std::size_t size) override;

    void updateResource(BufferResource&, const void* data, std::size_t size);

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
    void pushDebugGroup(const char* name) override;
    void popDebugGroup() override;

    void endEncoding();

private:
    CommandEncoder& commandEncoder;
    MTLCommandBufferPtr buffer;
    MTLBlitCommandEncoderPtr encoder;
    std::vector<gfx::DebugGroup<gfx::UploadPass>> debugGroups;
};

} // namespace mtl
} // namespace mbgl
