#include "glfw_vulkan_backend.hpp"

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/vulkan/renderable_resource.hpp>
#include <mbgl/vulkan/context.hpp>

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class GLFWVulkanRenderableResource final : public mbgl::vulkan::SurfaceRenderableResource {
public:
    explicit GLFWVulkanRenderableResource(GLFWVulkanBackend& backend_)
        : SurfaceRenderableResource(backend_) {}

    std::vector<const char*> getDeviceExtensions() override { return {VK_KHR_SWAPCHAIN_EXTENSION_NAME}; }

    void createPlatformSurface() override {
        auto& glfwBackend = static_cast<GLFWVulkanBackend&>(backend);

        VkSurfaceKHR surface_;

        VkResult result = glfwCreateWindowSurface(
            glfwBackend.getInstance().get(), glfwBackend.getWindow(), nullptr, &surface_);
        if (result != VK_SUCCESS) throw std::runtime_error("Failed to create glfw window surface");

        surface = vk::UniqueSurfaceKHR(
            surface_, vk::ObjectDestroy<vk::Instance, vk::DispatchLoaderDynamic>(glfwBackend.getInstance().get()));
    }

    void bind() override {}
};

GLFWVulkanBackend::GLFWVulkanBackend(GLFWwindow* window_, const bool)
    : mbgl::vulkan::RendererBackend(mbgl::gfx::ContextMode::Unique),
      mbgl::gfx::Renderable(
          [window_] {
              int fbWidth;
              int fbHeight;
              glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);
              return mbgl::Size{static_cast<uint32_t>(fbWidth), static_cast<uint32_t>(fbHeight)};
          }(),
          std::make_unique<GLFWVulkanRenderableResource>(*this)),
      window(window_) {
    init();
}

GLFWVulkanBackend::~GLFWVulkanBackend() {
    context.reset();
}

mbgl::Size GLFWVulkanBackend::getSize() const {
    return size;
}

void GLFWVulkanBackend::setSize(const mbgl::Size newSize) {
    size = newSize;

    auto& contextImpl = static_cast<mbgl::vulkan::Context&>(*context);
    contextImpl.requestSurfaceUpdate();
}

std::vector<const char*> GLFWVulkanBackend::getInstanceExtensions() {
    auto extensions = mbgl::vulkan::RendererBackend::getInstanceExtensions();

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::copy(glfwExtensions, glfwExtensions + glfwExtensionCount, std::back_inserter(extensions));
    return extensions;
}

namespace mbgl {
namespace gfx {

template <>
std::unique_ptr<GLFWBackend> Backend::Create<mbgl::gfx::Backend::Type::Vulkan>(GLFWwindow* window, bool capFrameRate) {
    return std::make_unique<GLFWVulkanBackend>(window, capFrameRate);
}

} // namespace gfx
} // namespace mbgl
