#pragma once

#include <mbgl/map/map_observer.hpp>

#include <functional>

namespace mbgl {

class StubMapObserver : public MapObserver {
public:
    void onWillStartLoadingMap() final {
        if (willStartLoadingMapCallback) {
            willStartLoadingMapCallback();
        }
    }

    void onDidFinishLoadingMap() final {
        if (didFinishLoadingMapCallback) {
            didFinishLoadingMapCallback();
        }
    }

    void onDidFailLoadingMap(MapLoadError, const std::string&) final {
        if (didFailLoadingMapCallback) {
            didFailLoadingMapCallback();
        }
    }

    void onDidFinishLoadingStyle() final {
        if (didFinishLoadingStyleCallback) {
            didFinishLoadingStyleCallback();
        }
    }

    void onDidFinishRenderingFrame(const RenderFrameStatus& status) final {
        if (didFinishRenderingFrameCallback) {
            didFinishRenderingFrameCallback(status);
        }
    }

    void onDidBecomeIdle() final {
        if (didBecomeIdleCallback) {
            didBecomeIdleCallback();
        }
    }

    void onRegisterShaders(gfx::ShaderRegistry& registry) final {
        if (onRegisterShadersCallback) {
            onRegisterShadersCallback(registry);
        }
    }

    void onPreCompileShader(shaders::BuiltIn id, gfx::Backend::Type type, const std::string& additionalDefines) final {
        if (onPreCompileShaderCallback) {
            onPreCompileShaderCallback(id, type, additionalDefines);
        }
    }

    void onPostCompileShader(shaders::BuiltIn id, gfx::Backend::Type type, const std::string& additionalDefines) final {
        if (onPostCompileShaderCallback) {
            onPostCompileShaderCallback(id, type, additionalDefines);
        }
    }

    void onShaderCompileFailed(shaders::BuiltIn id,
                               gfx::Backend::Type type,
                               const std::string& additionalDefines) final {
        if (onShaderCompileFailedCallback) {
            onShaderCompileFailedCallback(id, type, additionalDefines);
        }
    }

    void onGlyphsLoaded(const FontStack& stack, const GlyphRange& range) final {
        if (onGlyphsLoadedCallback) {
            onGlyphsLoadedCallback(stack, range);
        }
    }

    void onGlyphsError(const FontStack& stack, const GlyphRange& range, std::exception_ptr ex) final {
        if (onGlyphsErrorCallback) {
            onGlyphsErrorCallback(stack, range, ex);
        }
    }

    void onGlyphsRequested(const FontStack& stack, const GlyphRange& range) final {
        if (onGlyphsRequestedCallback) {
            onGlyphsRequestedCallback(stack, range);
        }
    }

    void onTileAction(TileOperation op, const OverscaledTileID& id, const std::string& sourceID) final {
        if (onTileActionCallback) {
            onTileActionCallback(op, id, sourceID);
        }
    }

    std::function<void()> willStartLoadingMapCallback;
    std::function<void()> didFinishLoadingMapCallback;
    std::function<void()> didFailLoadingMapCallback;
    std::function<void()> didFinishLoadingStyleCallback;
    std::function<void(RenderFrameStatus)> didFinishRenderingFrameCallback;
    std::function<void()> didBecomeIdleCallback;
    std::function<void(gfx::ShaderRegistry&)> onRegisterShadersCallback;
    std::function<void(shaders::BuiltIn, gfx::Backend::Type, const std::string&)> onPreCompileShaderCallback;
    std::function<void(shaders::BuiltIn, gfx::Backend::Type, const std::string&)> onPostCompileShaderCallback;
    std::function<void(shaders::BuiltIn, gfx::Backend::Type, const std::string&)> onShaderCompileFailedCallback;
    std::function<void(const FontStack&, const GlyphRange&)> onGlyphsLoadedCallback;
    std::function<void(const FontStack&, const GlyphRange&, std::exception_ptr)> onGlyphsErrorCallback;
    std::function<void(const FontStack&, const GlyphRange&)> onGlyphsRequestedCallback;
    std::function<void(TileOperation, const OverscaledTileID&, const std::string&)> onTileActionCallback;
};

} // namespace mbgl
