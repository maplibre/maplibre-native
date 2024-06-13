#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

// TODO merge this with the metal one. Use a common enum for usage flags
namespace mbgl {
namespace vulkan {

class Context;

class BufferResource {
public:
    BufferResource() noexcept = delete;

    BufferResource(Context& context_,
                   const void* raw,
                   std::size_t size,
                   std::uint32_t usage,
                   bool persistent);
    BufferResource(BufferResource&&) noexcept;
    virtual ~BufferResource() noexcept;

    BufferResource& operator=(BufferResource&&) noexcept;

    BufferResource clone() const;

    void update(const void* data, std::size_t size, std::size_t offset) noexcept;

    std::size_t getSizeInBytes() const noexcept { return size; }
    const void* contents() const noexcept { return (raw.empty() ? nullptr : raw.data()); }

    Context& getContext() const noexcept { return context; }

    bool isValid() const noexcept { return !raw.empty(); }
    operator bool() const noexcept { return isValid(); }
    bool operator!() const noexcept { return !isValid(); }

    using VersionType = std::uint16_t;

    /// Used to detect whether buffer contents have changed
    VersionType getVersion() const noexcept { return version; }

protected:
    Context& context;
    std::vector<std::uint8_t> raw;
    std::size_t size;
    std::uint32_t usage;
    std::uint16_t version = 0;
    bool persistent;
};

} // namespace vulkan
} // namespace mbgl
