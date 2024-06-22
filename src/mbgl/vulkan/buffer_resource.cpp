#include <mbgl/vulkan/buffer_resource.hpp>

#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>
#include <mbgl/util/logging.hpp>

#include <algorithm>

namespace mbgl {
namespace vulkan {

BufferResource::BufferResource(Context& context_,
                               const void* data,
                               std::size_t size_,
                               std::uint32_t usage_,
                               bool persistent_)
    : context(context_),
      size(size_),
      usage(usage_),
      persistent(persistent_) {
    
    const auto& allocator = context.getBackend().getAllocator();

    VkBufferCreateInfo bufferInfo = {};

    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocationInfo = {};

    allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocationInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    //allocationInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    allocationInfo.flags = 
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | 
        VMA_ALLOCATION_CREATE_MAPPED_BIT;

    bufferAllocation = std::make_unique<BufferAllocation>(allocator);

    VkResult result = vmaCreateBuffer(allocator, &bufferInfo, &allocationInfo, 
        &bufferAllocation->buffer, &bufferAllocation->allocation, nullptr);
    if (result != VK_SUCCESS) {
        mbgl::Log::Error(mbgl::Event::Render, "Vulkan buffer allocation failed");
        return;
    }

    VkMemoryPropertyFlags memoryProps;
    vmaGetAllocationMemoryProperties(allocator, bufferAllocation->allocation, &memoryProps);

    if (memoryProps & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        // memory already mapped
        //mapped = allocation->GetMappedData();
        vmaMapMemory(allocator, bufferAllocation->allocation, &bufferAllocation->mappedBuffer);
    } else {
        // TODO create staing buffer for transfer
        //vmaMapMemory(allocator, buffer->allocation, &buffer->mapped);
    }
    
    if (data) {
        raw.resize(size);
        std::memcpy(raw.data(), data, size);

        std::memcpy(bufferAllocation->mappedBuffer, data, size);
        if ((memoryProps & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
            vmaFlushAllocation(allocator, bufferAllocation->allocation, 0, size);
        }
    }

    if (isValid()) {
        auto& stats = context.renderingStats();
        stats.numBuffers++;
        stats.memBuffers += size;
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
      bufferAllocation(std::move(other.bufferAllocation)) {

    other.bufferAllocation = nullptr;
}

BufferResource::~BufferResource() noexcept {
    if (isValid()) {
        context.renderingStats().numBuffers--;
        context.renderingStats().memBuffers -= size;
    }

    if (!bufferAllocation)
        return;

    auto allocation = std::shared_ptr<BufferAllocation>(std::move(bufferAllocation));
    context.enqueueDeletion(([=]() mutable {
        allocation.reset();
    }));
}

BufferResource BufferResource::clone() const {
    return {context, contents(), size, usage, persistent};
}

BufferResource& BufferResource::operator=(BufferResource&& other) noexcept {
    assert(&context == &other.context);
    if (isValid()) {
        context.renderingStats().numBuffers--;
        context.renderingStats().memBuffers -= size;
    }
    ;
    raw = std::move(other.raw);
    size = other.size;
    usage = other.usage;
    persistent = other.persistent;
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
    std::memcpy(static_cast<char*>(bufferAllocation->mappedBuffer) + offset, newData, updateSize);
    stats.bufferUpdateBytes += updateSize;

    stats.bufferUpdates++;
    version++;
}

} // namespace mtl
} // namespace mbgl
