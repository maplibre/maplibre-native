#pragma once

#include "glfw_backend.hpp"

#include <mbgl/vulkan/renderable_resource.hpp>
#include <mbgl/vulkan/renderer_backend.hpp>

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

    void activate() override {}
    void deactivate() override {}

private:
    GLFWwindow* window;
};
