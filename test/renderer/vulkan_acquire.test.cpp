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
        createPlatformSurface();
        surfaceCreations = 0;
    }
    ~StubSurfaceResource() override { removeSurface(); }

    // Never reaches the driver: surface calls are stubbed and the handle is
    // released before destruction.
    void createPlatformSurface() override {
        ++surfaceCreations;
        if (failSurfaceCreation) {
            throw std::runtime_error("stub surface creation failed");
        }
        surface = vk::UniqueSurfaceKHR(vk::SurfaceKHR(VkSurfaceKHR(1)),
                                       {backend.getInstance().get(), nullptr, backend.getDispatcher()});
    }
    void bind() override {}
    uint64_t getAcquireTimeout() const override { return stubAcquireTimeout; }

    // Back to the headless path, so frames record and submit without presenting.
    void removeSurface() { surface.release(); }

    unsigned surfaceCreations = 0;
    bool failSurfaceCreation = false;
};

// Counts the device calls made through the backend dispatcher.
struct DispatchProbe {
    VkResult acquireResult = VK_TIMEOUT;
    VkResult surfaceFormatsResult = VK_ERROR_OUT_OF_HOST_MEMORY;
    uint64_t acquireTimeout = 0;
    unsigned acquisitions = 0;
    unsigned surfaceQueries = 0;
    unsigned surfaceDestructions = 0;
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

// Swapchain creation starts with this query, so failing it fails the rebuild.
VKAPI_ATTR VkResult VKAPI_CALL getSurfaceFormats(VkPhysicalDevice, VkSurfaceKHR, uint32_t*, VkSurfaceFormatKHR*) {
    ++probe->surfaceQueries;
    return probe->surfaceFormatsResult;
}

VKAPI_ATTR void VKAPI_CALL destroySurface(VkInstance, VkSurfaceKHR, const VkAllocationCallbacks*) {
    ++probe->surfaceDestructions;
}

VKAPI_ATTR VkResult VKAPI_CALL beginCommandBuffer(VkCommandBuffer buffer, const VkCommandBufferBeginInfo* info) {
    ++probe->recordings;
    return probe->beginCommandBuffer(buffer, info);
}

VKAPI_ATTR VkResult VKAPI_CALL queueSubmit(VkQueue queue, uint32_t count, const VkSubmitInfo* info, VkFence fence) {
    ++probe->submissions;
    return probe->queueSubmit(queue, count, info, fence);
}

// A headless backend whose default renderable pretends to be a surface, with
// the device calls that touch that surface replaced by the probe.
class StubSurfaceTest {
public:
    StubSurfaceTest()
        : backend({64, 64}),
          scope(backend),
          context(backend.getContext<vulkan::Context>()),
          dispatcher(const_cast<vulkan::DispatchLoaderDynamic&>(backend.getDispatcher())),
          original(dispatcher) {
        // The backend exposes its dispatcher read-only; swapping entry points is the
        // only way to stall acquisition without a platform surface.
        state.beginCommandBuffer = dispatcher.vkBeginCommandBuffer;
        state.queueSubmit = dispatcher.vkQueueSubmit;
        probe = &state;
        dispatcher.vkAcquireNextImageKHR = acquireNextImage;
        dispatcher.vkGetPhysicalDeviceSurfaceFormatsKHR = getSurfaceFormats;
        dispatcher.vkDestroySurfaceKHR = destroySurface;
        dispatcher.vkBeginCommandBuffer = beginCommandBuffer;
        dispatcher.vkQueueSubmit = queueSubmit;

        auto resource = std::make_unique<StubSurfaceResource>(backend);
        surface = resource.get();
        backend.setResource(std::move(resource));
    }

    ~StubSurfaceTest() {
        surface->removeSurface();
        dispatcher = original;
        probe = nullptr;
    }

    vulkan::HeadlessBackend backend;
    gfx::BackendScope scope;
    vulkan::Context& context;
    vulkan::DispatchLoaderDynamic& dispatcher;
    const vulkan::DispatchLoaderDynamic original;
    StubSurfaceResource* surface = nullptr;
    DispatchProbe state;
};

} // namespace

// An expired acquire must abort the frame before it records anything and leave
// the frame fence signaled, so later frames reuse the same resources.
TEST(VulkanContext, AcquireTimeoutAbortsFrame) {
    StubSurfaceTest test;
    auto& context = test.context;
    auto& state = test.state;

    for (const auto result : {VK_TIMEOUT, VK_NOT_READY, VK_TIMEOUT}) {
        state.acquireResult = result;
        EXPECT_THROW(context.beginFrame(), vulkan::SurfaceNotReady);
        EXPECT_EQ(state.acquireTimeout, stubAcquireTimeout);
        EXPECT_EQ(state.recordings, 0u);
        EXPECT_TRUE(context.waitFrame());
    }
    EXPECT_EQ(state.acquisitions, 3u);

    // Other acquisition failures still propagate.
    state.acquireResult = VK_ERROR_DEVICE_LOST;
    EXPECT_THROW(context.beginFrame(), vk::DeviceLostError);
    EXPECT_EQ(state.recordings, 0u);

    // The same context and fences carry real submissions after the stalls.
    test.surface->removeSurface();
    for (int frame = 0; frame < 3; ++frame) {
        context.beginFrame();
        context.submitFrame();
        EXPECT_TRUE(context.waitFrame());
    }
    EXPECT_EQ(state.recordings, 3u);
    EXPECT_EQ(state.submissions, 3u);
}

// An out of date swapchain is rebuilt inside the frame. If the rebuild fails,
// the old swapchain is already gone, so the next frame must retry the rebuild
// instead of acquiring from the destroyed swapchain.
TEST(VulkanContext, FailedSwapchainRebuildIsRetried) {
    StubSurfaceTest test;
    auto& context = test.context;
    auto& state = test.state;

    state.acquireResult = VK_ERROR_OUT_OF_DATE_KHR;
    EXPECT_THROW(context.beginFrame(), vk::OutOfHostMemoryError);
    EXPECT_EQ(state.acquisitions, 1u);
    EXPECT_EQ(state.surfaceQueries, 1u);

    EXPECT_THROW(context.beginFrame(), vk::OutOfHostMemoryError);
    EXPECT_EQ(state.surfaceQueries, 2u);
    EXPECT_EQ(state.acquisitions, 1u);
    EXPECT_EQ(state.recordings, 0u);
}

// A lost surface is recreated before the swapchain is rebuilt, and both are
// retried on the next frame when either step fails.
TEST(VulkanContext, FailedSurfaceRecreationIsRetried) {
    StubSurfaceTest test;
    auto& context = test.context;
    auto& state = test.state;
    auto& surface = *test.surface;

    state.acquireResult = VK_ERROR_SURFACE_LOST_KHR;
    EXPECT_THROW(context.beginFrame(), vk::OutOfHostMemoryError);
    EXPECT_EQ(state.acquisitions, 1u);
    EXPECT_EQ(state.surfaceDestructions, 1u);
    EXPECT_EQ(surface.surfaceCreations, 1u);
    EXPECT_EQ(state.surfaceQueries, 1u);

    // Rebuilding the swapchain failed: the next frame recreates the surface again.
    EXPECT_THROW(context.beginFrame(), vk::OutOfHostMemoryError);
    EXPECT_EQ(state.surfaceDestructions, 2u);
    EXPECT_EQ(surface.surfaceCreations, 2u);
    EXPECT_EQ(state.surfaceQueries, 2u);
    EXPECT_EQ(state.acquisitions, 1u);

    // Creating the surface fails, which leaves the resource without one. That must not
    // send the next frame down the headless path, which has no images to draw into.
    surface.failSurfaceCreation = true;
    EXPECT_THROW(context.beginFrame(), std::runtime_error);
    EXPECT_FALSE(surface.getPlatformSurface());
    EXPECT_EQ(surface.surfaceCreations, 3u);

    EXPECT_THROW(context.beginFrame(), std::runtime_error);
    EXPECT_EQ(surface.surfaceCreations, 4u);
    EXPECT_EQ(state.acquisitions, 1u);
    EXPECT_EQ(state.recordings, 0u);
}

#endif
