#pragma once

#include <mbgl/gfx/uniform_buffer.hpp>
#include <mbgl/vulkan/buffer_resource.hpp>
#include <mbgl/vulkan/descriptor_set.hpp>

namespace mbgl {
namespace vulkan {

class UniformBuffer final : public gfx::UniformBuffer {
public:
    UniformBuffer(BufferResource&&);
    UniformBuffer(const UniformBuffer&) = delete;
    UniformBuffer(UniformBuffer&&);
    ~UniformBuffer() override;

    const BufferResource& getBufferResource() const { return buffer; }

    UniformBuffer clone() const { return {buffer.clone()}; }

    void update(const void* data, std::size_t size_) override;

protected:
    BufferResource buffer;
};

/// Stores a collection of uniform buffers by name
class UniformBufferArray final : public gfx::UniformBufferArray {
public:
    UniformBufferArray() = delete;
    UniformBufferArray(DescriptorSetType descriptorSetType_,
                       uint32_t descriptorStartIndex_,
                       uint32_t descriptorBindingCount_)
        : descriptorSetType(descriptorSetType_),
          descriptorStartIndex(descriptorStartIndex_),
          descriptorBindingCount(descriptorBindingCount_) {}

    UniformBufferArray(UniformBufferArray&& other)
        : gfx::UniformBufferArray(std::move(other)) {}
    UniformBufferArray(const UniformBufferArray&) = delete;

    UniformBufferArray& operator=(UniformBufferArray&& other) {
        gfx::UniformBufferArray::operator=(std::move(other));
        return *this;
    }
    UniformBufferArray& operator=(const UniformBufferArray& other) {
        gfx::UniformBufferArray::operator=(other);
        return *this;
    }

    ~UniformBufferArray() = default;

    const std::shared_ptr<gfx::UniformBuffer>& set(const size_t id,
                                                   std::shared_ptr<gfx::UniformBuffer> uniformBuffer, bool bindVertex, bool bindFragment) override;

    void createOrUpdate(
        const size_t id, const void* data, std::size_t size, gfx::Context& context, bool bindVertex, bool bindFragment) override;

    void bindDescriptorSets(CommandEncoder& encoder);
    void freeDescriptorSets() { descriptorSet.reset(); }

private:
    gfx::UniqueUniformBuffer copy(const gfx::UniformBuffer& buffer) override {
        return std::make_unique<UniformBuffer>(static_cast<const UniformBuffer&>(buffer).clone());
    }

    const DescriptorSetType descriptorSetType{DescriptorSetType::DrawableUniform};
    const uint32_t descriptorStartIndex{0};
    const uint32_t descriptorBindingCount{0};

    std::unique_ptr<UniformDescriptorSet> descriptorSet;
};

} // namespace vulkan
} // namespace mbgl
