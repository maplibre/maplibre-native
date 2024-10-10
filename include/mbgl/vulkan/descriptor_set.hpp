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

class DescriptorSet {
public:
    DescriptorSet(Context& context_, DescriptorSetType type_);
    virtual ~DescriptorSet();

    void allocate();

    void markDirty(bool value = true);
    void bind(CommandEncoder& encoder);

protected:
    Context& context;
    DescriptorSetType type;

    std::vector<bool> dirty;
    std::vector<vk::DescriptorSet> descriptorSets;
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
