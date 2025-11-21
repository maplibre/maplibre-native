#include <mbgl/vulkan/buffer_resource.hpp>

#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/instrumentation.hpp>

#include <algorithm>
#include <numeric>

namespace mbgl {
namespace vulkan {

bool BufferAllocation::create(const VmaAllocationCreateInfo& allocInfo, const vk::BufferCreateInfo& bufferInfo) {
    VkBuffer buffer_;
    VkResult result = vmaCreateBuffer(allocator,
                                      reinterpret_cast<const VkBufferCreateInfo*>(&bufferInfo),
                                      &allocInfo,
                                      &buffer_,
                                      &allocation,
                                      nullptr);

    if (result != VK_SUCCESS) {
        return false;
    }

    buffer = vk::Buffer(buffer_);
    return true;
}

void BufferAllocation::destroy() {
    if (mappedBuffer) vmaUnmapMemory(allocator, allocation);
    vmaDestroyBuffer(allocator, VkBuffer(buffer), allocation);

    buffer = nullptr;
    mappedBuffer = nullptr;
}

void BufferAllocation::setName([[maybe_unused]] const std::string& name) const {
#ifdef ENABLE_VMA_DEBUG
    if (allocation) {
        vmaSetAllocationName(allocator, allocation, name.data());
    }
#endif
}

BufferResource::BufferResource(
    Context& context_, const void* data, std::size_t size_, std::uint32_t usage_, bool persistent_)
    : context(context_),
      size(size_),
      usage(usage_),
      persistent(persistent_) {
    MLN_TRACE_FUNC();

    const auto& allocator = context.getBackend().getAllocator();

    std::size_t totalSize = size;

    // TODO -> check avg minUniformBufferOffsetAlignment vs individual buffers
    if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT || usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
        const auto& backend = context.getBackend();
        const auto& deviceProps = backend.getDeviceProperties();

        vk::DeviceSize align = 0;
        if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {
            align = deviceProps.limits.minUniformBufferOffsetAlignment;
        }

        if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
            align = align ? std::lcm(align, deviceProps.limits.minStorageBufferOffsetAlignment)
                          : deviceProps.limits.minStorageBufferOffsetAlignment;
        }

        bufferWindowSize = (size + align - 1) & ~(align - 1);

        assert(bufferWindowSize != 0);

        const auto frameCount = backend.getMaxFrames();
        totalSize = bufferWindowSize * frameCount;

        bufferWindowVersions = std::vector<VersionType>(frameCount, VersionType{});
    }

    const auto bufferInfo = vk::BufferCreateInfo()
                                .setSize(totalSize)
                                .setUsage(vk::BufferUsageFlags(usage))
                                .setSharingMode(vk::SharingMode::eExclusive);

    VmaAllocationCreateInfo allocationInfo = {};

    allocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    allocationInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    allocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    bufferAllocation = std::make_shared<BufferAllocation>(allocator);
    if (!bufferAllocation->create(allocationInfo, bufferInfo)) {
        mbgl::Log::Error(mbgl::Event::Render, "Vulkan buffer allocation failed");
        return;
    }

    vmaMapMemory(allocator, bufferAllocation->allocation, &bufferAllocation->mappedBuffer);

    if (data) {
        update(data, size, 0);
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
      size(other.size),
      usage(other.usage),
      version(other.version),
      persistent(other.persistent),
      bufferAllocation(std::move(other.bufferAllocation)),
      bufferWindowSize(other.bufferWindowSize),
      bufferWindowVersions(std::move(other.bufferWindowVersions)) {
    other.bufferAllocation = nullptr;
}

BufferResource::~BufferResource() noexcept {
    if (isValid()) {
        context.renderingStats().numBuffers--;

        if (bufferWindowSize > 0) {
            context.renderingStats().memBuffers -= bufferWindowSize * context.getBackend().getMaxFrames();
        } else {
            context.renderingStats().memBuffers -= size;
        }
    }

    if (!bufferAllocation) return;

    context.enqueueDeletion([allocation = std::move(bufferAllocation)](auto&) mutable { allocation.reset(); });
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

    size = other.size;
    usage = other.usage;
    persistent = other.persistent;
    bufferAllocation = std::move(other.bufferAllocation);
    bufferWindowSize = other.bufferWindowSize;
    return *this;
}

void BufferResource::update(const void* newData, std::size_t updateSize, std::size_t offset) noexcept {
    MLN_TRACE_FUNC();

    assert(updateSize + offset <= size);
    updateSize = std::min(updateSize, size - offset);
    if (updateSize <= 0) {
        return;
    }

    uint8_t* data = static_cast<uint8_t*>(bufferAllocation->mappedBuffer) + getVulkanBufferOffset() + offset;

    if (memcmp(data, newData, updateSize) == 0) {
        return;
    }

    std::memcpy(data, newData, updateSize);

    auto& stats = context.renderingStats();
    stats.bufferUpdateBytes += updateSize;
    stats.bufferUpdates++;
    stats.bufferObjUpdates++;
    version++;

    if (version == std::numeric_limits<VersionType>::max()) {
        version = VersionType{} + 1;

        if (bufferWindowSize) {
            std::ranges::fill(bufferWindowVersions, VersionType{});
        }
    }

    if (bufferWindowSize) {
        const auto frameIndex = context.getCurrentFrameResourceIndex();
        bufferWindowVersions[frameIndex] = version;
    }
}

const void* BufferResource::contents() const noexcept {
    return contents(context.getCurrentFrameResourceIndex());
}

const void* BufferResource::contents(uint8_t resourceIndex) const noexcept {
    if (!isValid()) {
        return nullptr;
    }

    return static_cast<uint8_t*>(bufferAllocation->mappedBuffer) + getVulkanBufferOffset(resourceIndex);
}

std::size_t BufferResource::getVulkanBufferOffset() const noexcept {
    return getVulkanBufferOffset(context.getCurrentFrameResourceIndex());
}

std::size_t BufferResource::getVulkanBufferOffset(std::uint8_t resourceIndex) const noexcept {
    assert(context.getBackend().getMaxFrames() >= resourceIndex);
    return bufferWindowSize ? resourceIndex * bufferWindowSize : 0;
}

void BufferResource::updateVulkanBuffer() {
    const auto frameCount = context.getBackend().getMaxFrames();

    const int8_t currentIndex = context.getCurrentFrameResourceIndex();
    const int8_t prevIndex = currentIndex == 0 ? frameCount - 1 : currentIndex - 1;

    updateVulkanBuffer(currentIndex, prevIndex);
}

void BufferResource::updateVulkanBuffer(const int8_t destination, const uint8_t source) {
    if (!bufferWindowSize) {
        return;
    }

    if (bufferWindowVersions[destination] < bufferWindowVersions[source]) {
        uint8_t* dstData = static_cast<uint8_t*>(bufferAllocation->mappedBuffer) + bufferWindowSize * destination;
        uint8_t* srcData = static_cast<uint8_t*>(bufferAllocation->mappedBuffer) + bufferWindowSize * source;

        std::memcpy(dstData, srcData, size);

        bufferWindowVersions[destination] = bufferWindowVersions[source];
    }
}

} // namespace vulkan
} // namespace mbgl
