#if defined(MLN_RENDER_BACKEND_VULKAN)

#include "vulkan_renderer_backend.hpp"

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/vulkan/context.hpp>
#include <mbgl/vulkan/renderable_resource.hpp>

#include <QtGui/QVulkanInstance>
#include <QtGui/QWindow>
#include <QGuiApplication>

#include <cassert>
#include <vulkan/vulkan.h>

namespace QMapLibre {

// ------------- QtVulkanRenderableResource -----------------------------------
QtVulkanRenderableResource::QtVulkanRenderableResource(VulkanRendererBackend& backend_)
    : SurfaceRenderableResource(backend_) {}

std::vector<const char*> QtVulkanRenderableResource::getDeviceExtensions() {
    return {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}

void QtVulkanRenderableResource::createPlatformSurface() {
    auto& qtBackend = static_cast<VulkanRendererBackend&>(backend);

    QVulkanInstance* inst = qtBackend.getQtVulkanInstance();
    if (!inst) {
        throw std::runtime_error(
            "Qt Vulkan instance not available. Make sure QVulkanInstance is initialised before creating the map view.");
    }

    VkSurfaceKHR surfaceKHR = VK_NULL_HANDLE;
    if (!inst->createSurface(qtBackend.getWindow(), &surfaceKHR)) {
        throw std::runtime_error("Failed to create Vulkan surface for Qt window");
    }
    surface = vk::UniqueSurfaceKHR(surfaceKHR,
                                   vk::ObjectDestroy<vk::Instance, vk::DispatchLoaderDynamic>(qtBackend.getInstance()));
}

// ------------- VulkanRendererBackend ---------------------------------------
VulkanRendererBackend::VulkanRendererBackend(QWindow* window_)
    : mbgl::vulkan::RendererBackend(mbgl::gfx::ContextMode::Unique),
      mbgl::vulkan::Renderable({static_cast<uint32_t>(window_->width() * window_->devicePixelRatio()),
                                static_cast<uint32_t>(window_->height() * window_->devicePixelRatio())},
                               std::make_unique<QtVulkanRenderableResource>(*this)),
      window(window_) {
    // Attempt to reuse existing QVulkanInstance or create one.
    vulkanInstance = QVulkanInstance::instance();
    if (!vulkanInstance) {
        // Create and set up a minimal instance so surface creation succeeds.
        vulkanInstance = new QVulkanInstance();
        vulkanInstance->setApiVersion(QVersionNumber(1, 0));
        if (!vulkanInstance->create()) {
            throw std::runtime_error("Failed to create QVulkanInstance");
        }
        // Note: Not calling Qt's setPlatformInstance on window;
        // it's caller's responsibility to ensure correct setup.
    }

    init();
}

VulkanRendererBackend::~VulkanRendererBackend() = default;

void VulkanRendererBackend::setSize(const mbgl::Size newSize) {
    size = newSize;
    if (context) {
        static_cast<mbgl::vulkan::Context&>(*context).requestSurfaceUpdate();
    }
}

std::vector<const char*> VulkanRendererBackend::getInstanceExtensions() {
    auto extensions = mbgl::vulkan::RendererBackend::getInstanceExtensions();

    // QVulkanInstance already enables VK_KHR_surface and platform-specific
    // extensions internally, but make sure to request them for completeness.
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#ifdef __APPLE__
    extensions.push_back("VK_EXT_metal_surface");
#elif defined(Q_OS_WIN)
    extensions.push_back("VK_KHR_win32_surface");
#elif defined(Q_OS_UNIX)
    extensions.push_back("VK_KHR_xcb_surface"); // best effort
#endif
    return extensions;
}

} // namespace QMapLibre

#endif // MLN_RENDER_BACKEND_VULKAN
