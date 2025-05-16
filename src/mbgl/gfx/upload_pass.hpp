#pragma once

#include <mbgl/gfx/attribute.hpp>
#include <mbgl/gfx/debug_group.hpp>
#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/gfx/index_vector.hpp>
#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/util/size.hpp>
#include <mbgl/util/image.hpp>

#include <mbgl/gfx/texture2d.hpp>

#include <optional>
#include <string>
#include <vector>

namespace mbgl {
namespace gfx {

class Conext;
class Texture2D;
class VertexAttributeArray;

using AttributeBindingArray = std::vector<std::optional<gfx::AttributeBinding>>;
using Texture2DPtr = std::shared_ptr<Texture2D>;

class UploadPass {
protected:
    UploadPass() = default;

    friend class DebugGroup<UploadPass>;
    virtual void pushDebugGroup(const char* name) = 0;
    virtual void popDebugGroup() = 0;

public:
    virtual ~UploadPass() = default;
    UploadPass(const UploadPass&) = delete;
    UploadPass& operator=(const UploadPass&) = delete;

    DebugGroup<UploadPass> createDebugGroup(const char* name) { return {*this, name}; }
    // NOLINTNEXTLINE(bugprone-suspicious-stringview-data-usage)
    DebugGroup<UploadPass> createDebugGroup(std::string_view name) { return createDebugGroup(name.data()); }

    virtual Context& getContext() = 0;
    virtual const Context& getContext() const = 0;

public:
    template <class Vertex>
    VertexBuffer<Vertex> createVertexBuffer(const VertexVector<Vertex>& v,
                                            const BufferUsageType usage = BufferUsageType::StaticDraw,
                                            bool persistent = false) {
        return {v.elements(), createVertexBufferResource(v.data(), v.bytes(), usage, persistent)};
    }

    template <class Vertex>
    void updateVertexBuffer(VertexBuffer<Vertex>& buffer, const VertexVector<Vertex>& v) {
        assert(v.elements() == buffer.elements);
        updateVertexBufferResource(buffer.getResource(), v.data(), v.bytes());
    }

    template <class DrawMode>
    IndexBuffer createIndexBuffer(IndexVector<DrawMode>&& v,
                                  const BufferUsageType usage = BufferUsageType::StaticDraw,
                                  bool persistent = false) {
        return {v.elements(), createIndexBufferResource(v.data(), v.bytes(), usage, persistent)};
    }

    template <class DrawMode>
    void updateIndexBuffer(IndexBuffer& buffer, IndexVector<DrawMode>&& v) {
        assert(v.elements() == buffer.elements);
        updateIndexBufferResource(buffer.getResource(), v.data(), v.bytes());
    }

    virtual gfx::AttributeBindingArray buildAttributeBindings(
        const std::size_t vertexCount,
        const gfx::AttributeDataType vertexType,
        const std::size_t vertexAttributeIndex,
        const std::vector<std::uint8_t>& vertexData,
        const gfx::VertexAttributeArray& defaults,
        const gfx::VertexAttributeArray& overrides,
        gfx::BufferUsageType,
        const std::optional<std::chrono::duration<double>> lastUpdate,
        /*out*/ std::vector<std::unique_ptr<gfx::VertexBufferResource>>& outBuffers) = 0;

protected:
    virtual std::unique_ptr<VertexBufferResource> createVertexBufferResource(const void* data,
                                                                             std::size_t size,
                                                                             BufferUsageType,
                                                                             bool persistent = false) = 0;
    virtual void updateVertexBufferResource(VertexBufferResource&, const void* data, std::size_t size) = 0;

public:
    virtual std::unique_ptr<IndexBufferResource> createIndexBufferResource(const void* data,
                                                                           std::size_t size,
                                                                           BufferUsageType,
                                                                           bool persistent = false) = 0;
    virtual void updateIndexBufferResource(IndexBufferResource&, const void* data, std::size_t size) = 0;
};

} // namespace gfx
} // namespace mbgl
