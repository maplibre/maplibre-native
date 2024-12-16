#pragma once

#include <mbgl/vulkan/renderer_backend.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace mbgl {
namespace vulkan {

class Context;

struct BufferAllocation {
    const VmaAllocator& allocator;
    VmaAllocation allocation{};
    vk::Buffer buffer{};
    void* mappedBuffer{};

    BufferAllocation() = delete;
    BufferAllocation(BufferAllocation&) = delete;
    BufferAllocation& operator=(const BufferAllocation& other) = delete;
    BufferAllocation(BufferAllocation&& other) = default;

    BufferAllocation(const VmaAllocator& allocator_)
        : allocator(allocator_) {}

    ~BufferAllocation() { destroy(); }

    bool create(const VmaAllocationCreateInfo& allocInfo, const vk::BufferCreateInfo& bufferInfo);
    void destroy();

    void setName(const std::string& name) const;
};

using UniqueBufferAllocation = std::unique_ptr<BufferAllocation>;
using SharedBufferAllocation = std::shared_ptr<BufferAllocation>;

class BufferResource {
public:
    BufferResource() noexcept = delete;

    BufferResource(Context& context_, const void* raw, std::size_t size, std::uint32_t usage, bool persistent);
    BufferResource(BufferResource&&) noexcept;
    virtual ~BufferResource() noexcept;

    BufferResource& operator=(BufferResource&&) noexcept;

    BufferResource clone() const;

    void update(const void* data, std::size_t size, std::size_t offset) noexcept;

    std::size_t getSizeInBytes() const noexcept { return size; }
    const void* contents() const noexcept;
    const void* contents(uint8_t resourceIndex) const noexcept;

    Context& getContext() const noexcept { return context; }
    const vk::Buffer& getVulkanBuffer() const noexcept { return bufferAllocation->buffer; }
    std::size_t getVulkanBufferOffset() const noexcept;
    std::size_t getVulkanBufferOffset(uint8_t resourceIndex) const noexcept;
    // update the current sub-buffer with the latest data
    void updateVulkanBuffer();
    void updateVulkanBuffer(const int8_t destination, const uint8_t source);

    bool isValid() const noexcept { return !!bufferAllocation; }
    operator bool() const noexcept { return isValid(); }
    bool operator!() const noexcept { return !isValid(); }

    using VersionType = std::uint16_t;

    /// Used to detect whether buffer contents have changed
    VersionType getVersion() const noexcept { return version; }

protected:
    Context& context;
    std::size_t size;
    std::uint32_t usage;
    VersionType version = 0;
    bool persistent;

    SharedBufferAllocation bufferAllocation;
    size_t bufferWindowSize = 0;
    std::vector<VersionType> bufferWindowVersions;
};

} // namespace vulkan
} // namespace mbgl
