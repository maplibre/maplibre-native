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
    // Drape target resolution = base tile size x qualityFactor. maplibre-gl-js renders
    // its render-to-texture tiles at tileSize * qualityFactor (qualityFactor = 2, i.e.
    // 1024x1024) precisely so draped content - thin lines especially - is not pixelated
    // or aliased when the terrain mesh magnifies it onto the screen. Its own words
    // (src/render/terrain.ts): "to not see pixels in the render-to-texture tiles it is
    // good to render them bigger ... a value of 2 should be fine". At 512 (qualityFactor
    // 1) draped roads look aliased because the line AA band, sized to the screen's device
    // pixel ratio, falls sub-texel in the lower-resolution target. Lower drapeQualityFactor
    // back to 1 to save GPU memory on constrained devices (each step is 4x memory/target).
    static constexpr uint32_t drapeTileSize = 512;
    static constexpr uint32_t drapeQualityFactor = 2;
    TexturePool texturePool{drapeTileSize * drapeQualityFactor};

    gfx::RendererBackend& backend;

    RendererObserver* observer;

    const float pixelRatio;
    std::unique_ptr<RenderStaticData> staticData;
    gfx::DynamicTextureAtlasPtr dynamicTextureAtlas;
    bool styleLoaded = false;

    // Previous frame's frame-global draped-content signature, to detect when the terrain
    // drape content changed and the draped tweakers/targets must re-run (see render()).
    std::size_t lastDrapedContentSignature = 0;

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
