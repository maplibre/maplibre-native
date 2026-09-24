#include "glfw_vulkan_backend.hpp"

#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/command_encoder.hpp>
#include <mln/gfx/render_pass.hpp>
#include <mln/util/run_loop.hpp>
#include <mln/vulkan/context.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <array>
#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>

int main() try {
    mln::util::RunLoop loop;
    glfwSetErrorCallback([](int, const char* message) { std::cerr << message << '\n'; });
    if (!glfwInit()) throw std::runtime_error("GLFW initialization failed; a desktop session is required");
    struct GLFWCleanup {
        ~GLFWCleanup() { glfwTerminate(); }
    } cleanup;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
    std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> window(
        glfwCreateWindow(320, 240, "MapLibre Vulkan resize test", nullptr, nullptr), glfwDestroyWindow);
    if (!window) throw std::runtime_error("GLFW window creation failed");

    GLFWVulkanBackend backend(window.get(), true);
    mln::gfx::BackendScope scope(backend);
    auto& context = backend.getContext<mln::vulkan::Context>();
    auto& resource = backend.getDefaultRenderable().getResource<mln::vulkan::SurfaceRenderableResource>();
    const auto render = [&] {
        context.beginFrame();
        auto encoder = context.createCommandEncoder();
        auto pass = encoder->createRenderPass(
            "Resize test", {backend.getDefaultRenderable(), mln::Color{0.2f, 0.4f, 0.6f, 1.0f}, 1.0f, 0});
        pass.reset();
        encoder->present(backend.getDefaultRenderable());
        context.endFrame();
    };
    render();

    bool passed = true;
    for (const auto size : std::array<mln::Size, 5>{{{640, 480}, {900, 500}, {400, 700}, {320, 240}, {800, 600}}}) {
        // Drain configure events from the preceding presentation before resizing.
        glfwPollEvents();
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        for (;;) {
            glfwSetWindowSize(window.get(), static_cast<int>(size.width), static_cast<int>(size.height));
            glfwWaitEventsTimeout(0.01);
            int logicalWidth = 0, logicalHeight = 0;
            glfwGetWindowSize(window.get(), &logicalWidth, &logicalHeight);
            if (logicalWidth == static_cast<int>(size.width) && logicalHeight == static_cast<int>(size.height)) break;
            if (std::chrono::steady_clock::now() >= deadline) throw std::runtime_error("Window resize timed out");
        }
        int width = 0, height = 0;
        glfwGetFramebufferSize(window.get(), &width, &height);
        if (width <= 0 || height <= 0) throw std::runtime_error("Window has no drawable framebuffer");
        const mln::Size physicalSize{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        backend.setFramebufferSize(physicalSize);

        // Check both the next frame and after the old deferred update would have run.
        for (int frame = 0; frame < 8; ++frame) {
            render();
            if (frame != 0 && frame != 7) continue;
            const auto extent = resource.getExtent();
            const bool matches = extent.width == physicalSize.width && extent.height == physicalSize.height;
            passed &= matches;
            std::cout << "Framebuffer " << width << 'x' << height << ", frame " << frame + 1 << ": swapchain "
                      << extent.width << 'x' << extent.height << (matches ? " OK\n" : " MISMATCH\n");
        }
    }
    return passed ? 0 : 1;
} catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
}
