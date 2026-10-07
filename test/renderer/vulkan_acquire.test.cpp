#if MLN_RENDER_BACKEND_VULKAN
#include <mln/test/util.hpp>

#include <mln/gfx/backend_scope.hpp>
#include <mln/util/scoped.hpp>
#include <mln/vulkan/context.hpp>
#include <mln/vulkan/headless_backend.hpp>
#include <mln/vulkan/renderable_resource.hpp>

using namespace mln;

namespace {

constexpr uint64_t stubAcquireTimeout = 16'000'000;

// Keeps the headless device, images, and frame fences, and stands in a surface
// so the context takes the swapchain acquisition path.
class StubSurfaceResource final : public vulkan::SurfaceRenderableResource {
public:
    explicit StubSurfaceResource(vulkan::RendererBackend& backend_)
        : SurfaceRenderableResource(backend_) {
        init(64, 64);
        acquireSemaphores.emplace_back(
            backend.getDevice()->createSemaphoreUnique({}, nullptr, backend.getDispatcher()));
        // Never reaches the driver: acquisition is stubbed and the handle is
        // released before destruction.
        surface = vk::UniqueSurfaceKHR(vk::SurfaceKHR(VkSurfaceKHR(1)), {});
    }
    ~StubSurfaceResource() override { removeSurface(); }

    void createPlatformSurface() override {}
    void bind() override {}
    uint64_t getAcquireTimeout() const override { return stubAcquireTimeout; }

    // Back to the headless path, so frames record and submit without presenting.
    void removeSurface() { surface.release(); }
};

// Counts the device calls made through the backend dispatcher.
struct DispatchProbe {
    VkResult acquireResult = VK_TIMEOUT;
    uint64_t acquireTimeout = 0;
    unsigned acquisitions = 0;
    unsigned recordings = 0;
    unsigned submissions = 0;
    PFN_vkBeginCommandBuffer beginCommandBuffer = nullptr;
    PFN_vkQueueSubmit queueSubmit = nullptr;
};

DispatchProbe* probe = nullptr;

VKAPI_ATTR VkResult VKAPI_CALL
acquireNextImage(VkDevice, VkSwapchainKHR, uint64_t timeout, VkSemaphore, VkFence, uint32_t*) {
    probe->acquireTimeout = timeout;
    ++probe->acquisitions;
    return probe->acquireResult;
}

VKAPI_ATTR VkResult VKAPI_CALL beginCommandBuffer(VkCommandBuffer buffer, const VkCommandBufferBeginInfo* info) {
    ++probe->recordings;
    return probe->beginCommandBuffer(buffer, info);
}

VKAPI_ATTR VkResult VKAPI_CALL queueSubmit(VkQueue queue, uint32_t count, const VkSubmitInfo* info, VkFence fence) {
    ++probe->submissions;
    return probe->queueSubmit(queue, count, info, fence);
}

} // namespace

// An expired acquire must abort the frame before it records anything and leave
// the frame fence signaled, so later frames reuse the same resources.
TEST(VulkanContext, AcquireTimeoutAbortsFrame) {
    vulkan::HeadlessBackend backend({64, 64});
    gfx::BackendScope scope(backend);
    auto& context = backend.getContext<vulkan::Context>();
    auto resource = std::make_unique<StubSurfaceResource>(backend);
    auto& surface = *resource;
    backend.setResource(std::move(resource));

    // The backend exposes its dispatcher read-only; swapping entry points is the
    // only way to stall acquisition without a platform surface.
    auto& dispatcher = const_cast<vulkan::DispatchLoaderDynamic&>(backend.getDispatcher());
    const auto original = dispatcher;
    DispatchProbe state;
    state.beginCommandBuffer = dispatcher.vkBeginCommandBuffer;
    state.queueSubmit = dispatcher.vkQueueSubmit;
    probe = &state;
    dispatcher.vkAcquireNextImageKHR = acquireNextImage;
    dispatcher.vkBeginCommandBuffer = beginCommandBuffer;
    dispatcher.vkQueueSubmit = queueSubmit;
    Scoped restore{[&] {
        dispatcher = original;
        probe = nullptr;
    }};

    for (const auto result : {VK_TIMEOUT, VK_NOT_READY, VK_TIMEOUT}) {
        state.acquireResult = result;
        EXPECT_THROW(context.beginFrame(), vulkan::SurfaceNotReady);
        EXPECT_EQ(state.acquireTimeout, stubAcquireTimeout);
        EXPECT_EQ(state.recordings, 0u);
        EXPECT_TRUE(context.waitFrame());
    }
    EXPECT_EQ(state.acquisitions, 3u);

    // Other acquisition failures still propagate.
    state.acquireResult = VK_ERROR_SURFACE_LOST_KHR;
    EXPECT_THROW(context.beginFrame(), vk::SurfaceLostKHRError);
    EXPECT_EQ(state.recordings, 0u);

    // The same context and fences carry real submissions after the stalls.
    surface.removeSurface();
    for (int frame = 0; frame < 3; ++frame) {
        context.beginFrame();
        context.submitFrame();
        EXPECT_TRUE(context.waitFrame());
    }
    EXPECT_EQ(state.recordings, 3u);
    EXPECT_EQ(state.submissions, 3u);
}

#endif
