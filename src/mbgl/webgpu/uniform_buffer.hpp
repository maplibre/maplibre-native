#pragma once

#include <mbgl/gfx/uniform_buffer.hpp>
#include <cstddef>
#include <vector>

namespace mbgl {
namespace webgpu {

class UniformBuffer : public gfx::UniformBuffer {
public:
    UniformBuffer(const void* data, std::size_t size, bool persistent);
    ~UniformBuffer() override;
    
    void update(const void* data, std::size_t size) override;
    
    void* getBuffer() const { return buffer; }
    std::size_t getSize() const { return size_; }

private:
    void* buffer = nullptr;
    std::size_t size_ = 0;
    std::vector<uint8_t> data_;
};

} // namespace webgpu
} // namespace mbgl