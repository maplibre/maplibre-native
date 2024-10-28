#pragma once

#include <mbgl/gfx/uniform_buffer.hpp>
#include <mbgl/vulkan/buffer_resource.hpp>
#include <span>

namespace mbgl {
namespace vulkan {

class CommandEncoder;

enum class DescriptorSetType : uint8_t {
    Global,
    Layer,
    DrawableUniform,
    DrawableImage,
    Count,
};

struct DescriptorPoolGrowable {
    struct PoolInfo {
        vk::UniqueDescriptorPool pool;
        uint32_t remainingSets{0};

        PoolInfo(vk::UniqueDescriptorPool&& pool_, uint32_t remainingSets_)
            : pool(std::move(pool_)),
              remainingSets(remainingSets_) {}
    };

    const uint32_t maxSets{0};
    const uint32_t descriptorsPerSet{0};
    const float growFactor{1.5f};

    std::vector<PoolInfo> pools;
    int32_t currentPoolIndex{-1};

    PoolInfo& current() { return pools[currentPoolIndex]; }

    DescriptorPoolGrowable() = default;
    DescriptorPoolGrowable(uint32_t maxSets_, uint32_t descriptorsPerSet_, float growFactor_ = 1.5f)
        : maxSets(maxSets_),
          descriptorsPerSet(descriptorsPerSet_),
          growFactor(growFactor_) {}
};

class DescriptorSet {
public:
    DescriptorSet(Context& context_, DescriptorSetType type_);
    virtual ~DescriptorSet();

    void allocate();

    void markDirty(bool value = true);
    void bind(CommandEncoder& encoder);

protected:
    void createDescriptorPool(DescriptorPoolGrowable& growablePool);

protected:
    Context& context;
    DescriptorSetType type;

    std::vector<bool> dirty;
    std::vector<vk::DescriptorSet> descriptorSets;
    int32_t descriptorPoolIndex{-1};
};

class UniformDescriptorSet : public DescriptorSet {
public:
    UniformDescriptorSet(Context& context_, DescriptorSetType type_);
    virtual ~UniformDescriptorSet() = default;

    void update(const gfx::UniformBufferArray& uniforms, uint32_t uniformStartIndex, uint32_t descriptorBindingCount);
};

class ImageDescriptorSet : public DescriptorSet {
public:
    ImageDescriptorSet(Context& context_);
    virtual ~ImageDescriptorSet() = default;

    void update(const std::array<gfx::Texture2DPtr, shaders::maxTextureCountPerShader>& textures);
};

} // namespace vulkan
} // namespace mbgl
