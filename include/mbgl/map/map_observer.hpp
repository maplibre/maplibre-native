#pragma once

#include <mbgl/style/source.hpp>
#include <mbgl/style/image.hpp>

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
        double frameEncodingTime;
        double frameRenderingTime;
    };

    virtual void onCameraWillChange(CameraChangeMode) {}
    virtual void onCameraIsChanging() {}
    virtual void onCameraDidChange(CameraChangeMode) {}
    virtual void onWillStartLoadingMap() {}
    virtual void onDidFinishLoadingMap() {}
    virtual void onDidFailLoadingMap(MapLoadError, const std::string&) {}
    virtual void onWillStartRenderingFrame() {}
    virtual void onDidFinishRenderingFrame(RenderFrameStatus) {}
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
    virtual void onRegisterShaders(gfx::ShaderRegistry&) {};
};

} // namespace mbgl
