#pragma once

#include <mln/annotation/annotation_manager.hpp>
#include <mln/map/map.hpp>
#include <mln/map/map_observer.hpp>
#include <mln/map/map_options.hpp>
#include <mln/map/mode.hpp>
#include <mln/map/transform.hpp>
#include <mln/renderer/renderer_frontend.hpp>
#include <mln/renderer/renderer_observer.hpp>
#include <mln/style/observer.hpp>
#include <mln/style/source.hpp>
#include <mln/style/style.hpp>
#include <mln/util/size.hpp>
#include <mln/tile/tile_operation.hpp>

#include <limits>
#include <numbers>

namespace mln {

class FileSource;
class ResourceTransform;

namespace gfx {
class ShaderRegistry;
} // namespace gfx

namespace util {
class ActionJournal;
} // namespace util

struct StillImageRequest {
    StillImageRequest(Map::StillImageCallback&& callback_)
        : callback(std::move(callback_)) {}

    Map::StillImageCallback callback;
};

class Map::Impl final : public TransformObserver, public style::Observer, public RendererObserver {
public:
    Impl(RendererFrontend&, MapObserver&, std::shared_ptr<FileSource>, const MapOptions&);
    ~Impl() final;

    // TransformObserver
    void onCameraWillChange(MapObserver::CameraChangeMode) final;
    void onCameraIsChanging() final;
    void onCameraDidChange(MapObserver::CameraChangeMode) final;

    // StyleObserver
    void onSourceChanged(style::Source&) final;
    void onUpdate() final;
    void onStyleLoading() final;
    void onStyleLoaded() final;
    void onStyleError(std::exception_ptr) final;
    void onSpriteLoaded(const std::optional<style::Sprite>&) final;
    void onSpriteError(const std::optional<style::Sprite>&, std::exception_ptr) final;
    void onSpriteRequested(const std::optional<style::Sprite>&) final;

    // RendererObserver
    void onInvalidate() final;
    void onResourceError(std::exception_ptr) final;
    void onWillStartRenderingFrame() final;
    void onDidFinishRenderingFrame(RenderMode, bool, bool, std::shared_ptr<gfx::RenderingStats>) final;
    void onWillStartRenderingMap() final;
    void onDidFinishRenderingMap() final;
    void onTerrainCenterElevationChanged(double elevationMeters) final;
    void onTerrainEyeElevationChanged(double elevationMeters) final;
    void onStyleImageMissing(const std::string&, const std::function<void()>&) final;
    void onRemoveUnusedStyleImages(const std::vector<std::string>&) final;
    void onRegisterShaders(gfx::ShaderRegistry&) final;

    void onPreCompileShader(shaders::BuiltIn, gfx::Backend::Type, const std::string&) final;
    void onPostCompileShader(shaders::BuiltIn, gfx::Backend::Type, const std::string&) final;
    void onShaderCompileFailed(shaders::BuiltIn, gfx::Backend::Type, const std::string&) final;
    void onGlyphsLoaded(const FontStack&, const GlyphRange&) final;
    void onGlyphsError(const FontStack&, const GlyphRange&, std::exception_ptr) final;
    void onGlyphsRequested(const FontStack&, const GlyphRange&) final;
    void onTileAction(TileOperation op, const OverscaledTileID&, const std::string&) final;
    void onRenderError(std::exception_ptr) final;
    void onSymbolError(const std::string&) final;

    // Map
    void jumpTo(const CameraOptions&);

    bool isRenderingStatsViewEnabled() const;
    void enableRenderingStatsView(bool value);

    MapObserver& observer;
    RendererFrontend& rendererFrontend;
    std::unique_ptr<util::ActionJournal> actionJournal;

    Transform transform;

    const MapMode mode;
    const float pixelRatio;
    const bool crossSourceCollisions;
    const bool fastPFOREnabled;
    const bool captureRenderedFeatures;

    MapDebugOptions debugOptions{MapDebugOptions::NoDebug};
    std::unique_ptr<gfx::RenderingStatsView> renderingStatsView;
    std::shared_ptr<gfx::RenderingStats> lastFrameStats;

    std::shared_ptr<FileSource> fileSource;

    std::unique_ptr<style::Style> style;
    AnnotationManager annotationManager;

    bool cameraMutated = false;

    uint8_t prefetchZoomDelta = util::DEFAULT_PREFETCH_ZOOM_DELTA;

    bool loading = false;
    bool rendererFullyLoaded;
    std::unique_ptr<StillImageRequest> stillImageRequest;

    double tileLodMinRadius = 3;
    double tileLodScale = 1;
    double tileLodPitchThreshold = (60.0 / 180.0) * std::numbers::pi;
    double tileLodZoomShift = 0;
    TileLodMode tileLodMode = TileLodMode::Default;
    TerrainLoadMode terrainLoadMode = TerrainLoadMode::Quality;
    TerrainSkirtLength terrainSkirtLength = TerrainSkirtLength::Auto;
    /// Off by default. Opt in with Map::setCenterClampedToGround. A gesture holds the center's
    /// height, and its end anchors the center on the ground keeping the view, the way GL JS's
    /// elevationFreeze and recalculateZoomAndCenter do.
    bool centerClampedToGround = false;
    /// Terrain height under the centre, as the renderer last reported it (NaN until then).
    double terrainCenterElevation = std::numeric_limits<double>::quiet_NaN();
    /// Terrain height under the camera's eye, as the renderer last reported it (NaN until then).
    double terrainEyeElevation = std::numeric_limits<double>::quiet_NaN();
    /// How far above the ground under it the eye is kept, in meters.
    static constexpr double eyeClearanceMeters = 10.0;
    /// The lowest height the center can be at with the eye `eyeClearanceMeters` above the ground
    /// under it; -infinity until the renderer has reported that ground.
    double eyeClearCenterAltitude() const;
    /// The height the clamp puts the center at: the ground under it, or higher when the eye
    /// would otherwise sit under the ground under the eye.
    double clampedCenterAltitude() const;
    /// The center altitude a gesture holds, as gl-js's elevationFreeze does, so the camera keeps
    /// its height while the ground under the center rises and falls; empty while no gesture is
    /// in progress.
    std::optional<double> gestureHeldCenterAltitude;
    /// How many more ground reports after an anchor may move the center along the line of sight
    /// instead of moving the camera. The anchor takes the height of the ground under the old
    /// center, so on a slope the center lands on a plane above or below the ground it then looks
    /// at; as gl-js's recalculateZoomAndCenter does in its passes, the next reports slide it along
    /// the line of sight onto that ground with the camera kept in place.
    int anchorPassesLeft = 0;
    /// The heights the passes have shown the line of sight meets the ground above and below:
    /// over broken ground the height under the center can flip between two values as it slides,
    /// so the passes halve the bracket instead of chasing them, and leave the center there once
    /// it is under a meter wide.
    double anchorBracketLow = -std::numeric_limits<double>::infinity();
    double anchorBracketHigh = std::numeric_limits<double>::infinity();
    /// The passes left the center beside a cliff in the data, off the height under it: the clamp
    /// leaves it there until the camera next moves, rather than move the camera onto that height.
    bool centerSettledBesideACliff = false;
    /// The running transition is a pan's momentum (an animated Map::moveBy), which the eye
    /// clearance may cut short, rather than a camera move the app asked for, which it leaves
    /// alone.
    bool transitionIsFling = false;
    /// What the clamp is owed when the transition or gesture ends (centerAwaitingTerrainAnchor)
    /// is an anchor that keeps the view, because a gesture or its fling moved the camera, and not
    /// the plain jump onto the ground that follows an animation the app asked for.
    bool centerAnchorKeepsView = false;
    /// Notes that the app sets the camera: a pass budget and a settled center belong to the
    /// camera being replaced, and `fling` marks a pan's momentum.
    void noteCameraSet(bool fling = false);
    /// Ends a hold: anchors the center keeping the view and starts the passes that follow it.
    void anchorAfterHold();
    /// Applies the clamp for the heights last reported, or leaves it to the end of a running
    /// transition.
    void applyCenterClamp();
    void anchorCenterOnTerrain();
    /// Slides the centre along the line of sight to `altitudeMeters`, the zoom following, so the
    /// camera stays where it is. False, changing nothing, when there was nothing to move, the line
    /// of sight nearly grazes the plane at that altitude, the plane is (nearly) at the camera, or
    /// the zoom limits, the bounds or the constrain mode would cut the move short.
    bool setCenterAltitudeKeepingView(double altitudeMeters);
    /// Whether the style had terrain when last checked, and whether the centre has yet to come
    /// back to sea level since it was removed (a transition was in flight, or a still map has
    /// not rendered since).
    bool hadTerrain = false;
    bool centerAwaitingSeaLevel = false;
    void noteTerrainRemoval();
    /// Brings the centre down to sea level keeping the view or, where the limits refuse that,
    /// keeping the centre and zoom.
    void returnCenterToSeaLevel();
    /// Runs a return still waiting before the app sets a camera, which every camera entry point
    /// does first, so that the return does not move the app's camera afterwards.
    void settleCenterAwaitingSeaLevel();
    /// A height report arrived while a camera transition or a gesture was running; the clamp
    /// applies it once that ends instead of cutting it short.
    bool centerAwaitingTerrainAnchor = false;
    /// Moves the centre altitude to `elevationMeters` keeping lng/lat; false when the change
    /// is under the sub-metre threshold.
    bool clampCenterToTerrain(double elevationMeters);
    bool debugAboveGroundLog = false;
};

// Forward declaration of this method is required for the MapProjection class
CameraOptions cameraForLatLngs(const std::vector<LatLng>& latLngs,
                               const Transform& transform,
                               const EdgeInsets& padding);

} // namespace mln
