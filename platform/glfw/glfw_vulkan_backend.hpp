#pragma once

#include "glfw_backend.hpp"

#include <mbgl/vulkan/renderable_resource.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>

// Example of using an application side VkInstance/VkDevice
// that's shared with MapLibre's renderer backend
// #define USE_SHARED_VK_CONTEXT

struct GLFWwindow;

class GLFWVulkanBackend final : public GLFWBackend,
                                public mbgl::vulkan::RendererBackend,
                                public mbgl::vulkan::Renderable {
public:
    GLFWVulkanBackend(GLFWwindow*, bool capFrameRate);
    ~GLFWVulkanBackend() override;

    GLFWwindow* getWindow() { return window; }

    // GLFWRendererBackend implementation
public:
    mbgl::gfx::RendererBackend& getRendererBackend() override { return *this; }
    mbgl::Size getSize() const override;
    void setSize(mbgl::Size) override;

    // mbgl::gfx::RendererBackend implementation
public:
    mbgl::gfx::Renderable& getDefaultRenderable() override { return *this; }

protected:
    std::vector<const char*> getInstanceExtensions() override;

#ifdef USE_SHARED_VK_CONTEXT
    void initInstance() override;
    void initDevice() override;
#endif

    void activate() override {}
    void deactivate() override {}

private:
    GLFWwindow* window;
};
