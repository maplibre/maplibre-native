#pragma once

#include "glfw_backend.hpp"
#include "metal_backend.h"

#include <mln/gfx/renderable.hpp>
#include <mln/mtl/renderer_backend.hpp>

struct GLFWwindow;
@class NSWindow;

class GLFWMetalBackend final : public GLFWBackend {
public:
  GLFWMetalBackend(GLFWwindow *window_, const bool capFrameRate);
  ~GLFWMetalBackend() = default;
  mln::gfx::RendererBackend &getRendererBackend() override;
  void setSize(mln::Size) override;
  mln::Size getSize() const override;

private:
  NSWindow *window;
  MetalBackend rendererBackend;
};

namespace mln {
namespace gfx {

template <>
std::unique_ptr<GLFWBackend> Backend::Create<mln::gfx::Backend::Type::Metal>(GLFWwindow *window,
                                                                             bool capFrameRate) {
  return std::make_unique<GLFWMetalBackend>(window, capFrameRate);
}

}  // namespace gfx
}  // namespace mln
