#pragma once

#include <mln/util/size.hpp>
#include <mln/gfx/backend.hpp>

namespace mln {
namespace gfx {
class RendererBackend;
} // namespace gfx
} // namespace mln

struct GLFWwindow;

class GLFWBackend {
public:
    explicit GLFWBackend() = default;
    GLFWBackend(const GLFWBackend&) = delete;
    GLFWBackend& operator=(const GLFWBackend&) = delete;
    virtual ~GLFWBackend() = default;

    static std::unique_ptr<GLFWBackend> Create(GLFWwindow* window, bool capFrameRate) {
        return mln::gfx::Backend::Create<GLFWBackend, GLFWwindow*, bool>(window, capFrameRate);
    }

    virtual mln::gfx::RendererBackend& getRendererBackend() = 0;
    virtual mln::Size getSize() const = 0;
    virtual void setSize(mln::Size) = 0;
};
