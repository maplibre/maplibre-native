#pragma once

#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/renderer/renderer_frontend.hpp>

#include <memory>
#include <optional>
#include <string>

namespace mbgl {

class Renderer;
class UpdateParameters;

namespace ohos {

class RendererFrontend final : public mbgl::RendererFrontend {
public:
    RendererFrontend(gfx::RendererBackend&,
                     float pixelRatio,
                     std::optional<std::string> localFontFamily = std::nullopt);
    ~RendererFrontend() override;

    void reset() override;
    void setObserver(RendererObserver&) override;
    void update(std::shared_ptr<UpdateParameters>) override;
    const TaggedScheduler& getThreadPool() const override;

    bool renderFrame();
    bool hasPendingRender() const;
    void setTileCacheEnabled(bool);
    bool getTileCacheEnabled() const;
    void reduceMemoryUse();
    Renderer* getRenderer();

private:
    gfx::RendererBackend& backend;
    std::unique_ptr<Renderer> renderer;
    std::shared_ptr<UpdateParameters> updateParameters;
    bool needsRender = false;
};

} // namespace ohos
} // namespace mbgl
