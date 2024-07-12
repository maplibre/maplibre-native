#include <mbgl/vulkan/buffer_resource.hpp>

#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>
#include <mbgl/util/logging.hpp>

#include <algorithm>

namespace mbgl {
namespace vulkan {

BufferResource::BufferResource(
    Context& context_, const void* data, std::size_t size_, std::uint32_t usage_, bool persistent_)
    : context(context_),
      size(size_),
      usage(usage_),
      persistent(persistent_) {
    const auto& allocator = context.getBackend().getAllocator();

    std::size_t totalSize = size;
    std::size_t offset = 0;

    // TODO -> check avg minUniformBufferOffsetAlignment vs individual buffers
    // if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {
    //    const auto& backend = context.getBackend();
    //    const auto& deviceProps = backend.getDeviceProperties();
    //    const auto& align = deviceProps.limits.minUniformBufferOffsetAlignment;
    //    bufferWindowSize = (size + align - 1) & ~(align - 1);
    //
    //    assert(bufferWindowSize != 0);
    //
    //    offset = bufferWindowSize * context.getCurrentFrameResourceIndex();
    //    totalSize = bufferWindowSize * backend.getMaxFrames();
    //}

    const auto bufferInfo = vk::BufferCreateInfo()
                                .setSize(totalSize)
                                .setUsage(vk::BufferUsageFlags(usage))
                                .setSharingMode(vk::SharingMode::eExclusive);

    VmaAllocationCreateInfo allocationInfo = {};

    allocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    allocationInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    // allocationInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    allocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    bufferAllocation = std::make_shared<BufferAllocation>(allocator);

    VkResult result = vmaCreateBuffer(allocator,
                                      reinterpret_cast<const VkBufferCreateInfo*>(&bufferInfo),
                                      &allocationInfo,
                                      &bufferAllocation->buffer,
                                      &bufferAllocation->allocation,
                                      nullptr);
    if (result != VK_SUCCESS) {
        mbgl::Log::Error(mbgl::Event::Render, "Vulkan buffer allocation failed");
        return;
    }

    VkMemoryPropertyFlags memoryProps;
    vmaGetAllocationMemoryProperties(allocator, bufferAllocation->allocation, &memoryProps);

    if (memoryProps & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        // memory already mapped
        // mapped = allocation->GetMappedData();
        vmaMapMemory(allocator, bufferAllocation->allocation, &bufferAllocation->mappedBuffer);
    } else {
        // TODO create staing buffer for transfer
        // vmaMapMemory(allocator, buffer->allocation, &buffer->mapped);
    }

    if (data) {
        raw.resize(size);
        std::memcpy(raw.data(), data, size);

        std::memcpy(static_cast<uint8_t*>(bufferAllocation->mappedBuffer) + offset, data, size);
        if ((memoryProps & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
            vmaFlushAllocation(allocator, bufferAllocation->allocation, offset, size);
        }
    }

    if (isValid()) {
        auto& stats = context.renderingStats();
        stats.numBuffers++;
        stats.memBuffers += totalSize;
        stats.totalBuffers++;

        stats.totalBufferObjs++;
    }
}

BufferResource::BufferResource(BufferResource&& other) noexcept
    : context(other.context),
      raw(std::move(other.raw)),
      size(other.size),
      usage(other.usage),
      persistent(other.persistent),
      bufferAllocation(std::move(other.bufferAllocation)),
      bufferWindowSize(other.bufferWindowSize) {
    other.bufferAllocation = nullptr;
}

BufferResource::~BufferResource() noexcept {
    if (isValid()) {
        context.renderingStats().numBuffers--;
        context.renderingStats().memBuffers -= size;
    }

    if (!bufferAllocation) return;

    context.enqueueDeletion([allocation = std::move(bufferAllocation)](const auto&) mutable { allocation.reset(); });
}

BufferResource BufferResource::clone() const {
    return {context, contents(), size, usage, persistent};
}

BufferResource& BufferResource::operator=(BufferResource&& other) noexcept {
    assert(&context == &other.context);
    if (isValid()) {
        context.renderingStats().numBuffers--;
        context.renderingStats().memBuffers -= size;
    };
    raw = std::move(other.raw);
    size = other.size;
    usage = other.usage;
    persistent = other.persistent;
    bufferAllocation = std::move(other.bufferAllocation);
    bufferWindowSize = other.bufferWindowSize;
    return *this;
}

void BufferResource::update(const void* newData, std::size_t updateSize, std::size_t offset) noexcept {
    assert(size >= 0 && updateSize + offset <= size);
    updateSize = std::min(updateSize, size - offset);
    if (updateSize <= 0) {
        return;
    }

    auto& stats = context.renderingStats();

    std::memcpy(raw.data() + offset, newData, updateSize);
    std::memcpy(
        static_cast<uint8_t*>(bufferAllocation->mappedBuffer) + getVulkanBufferOffset() + offset, newData, updateSize);
    stats.bufferUpdateBytes += updateSize;

    stats.bufferUpdates++;
    version++;
}

std::size_t BufferResource::getVulkanBufferOffset() const noexcept {
    if (bufferWindowSize > 0) return 0;

    return context.getCurrentFrameResourceIndex() * bufferWindowSize;
}

std::size_t BufferResource::getVulkanBufferSize() const noexcept {
    return bufferWindowSize > 0 ? bufferWindowSize : size;
}

} // namespace vulkan
} // namespace mbgl
