#include "glfw_metal_backend.h"

#include "metal_backend.h"

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

GLFWMetalBackend::GLFWMetalBackend(GLFWwindow* window_, const bool capFrameRate)
    : 
      window(glfwGetCocoaWindow(window_)),
      rendererBackend(window)
      {
      }

mbgl::gfx::RendererBackend& GLFWMetalBackend::getRendererBackend() {
    return rendererBackend;
}

void GLFWMetalBackend::setSize(mbgl::Size) {}

mbgl::Size GLFWMetalBackend::getSize() const {
    return mbgl::Size(0, 0);
}
