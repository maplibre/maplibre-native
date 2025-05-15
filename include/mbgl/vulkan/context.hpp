#pragma once

#include <mbgl/gfx/texture.hpp>
#include <mbgl/gfx/draw_mode.hpp>
#include <mbgl/gfx/depth_mode.hpp>
#include <mbgl/gfx/stencil_mode.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/texture2d.hpp>
#include <mbgl/gfx/context.hpp>
#include <mbgl/util/noncopyable.hpp>
#include <mbgl/util/containers.hpp>
#include <mbgl/vulkan/uniform_buffer.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>
#include <mbgl/vulkan/pipeline.hpp>
#include <mbgl/vulkan/descriptor_set.hpp>

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace mbgl {

class ProgramParameters;
class RenderStaticData;

namespace gfx {
class VertexAttributeArray;
using VertexAttributeArrayPtr = std::shared_ptr<VertexAttributeArray>;
} // namespace gfx

namespace shaders {
struct ClipUBO;
} // namespace shaders

namespace vulkan {

class RenderPass;
class RendererBackend;
class ShaderProgram;
class VertexBufferResource;
class Texture2D;

using UniqueShaderProgram = std::unique_ptr<ShaderProgram>;
using UniqueVertexBufferResource = std::unique_ptr<VertexBufferResource>;
using UniqueUniformBufferArray = std::unique_ptr<gfx::UniformBufferArray>;

class Context final : public gfx::Context {
public:
    Context(RendererBackend&);
    ~Context() noexcept override;
    Context(const Context&) = delete;
    Context& operator=(const Context& other) = delete;

    RendererBackend& getBackend() const { return backend; }

    void beginFrame() override;
    void endFrame() override;
    void submitFrame();
    void waitFrame() const;

    std::unique_ptr<gfx::CommandEncoder> createCommandEncoder() override;

    BufferResource createBuffer(const void* data, std::size_t size, std::uint32_t usage, bool persistent) const;

    UniqueShaderProgram createProgram(shaders::BuiltIn shaderID,
                                      std::string name,
                                      const std::string_view vertex,
                                      const std::string_view fragment,
                                      const ProgramParameters& programParameters,
                                      const mbgl::unordered_map<std::string, std::string>& additionalDefines);

    /// Called at the end of a frame.
    void performCleanup() override {}
    void reduceMemoryUsage() override {}

    gfx::UniqueDrawableBuilder createDrawableBuilder(std::string name) override;
    gfx::UniformBufferPtr createUniformBuffer(const void* data,
                                              std::size_t size,
                                              bool persistent,
                                              bool ssbo = false) override;

    UniqueUniformBufferArray createLayerUniformBufferArray() override;

    gfx::ShaderProgramBasePtr getGenericShader(gfx::ShaderRegistry&, const std::string& name) override;

    TileLayerGroupPtr createTileLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) override;

    LayerGroupPtr createLayerGroup(int32_t layerIndex, std::size_t initialCapacity, std::string name) override;

    gfx::Texture2DPtr createTexture2D() override;

    RenderTargetPtr createRenderTarget(const Size size, const gfx::TextureChannelDataType type) override;

    void resetState(gfx::DepthMode, gfx::ColorMode) override {}

    bool emplaceOrUpdateUniformBuffer(gfx::UniformBufferPtr&,
                                      const void* data,
                                      std::size_t size,
                                      bool persistent = false) override;

    void setDirtyState() override {}

    std::unique_ptr<gfx::OffscreenTexture> createOffscreenTexture(Size, gfx::TextureChannelDataType, bool, bool);

    std::unique_ptr<gfx::OffscreenTexture> createOffscreenTexture(Size, gfx::TextureChannelDataType) override;

    std::unique_ptr<gfx::TextureResource> createTextureResource(Size,
                                                                gfx::TexturePixelType,
                                                                gfx::TextureChannelDataType) override;

    std::unique_ptr<gfx::RenderbufferResource> createRenderbufferResource(gfx::RenderbufferPixelType,
                                                                          Size size) override;

    std::unique_ptr<gfx::DrawScopeResource> createDrawScopeResource() override;

    gfx::VertexAttributeArrayPtr createVertexAttributeArray() const override;

#if !defined(NDEBUG)
    void visualizeStencilBuffer() override;
    void visualizeDepthBuffer(float depthRangeSize) override;
#endif

    void clearStencilBuffer(int32_t) override;

    /// Get the global uniform buffers
    const gfx::UniformBufferArray& getGlobalUniformBuffers() const override { return globalUniformBuffers; };

    /// Get the mutable global uniform buffer array
    gfx::UniformBufferArray& mutableGlobalUniformBuffers() override { return globalUniformBuffers; };

    /// Bind the global uniform buffers
    void bindGlobalUniformBuffers(gfx::RenderPass&) const noexcept override;

    /// Unbind the global uniform buffers
    void unbindGlobalUniformBuffers(gfx::RenderPass&) const noexcept override {}

    bool renderTileClippingMasks(gfx::RenderPass& renderPass,
                                 RenderStaticData& staticData,
                                 const std::vector<shaders::ClipUBO>& tileUBOs);

    const std::unique_ptr<BufferResource>& getDummyBuffer();
    const std::unique_ptr<Texture2D>& getDummyTexture();

    const vk::DescriptorSetLayout& getDescriptorSetLayout(DescriptorSetType type);
    DescriptorPoolGrowable& getDescriptorPool(DescriptorSetType type);
    const vk::UniquePipelineLayout& getGeneralPipelineLayout();
    const vk::UniquePipelineLayout& getPushConstantPipelineLayout();

    uint8_t getCurrentFrameResourceIndex() const { return frameResourceIndex; }
    void enqueueDeletion(std::function<void(Context&)>&& function);
    void submitOneTimeCommand(const std::function<void(const vk::UniqueCommandBuffer&)>& function) const;

    void requestSurfaceUpdate(bool useDelay = true);

private:
    struct FrameResources {
        vk::UniqueCommandBuffer commandBuffer;

        vk::UniqueSemaphore acquireSurfaceSemaphore;
        vk::UniqueFence flightFrameFence;

        std::vector<std::function<void(Context&)>> deletionQueue;

        FrameResources(vk::UniqueCommandBuffer& cb, vk::UniqueSemaphore&& surf, vk::UniqueFence&& flight)
            : commandBuffer(std::move(cb)),
              acquireSurfaceSemaphore(std::move(surf)),
              flightFrameFence(std::move(flight)) {}

        void runDeletionQueue(Context&);
    };

    void initFrameResources();
    void destroyResources();

    void buildImageDescriptorSetLayout();
    void buildUniformDescriptorSetLayout(vk::UniqueDescriptorSetLayout& layout,
                                         size_t startId,
                                         size_t storageCount,
                                         size_t uniformCount,
                                         const std::string& name);

private:
    RendererBackend& backend;

    vulkan::UniformBufferArray globalUniformBuffers;
    std::unordered_map<DescriptorSetType, DescriptorPoolGrowable> descriptorPoolMap;

    std::unique_ptr<BufferResource> dummyBuffer;
    std::unique_ptr<Texture2D> dummyTexture2D;
    vk::UniqueDescriptorSetLayout globalUniformDescriptorSetLayout;
    vk::UniqueDescriptorSetLayout layerUniformDescriptorSetLayout;
    vk::UniqueDescriptorSetLayout drawableUniformDescriptorSetLayout;
    vk::UniqueDescriptorSetLayout drawableImageDescriptorSetLayout;
    vk::UniquePipelineLayout generalPipelineLayout;
    vk::UniquePipelineLayout pushConstantPipelineLayout;

    uint8_t frameResourceIndex = 0;
    std::vector<FrameResources> frameResources;
    bool surfaceUpdateRequested{false};
    int32_t surfaceUpdateLatency{0};
    int32_t currentFrameCount{0};

    struct {
        gfx::ShaderProgramBasePtr shader;
        std::optional<BufferResource> vertexBuffer;
        std::optional<BufferResource> indexBuffer;
        uint32_t indexCount = 0;

        PipelineInfo pipelineInfo;
    } clipping;
};

} // namespace vulkan
} // namespace mbgl
