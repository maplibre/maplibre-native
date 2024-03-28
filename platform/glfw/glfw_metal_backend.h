#pragma once

#include "glfw_backend.hpp"
#include "metal_backend.h"

#include <mbgl/gfx/renderable.hpp>
#include <mbgl/mtl/renderer_backend.hpp>

struct GLFWwindow;
@class NSWindow;

class GLFWMetalBackend final : public GLFWBackend {
 public:
  GLFWMetalBackend(GLFWwindow *window_, const bool capFrameRate);
  ~GLFWMetalBackend() = default;
  mbgl::gfx::RendererBackend &getRendererBackend() override;
  void setSize(mbgl::Size) override;
  mbgl::Size getSize() const override;

 private:
  NSWindow *window;
  MetalBackend rendererBackend;
};

namespace mbgl {
namespace gfx {

template <>
std::unique_ptr<GLFWBackend> Backend::Create<mbgl::gfx::Backend::Type::Metal>(GLFWwindow *window,
                                                                              bool capFrameRate) {
  return std::make_unique<GLFWMetalBackend>(window, capFrameRate);
}

}  // namespace gfx
}  // namespace mbgl
