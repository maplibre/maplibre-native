#pragma once

#include "camera_bounds_options.hpp"
#include "renderer_frontend.hpp"
#include "window_backend.hpp"

#include <mbgl/map/bound_options.hpp>
#include <mbgl/map/map.hpp>
#include <mbgl/map/map_observer.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/util/size.hpp>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

namespace mbgl {
namespace ohos {

class MapView final : public MapObserver {
public:
    explicit MapView(float pixelRatio = 1.0f);
    ~MapView() override;

    MapView(const MapView&) = delete;
    MapView& operator=(const MapView&) = delete;

    void setSurface(OHNativeWindow*, Size);
    void clearSurface();
    bool renderFrame();
    void runLoopOnce();
    void reduceMemoryUse();

    void setStyleURL(const std::string&);
    void setStyleJSON(const std::string&);
    void jumpTo(CameraOptions);
    void easeTo(CameraOptions, AnimationOptions);
    void flyTo(CameraOptions, AnimationOptions);
    void fitBounds(CameraBoundsOptions);
    void setFreeCameraOptions(FreeCameraOptions);
    void setGestureInProgress(bool);
    void moveBy(double x, double y, AnimationOptions = {});
    void pitchBy(double deltaPitch);
    void scaleBy(double scale, double anchorX, double anchorY);
    void flyBy(double scale, double anchorX, double anchorY, AnimationOptions = {});
    void rotateBy(double previousAngle, double currentAngle, double anchorX, double anchorY);
    void setDebugOptions(MapDebugOptions);
    void setBounds(BoundOptions);
    void setClientOptions(std::string name, std::string version);
    void setResourceOptions(ResourceOptions);
    void setTileCacheEnabled(bool);
    void setPixelRatio(float);
    void setSize(Size);

    bool hasMap() const { return map != nullptr; }
    OHNativeWindow* getNativeWindow() const { return window; }
    Size getSurfaceSize() const { return surfaceSize; }
    float getPixelRatio() const { return pixelRatio; }
    MapDebugOptions getDebugOptions() const { return debugOptions; }
    BoundOptions getBounds() const;
    bool getTileCacheEnabled() const { return tileCacheEnabled; }
    CameraOptions getCameraOptions() const;
    FreeCameraOptions getFreeCameraOptions() const;
    CameraOptions cameraForBounds(const CameraBoundsOptions&) const;
    const std::string& getClientName() const { return clientName; }
    const std::string& getClientVersion() const { return clientVersion; }
    const ResourceOptions& getResourceOptions() const { return resourceOptions; }
    bool hasPendingRender() const;
    std::uint64_t getRenderedFrameCount() const { return renderedFrameCount; }
    bool hasLoadedStyle() const { return styleLoaded; }
    bool hasLoadedMap() const { return mapLoaded; }
    bool isFullyLoaded() const;
    bool isIdle() const { return idle; }
    bool lastFrameNeededRepaint() const { return lastNeedsRepaint; }
    bool lastFrameWasComplete() const { return lastFrameComplete; }
    std::uint64_t getCoreFrameCount() const { return coreFrameCount; }
    std::uint64_t getMapLoadErrorCount() const { return mapLoadErrorCount; }
    std::uint64_t getRenderErrorCount() const { return renderErrorCount; }
    std::uint64_t getSourceChangedCount() const { return sourceChangedCount; }
    std::uint64_t getStyleImageMissingCount() const { return styleImageMissingCount; }
    std::uint64_t getGlyphsRequestedCount() const { return glyphsRequestedCount; }
    std::uint64_t getGlyphsLoadedCount() const { return glyphsLoadedCount; }
    std::uint64_t getGlyphsErrorCount() const { return glyphsErrorCount; }
    std::uint64_t getTileActionCount() const { return tileActionCount; }
    std::uint64_t getSpritesRequestedCount() const { return spritesRequestedCount; }
    std::uint64_t getSpritesLoadedCount() const { return spritesLoadedCount; }
    std::uint64_t getSpritesErrorCount() const { return spritesErrorCount; }
    double getLastFrameTimeMs() const { return lastFrameTimeMs; }
    double getLastRunLoopTimeMs() const { return lastRunLoopTimeMs; }
    double getLastRenderTimeMs() const { return lastRenderTimeMs; }
    std::int32_t getGlesContextClientVersion() const;
    const std::string& getEGLConfigDiagnostic() const;
    const std::string& getFramebufferDiagnostic() const;
    const std::string& getRendererDiagnostic() const;
    const std::string& getLastMapLoadError() const { return lastMapLoadError; }
    const std::string& getLastRenderError() const { return lastRenderError; }
    const std::string& getLastStyleImageMissing() const { return lastStyleImageMissing; }
    const std::string& getLastGlyphsError() const { return lastGlyphsError; }
    const std::string& getLastSpritesError() const { return lastSpritesError; }

private:
    void createMap(OHNativeWindow*, Size);
    CameraOptions mergeCameraOptions(const CameraOptions&) const;
    FreeCameraOptions mergeFreeCameraOptions(const FreeCameraOptions&) const;
    BoundOptions mergeBoundOptions(const BoundOptions&) const;
    void applyDesiredBounds();
    void applyDesiredCamera();

    void onDidFailLoadingMap(MapLoadError, const std::string&) override;
    void onDidFinishLoadingMap() override;
    void onDidFinishLoadingStyle() override;
    void onWillStartRenderingFrame() override;
    void onDidFinishRenderingFrame(const RenderFrameStatus&) override;
    void onDidBecomeIdle() override;
    void onSourceChanged(style::Source&) override;
    void onStyleImageMissing(const std::string&) override;
    void onGlyphsLoaded(const FontStack&, const GlyphRange&) override;
    void onGlyphsError(const FontStack&, const GlyphRange&, std::exception_ptr) override;
    void onGlyphsRequested(const FontStack&, const GlyphRange&) override;
    void onTileAction(TileOperation, const OverscaledTileID&, const std::string&) override;
    void onSpriteLoaded(const std::optional<style::Sprite>&) override;
    void onSpriteError(const std::optional<style::Sprite>&, std::exception_ptr) override;
    void onSpriteRequested(const std::optional<style::Sprite>&) override;
    void onRenderError(std::exception_ptr) override;

    void resetRuntimeState();

    float pixelRatio = 1.0f;
    OHNativeWindow* window = nullptr;
    Size surfaceSize;
    std::unique_ptr<WindowBackend> backend;
    std::unique_ptr<RendererFrontend> frontend;
    std::unique_ptr<Map> map;
    std::optional<CameraOptions> desiredCamera;
    std::optional<CameraBoundsOptions> desiredCameraBounds;
    std::optional<FreeCameraOptions> desiredFreeCamera;
    std::optional<BoundOptions> desiredBounds;
    MapDebugOptions debugOptions = MapDebugOptions::NoDebug;
    bool tileCacheEnabled = true;
    bool styleLoaded = false;
    bool mapLoaded = false;
    bool idle = false;
    bool lastNeedsRepaint = false;
    bool lastFrameComplete = false;
    std::uint64_t renderedFrameCount = 0;
    std::uint64_t coreFrameCount = 0;
    std::uint64_t mapLoadErrorCount = 0;
    std::uint64_t renderErrorCount = 0;
    std::uint64_t sourceChangedCount = 0;
    std::uint64_t styleImageMissingCount = 0;
    std::uint64_t glyphsRequestedCount = 0;
    std::uint64_t glyphsLoadedCount = 0;
    std::uint64_t glyphsErrorCount = 0;
    std::uint64_t tileActionCount = 0;
    std::uint64_t spritesRequestedCount = 0;
    std::uint64_t spritesLoadedCount = 0;
    std::uint64_t spritesErrorCount = 0;
    double lastFrameTimeMs = 0.0;
    double lastRunLoopTimeMs = 0.0;
    double lastRenderTimeMs = 0.0;
    std::string lastMapLoadError;
    std::string lastRenderError;
    std::string lastStyleImageMissing;
    std::string lastGlyphsError;
    std::string lastSpritesError;
    std::string clientName;
    std::string clientVersion;
    ResourceOptions resourceOptions;
};

} // namespace ohos
} // namespace mbgl
