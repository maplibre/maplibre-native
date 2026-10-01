#pragma once

#include "glfw_view.hpp"
#include <mln/renderer/renderer_frontend.hpp>

#include <memory>

namespace mln {
class Renderer;
} // namespace mln

class GLFWRendererFrontend : public mln::RendererFrontend {
public:
    GLFWRendererFrontend(std::unique_ptr<mln::Renderer>, GLFWView&);
    ~GLFWRendererFrontend() override;

    void reset() override;
    void setObserver(mln::RendererObserver&) override;

    void update(std::shared_ptr<mln::UpdateParameters>) override;
    const mln::TaggedScheduler& getThreadPool() const override;
    void render();

    mln::Renderer* getRenderer();

private:
    GLFWView& glfwView;
    std::unique_ptr<mln::Renderer> renderer;
    std::shared_ptr<mln::UpdateParameters> updateParameters;
};
