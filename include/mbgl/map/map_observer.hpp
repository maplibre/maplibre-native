#pragma once

#include <mbgl/tile/tile_operation.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/style/source.hpp>
#include <mbgl/style/image.hpp>
#include <mbgl/style/sprite.hpp>
#include <mbgl/text/glyph.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/gfx/rendering_stats.hpp>

#include <cstdint>
#include <string>

namespace mbgl {

namespace gfx {
class ShaderRegistry;
}

enum class MapLoadError {
    StyleParseError,
    StyleLoadError,
    NotFoundError,
    UnknownError,
};

class MapObserver {
public:
    virtual ~MapObserver() = default;

    static MapObserver& nullObserver() {
        static MapObserver mapObserver;
        return mapObserver;
    }

    enum class CameraChangeMode : uint32_t {
        Immediate,
        Animated
    };

    enum class RenderMode : uint32_t {
        Partial,
        Full
    };

    struct RenderFrameStatus {
        RenderMode mode;
        bool needsRepaint; // In continous mode, shows that there are ongoig transitions.
        bool placementChanged;
        gfx::RenderingStats renderingStats;
    };

    virtual void onCameraWillChange(CameraChangeMode) {}
    virtual void onCameraIsChanging() {}
    virtual void onCameraDidChange(CameraChangeMode) {}
    virtual void onWillStartLoadingMap() {}
    virtual void onDidFinishLoadingMap() {}
    virtual void onDidFailLoadingMap(MapLoadError, const std::string&) {}
    virtual void onWillStartRenderingFrame() {}
    virtual void onDidFinishRenderingFrame(const RenderFrameStatus&) {}
    virtual void onWillStartRenderingMap() {}
    virtual void onDidFinishRenderingMap(RenderMode) {}
    virtual void onDidFinishLoadingStyle() {}
    virtual void onSourceChanged(style::Source&) {}
    virtual void onDidBecomeIdle() {}
    virtual void onStyleImageMissing(const std::string&) {}
    /// This method should return true if unused image can be removed,
    /// false otherwise. By default, unused image will be removed.
    virtual bool onCanRemoveUnusedStyleImage(const std::string&) { return true; }
    // Observe this event to easily mutate or observe shaders as soon
    // as the registry becomes available.
    virtual void onRegisterShaders(gfx::ShaderRegistry&) {}

    // Shaders compilation
    virtual void onPreCompileShader(shaders::BuiltIn, gfx::Backend::Type, const std::string&) {}
    virtual void onPostCompileShader(shaders::BuiltIn, gfx::Backend::Type, const std::string&) {}
    virtual void onShaderCompileFailed(shaders::BuiltIn, gfx::Backend::Type, const std::string&) {}

    // Glyph requests
    virtual void onGlyphsLoaded(const FontStack&, const GlyphRange&) {}
    virtual void onGlyphsError(const FontStack&, const GlyphRange&, std::exception_ptr) {}
    virtual void onGlyphsRequested(const FontStack&, const GlyphRange&) {}

    // Tile requests
    virtual void onTileAction(TileOperation, const OverscaledTileID&, const std::string&) {}

    // Sprite requests
    virtual void onSpriteLoaded(const std::optional<style::Sprite>&) {}
    virtual void onSpriteError(const std::optional<style::Sprite>&, std::exception_ptr) {}
    virtual void onSpriteRequested(const std::optional<style::Sprite>&) {}
};

} // namespace mbgl
