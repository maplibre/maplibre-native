#include "android_vulkan_renderer_backend.hpp"

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/renderable_resource.hpp>

#include <cassert>
#include <vulkan/vulkan_android.h>

namespace mbgl {
namespace android {

class AndroidVulkanRenderableResource final : public mbgl::vulkan::SurfaceRenderableResource {
public:
    AndroidVulkanRenderableResource(AndroidVulkanRendererBackend& backend_)
        : SurfaceRenderableResource(backend_) {}

    std::vector<const char*> getDeviceExtensions() override {
        return {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };
    }

    void createPlatformSurface() override {
        auto& backendImpl = static_cast<AndroidVulkanRendererBackend&>(backend);
        const vk::AndroidSurfaceCreateInfoKHR createInfo({}, backendImpl.getWindow());
        surface = backendImpl.getInstance()->createAndroidSurfaceKHRUnique(
            createInfo, nullptr, backendImpl.getDispatcher());

        const int apiLevel = android_get_device_api_level();
        if (apiLevel < __ANDROID_API_Q__) {
            setSurfaceTransformPollingInterval(30);
        }
    }

    void bind() override {}
    void swap() override {
        vulkan::SurfaceRenderableResource::swap();

        const auto& swapBehaviour = static_cast<AndroidVulkanRendererBackend&>(backend).getSwapBehavior();
        if (swapBehaviour == gfx::Renderable::SwapBehaviour::Flush) {
            static_cast<vulkan::Context&>(backend.getContext()).waitFrame();
        }
    }

private:
};

AndroidVulkanRendererBackend::AndroidVulkanRendererBackend(ANativeWindow* window_)
    : vulkan::RendererBackend(gfx::ContextMode::Unique),
      vulkan::Renderable({64, 64}, std::make_unique<AndroidVulkanRenderableResource>(*this)),
      window(window_) {
    init();
}

AndroidVulkanRendererBackend::~AndroidVulkanRendererBackend() = default;

std::vector<const char*> AndroidVulkanRendererBackend::getInstanceExtensions() {
    auto extensions = mbgl::vulkan::RendererBackend::getInstanceExtensions();
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
    return extensions;
}

void AndroidVulkanRendererBackend::resizeFramebuffer(int width, int height) {
    size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    if (context) {
        static_cast<vulkan::Context&>(*context).requestSurfaceUpdate();
    }
}

PremultipliedImage AndroidVulkanRendererBackend::readFramebuffer() {
    // TODO not implemented
    return PremultipliedImage(Size(2, 2));
}

} // namespace android
} // namespace mbgl

namespace mbgl {
namespace gfx {

template <>
std::unique_ptr<android::AndroidRendererBackend> Backend::Create<mbgl::gfx::Backend::Type::Vulkan>(
    ANativeWindow* window) {
    return std::make_unique<android::AndroidVulkanRendererBackend>(window);
}

} // namespace gfx
} // namespace mbgl
