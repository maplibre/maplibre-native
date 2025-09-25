#pragma once

#include <webgpu/webgpu.h>

#include <cstdint>
#include <cstddef>
#include <vector>

namespace mbgl {
namespace webgpu {

class Context;

class BufferResource {
public:
    BufferResource() noexcept = delete;
    BufferResource(Context& context,
                   const void* data,
                   std::size_t size,
                   uint32_t usage,
                   bool isIndexBuffer = false,
                   bool persistent = false);
    BufferResource(BufferResource&&) noexcept;
    virtual ~BufferResource() noexcept;

    BufferResource& operator=(BufferResource&&) noexcept;

    BufferResource clone() const;

    void update(const void* data, std::size_t size, std::size_t offset) noexcept;

    std::size_t getSizeInBytes() const noexcept { return size; }
    const void* contents() const noexcept { return raw.empty() ? nullptr : raw.data(); }

    Context& getContext() const noexcept { return context; }
    WGPUBuffer getBuffer() const noexcept { return buffer; }

    bool isValid() const noexcept { return buffer || !raw.empty(); }
    operator bool() const noexcept { return isValid(); }
    bool operator!() const noexcept { return !isValid(); }

    using VersionType = std::uint16_t;

    /// Used to detect whether buffer contents have changed
    VersionType getVersion() const noexcept { return version; }

    /// Indicates whether this buffer needs to be re-bound from a previous binding at the given version
    bool needReBind(VersionType version) const noexcept;

    uint32_t getUsage() const { return usage; }
    bool isPersistent() const { return persistent; }

protected:
    Context& context;
    WGPUBuffer buffer = nullptr;
    std::vector<std::uint8_t> raw;
    std::size_t size = 0;
    uint32_t usage = 0;
    std::uint16_t version = 0;
    bool isIndexBuffer = false;
    bool persistent = false;
};

} // namespace webgpu
} // namespace mbgl
