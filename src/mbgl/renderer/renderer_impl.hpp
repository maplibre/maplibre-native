#pragma once

#include <mbgl/renderer/render_orchestrator.hpp>
#include <mbgl/renderer/texture_pool.hpp>
#include <mbgl/gfx/context_observer.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <mbgl/mtl/mtl_fwd.hpp>
#include <Foundation/Foundation.hpp>
#endif // MLN_RENDER_BACKEND_METAL

#include <memory>
#include <string>

namespace mbgl {

class RendererObserver;
class RenderStaticData;
class RenderTree;

namespace gfx {
class RendererBackend;
class ShadeRegistry;
class DynamicTextureAtlas;
using DynamicTextureAtlasPtr = std::shared_ptr<gfx::DynamicTextureAtlas>;
} // namespace gfx

class Renderer::Impl : public gfx::ContextObserver {
public:
    Impl(gfx::RendererBackend&, float pixelRatio_, const std::optional<std::string>& localFontFamily_);
    virtual ~Impl();

    // ContextObserver
    void onPreCompileShader(shaders::BuiltIn, gfx::Backend::Type, const std::string&) override;
    void onPostCompileShader(shaders::BuiltIn, gfx::Backend::Type, const std::string&) override;
    void onShaderCompileFailed(shaders::BuiltIn, gfx::Backend::Type, const std::string&) override;
    void onRenderError(std::exception_ptr) override;

private:
    friend class Renderer;

    void setObserver(RendererObserver*);

    void render(const RenderTree&, const std::shared_ptr<UpdateParameters>&);

    void reduceMemoryUse();

    // TODO: Move orchestrator to Map::Impl.
    RenderOrchestrator orchestrator;

    /// Terrain drape render targets, persistent across frames. These must outlive
    /// the frame: each target caches its rendered texture (and the coverage baked
    /// into it, see RenderTarget::render), and the terrain drawables hold the
    /// textures they sample. Rebuilding the pool per frame would reallocate every
    /// tile-sized target every frame - churning GPU memory and discarding the
    /// baked content that suppresses drape flicker.
    TexturePool texturePool{512}; // TODO: tile size

    gfx::RendererBackend& backend;

    RendererObserver* observer;

    const float pixelRatio;
    std::unique_ptr<RenderStaticData> staticData;
    gfx::DynamicTextureAtlasPtr dynamicTextureAtlas;
    bool styleLoaded = false;

    enum class RenderState {
        Never,
        Partial,
        Fully,
    };

    RenderState renderState = RenderState::Never;

    uint64_t frameCount = 0;

#if MLN_RENDER_BACKEND_METAL
    mtl::MTLCaptureScopePtr commandCaptureScope;
#endif // MLN_RENDER_BACKEND_METAL
};

} // namespace mbgl
