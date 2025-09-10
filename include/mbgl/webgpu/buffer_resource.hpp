#pragma once

#include <cstdint>
#include <cstddef>

namespace mbgl {
namespace webgpu {

struct BufferResource {
    void* buffer = nullptr;
    std::size_t size = 0;
    std::uint32_t usage = 0;
    
    BufferResource() = default;
    BufferResource(void* buf, std::size_t sz, std::uint32_t use)
        : buffer(buf), size(sz), usage(use) {}
};

} // namespace webgpu
} // namespace mbgl