#include "glfw_metal_backend.h"

#include "metal_backend.h"

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

GLFWMetalBackend::GLFWMetalBackend(GLFWwindow* window_, [[maybe_unused]] const bool capFrameRate)
    : window(glfwGetCocoaWindow(window_)), rendererBackend(window) {}

mln::gfx::RendererBackend& GLFWMetalBackend::getRendererBackend() { return rendererBackend; }

void GLFWMetalBackend::setSize(mln::Size size) { rendererBackend.setSize(size); }

mln::Size GLFWMetalBackend::getSize() const { return rendererBackend.getSize(); }
