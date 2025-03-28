#pragma once

#include <mbgl/tile/tile_id.hpp>
#include <mbgl/util/font_stack.hpp>
#include <mbgl/gfx/rendering_stats.hpp>
#include <mbgl/text/glyph_range.hpp>
#include <mbgl/tile/tile_operation.hpp>
#include <mbgl/gfx/backend.hpp>
#include <mbgl/shaders/shader_source.hpp>

#include <cstdint>
#include <exception>
#include <functional>
#include <string>

namespace mbgl {

namespace gfx {
class ShaderRegistry;
}

class RendererObserver {
public:
    virtual ~RendererObserver() = default;

    enum class RenderMode : uint32_t {
        Partial,
        Full
    };

    /// Signals that a repaint is required
    virtual void onInvalidate() {}

    /// Resource failed to download / parse
    virtual void onResourceError(std::exception_ptr) {}

    /// First frame
    virtual void onWillStartRenderingMap() {}

    /// Start of frame, initial is the first frame for this map
    virtual void onWillStartRenderingFrame() {}

    /// End of frame, booleans flags that a repaint is required and that placement changed.
    virtual void onDidFinishRenderingFrame(RenderMode, bool /*repaint*/, bool /*placementChanged*/) {}

    /// End of frame, booleans flags that a repaint is required and that placement changed.
    virtual void onDidFinishRenderingFrame(RenderMode mode,
                                           bool repaint,
                                           bool placementChanged,
                                           double /*frameEncodingTime*/,
                                           double /*frameRenderingTime*/) {
        onDidFinishRenderingFrame(mode, repaint, placementChanged);
    }

    virtual void onDidFinishRenderingFrame(RenderMode mode,
                                           bool repaint,
                                           bool placementChanged,
                                           const gfx::RenderingStats& stats) {
        onDidFinishRenderingFrame(mode, repaint, placementChanged, stats.encodingTime, stats.renderingTime);
    }

    /// Final frame
    virtual void onDidFinishRenderingMap() {}

    /// Style is missing an image
    using StyleImageMissingCallback = std::function<void()>;
    virtual void onStyleImageMissing(const std::string&, const StyleImageMissingCallback& done) { done(); }
    virtual void onRemoveUnusedStyleImages(const std::vector<std::string>&) {}

    // Entry point for custom shader registration
    virtual void onRegisterShaders(gfx::ShaderRegistry&) {};
    virtual void onPreCompileShader(shaders::BuiltIn, gfx::Backend::Type, const std::string&) {}
    virtual void onPostCompileShader(shaders::BuiltIn, gfx::Backend::Type, const std::string&) {}
    virtual void onShaderCompileFailed(shaders::BuiltIn, gfx::Backend::Type, const std::string&) {}

    // Glyph loading
    virtual void onGlyphsLoaded(const FontStack&, const GlyphRange&) {}
    virtual void onGlyphsError(const FontStack&, const GlyphRange&, std::exception_ptr) {}
    virtual void onGlyphsRequested(const FontStack&, const GlyphRange&) {}

    // Tile loading
    virtual void onTileAction(TileOperation, const OverscaledTileID&, const std::string&) {}
};

} // namespace mbgl
