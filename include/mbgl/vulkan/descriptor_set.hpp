#pragma once

#include <mbgl/gfx/uniform_buffer.hpp>
#include <mbgl/vulkan/buffer_resource.hpp>
#include <span>
#include <queue>

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
        std::queue<std::vector<vk::DescriptorSet>> unusedSets;

        PoolInfo(vk::UniqueDescriptorPool&& pool_, uint32_t remainingSets_)
            : pool(std::move(pool_)),
              remainingSets(remainingSets_) {}
    };

    const uint32_t maxSets{0};
    const uint32_t descriptorStoragePerSet{0};
    const uint32_t descriptorUniformsPerSet{0};
    const uint32_t descriptorTexturesPerSet{0};
    const float growFactor{1.5f};

    std::vector<PoolInfo> pools;
    int32_t currentPoolIndex{-1};

    PoolInfo& current() { return pools[currentPoolIndex]; }

    DescriptorPoolGrowable() = default;
    DescriptorPoolGrowable(uint32_t maxSets_,
                           uint32_t descriptorStoragePerSet_,
                           uint32_t descriptorUniformsPerSet_,
                           uint32_t descriptorTexturesPerSet_,
                           float growFactor_ = 1.5f)
        : maxSets(maxSets_),
          descriptorStoragePerSet(descriptorStoragePerSet_),
          descriptorUniformsPerSet(descriptorUniformsPerSet_),
          descriptorTexturesPerSet(descriptorTexturesPerSet_),
          growFactor(growFactor_) {}
};

class DescriptorSet {
public:
    DescriptorSet(Context& context_, DescriptorSetType type_);
    virtual ~DescriptorSet();

    void allocate();

    virtual void markDirty();
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
    ~UniformDescriptorSet() override = default;

    void update(const gfx::UniformBufferArray& uniforms,
                uint32_t descriptorStartIndex,
                uint32_t descriptorStorageCount,
                uint32_t descriptorUniformCount);
};

class ImageDescriptorSet : public DescriptorSet {
public:
    ImageDescriptorSet(Context& context_);
    ~ImageDescriptorSet() override = default;

    void markDirty() override;
    const std::chrono::duration<double>& getLastModified() const { return lastModified; }

    void update(const std::array<gfx::Texture2DPtr, shaders::maxTextureCountPerShader>& textures);

protected:
    std::chrono::duration<double> lastModified;
};

} // namespace vulkan
} // namespace mbgl
