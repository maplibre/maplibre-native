#pragma once

#include <mbgl/gfx/uniform_buffer.hpp>
#include <mbgl/webgpu/backend_impl.hpp>
#include <cstddef>
#include <vector>

namespace mbgl {
namespace webgpu {

class Context;

class UniformBuffer : public gfx::UniformBuffer {
public:
    UniformBuffer(Context& context, const void* data, std::size_t size, bool persistent);
    ~UniformBuffer() override;

    // Update buffer data
    void update(const void* data, std::size_t size) override;
    
    // WebGPU specific
    WGPUBuffer getBuffer() const { return buffer; }
    std::size_t getSize() const { return size; }
    bool isPersistent() const { return persistent; }
    
    // For creating bind group entries
    WGPUBindGroupEntry createBindGroupEntry(uint32_t binding) const;

private:
    void createBuffer(const void* data);
    
    Context& context;
    WGPUBuffer buffer = nullptr;
    std::size_t size;
    bool persistent;
    
    // For dynamic updates
    std::vector<uint8_t> stagingData;
};

// SSBO (Storage Buffer) variant
class StorageBuffer : public UniformBuffer {
public:
    StorageBuffer(Context& context, const void* data, std::size_t size, bool persistent);
    
    // Override to use storage buffer usage flags
    WGPUBindGroupEntry createBindGroupEntry(uint32_t binding) const;
};

} // namespace webgpu
} // namespace mbgl