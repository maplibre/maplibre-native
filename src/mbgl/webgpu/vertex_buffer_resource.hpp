#pragma once

#include <mbgl/gfx/vertex_buffer.hpp>
#include <cstddef>

namespace mbgl {
namespace webgpu {

class Context;

class VertexBufferResource : public gfx::VertexBufferResource {
public:
    VertexBufferResource(const void* data, std::size_t size);
    ~VertexBufferResource() override;
    
    void* getBuffer() const { return buffer; }
    std::size_t getSize() const { return size_; }

private:
    void* buffer = nullptr;
    std::size_t size_ = 0;
};

} // namespace webgpu
} // namespace mbgl