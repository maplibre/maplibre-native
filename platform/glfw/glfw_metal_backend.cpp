#include "glfw_metal_backend.hpp"

GLFWMetalBackend::GLFWMetalBackend(GLFWwindow* window_, const bool capFrameRate)
    : // TODO:
      // rendererBackend(std::make_unique<mbgl::mtl::RendererBackend>(mbgl::gfx::ContextMode::Unique))
      rendererBackend(nullptr) {}

mbgl::gfx::RendererBackend& GLFWMetalBackend::getRendererBackend() {
    return *rendererBackend;
}

void GLFWMetalBackend::setSize(mbgl::Size) {}

mbgl::Size GLFWMetalBackend::getSize() const {
    return mbgl::Size(0, 0);
}