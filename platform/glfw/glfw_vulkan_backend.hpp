#pragma once

#include "glfw_backend.hpp"

#include <mln/vulkan/renderable_resource.hpp>
#include <mln/vulkan/renderer_backend.hpp>

// Example of using an application side VkInstance/VkDevice
// that's shared with MapLibre's renderer backend
// #define USE_SHARED_VK_CONTEXT

struct GLFWwindow;

class GLFWVulkanBackend final : public GLFWBackend,
                                public mln::vulkan::RendererBackend,
                                public mln::vulkan::Renderable {
public:
    GLFWVulkanBackend(GLFWwindow*, bool capFrameRate);
    ~GLFWVulkanBackend() override;

    GLFWwindow* getWindow() { return window; }

    // GLFWRendererBackend implementation
public:
    mln::gfx::RendererBackend& getRendererBackend() override { return *this; }
    mln::Size getSize() const override;
    void setSize(mln::Size) override;

    // mln::gfx::RendererBackend implementation
public:
    mln::gfx::Renderable& getDefaultRenderable() override { return *this; }

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
