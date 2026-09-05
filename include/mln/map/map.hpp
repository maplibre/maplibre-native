#pragma once

#include <mln/annotation/annotation.hpp>
#include <mln/map/bound_options.hpp>
#include <mln/map/camera.hpp>
#include <mln/map/map_observer.hpp>
#include <mln/map/map_options.hpp>
#include <mln/map/mode.hpp>
#include <mln/map/projection_mode.hpp>
#include <mln/storage/resource_options.hpp>
#include <mln/util/action_journal_options.hpp>
#include <mln/util/chrono.hpp>
#include <mln/util/client_options.hpp>
#include <mln/util/geometry.hpp>
#include <mln/util/noncopyable.hpp>
#include <mln/util/size.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace mln {

class RendererFrontend;
class TransformState;

namespace style {
class Image;
class Style;
} // namespace style

namespace util {
class ActionJournal;
} // namespace util

class Map : private util::noncopyable {
public:
    explicit Map(RendererFrontend&,
                 MapObserver&,
                 const MapOptions&,
                 const ResourceOptions&,
                 const ClientOptions& = ClientOptions(),
                 const util::ActionJournalOptions& = util::ActionJournalOptions());
    ~Map();

    /// Register a callback that will get called (on the render thread) when all
    /// resources have been loaded and a complete render occurs.
    using StillImageCallback = std::function<void(std::exception_ptr)>;
    void renderStill(StillImageCallback);
    void renderStill(const CameraOptions&, MapDebugOptions, StillImageCallback);

    /// Triggers a repaint.
    void triggerRepaint();

    style::Style& getStyle();
    const style::Style& getStyle() const;

    void setStyle(std::unique_ptr<style::Style>);

    // Transition
    void cancelTransitions();
    void setGestureInProgress(bool);
    bool isGestureInProgress() const;
    bool isRotating() const;
    bool isScaling() const;
    bool isPanning() const;

    // Camera
    CameraOptions getCameraOptions(const std::optional<EdgeInsets>& = std::nullopt) const;
    void jumpTo(const CameraOptions&);
    void easeTo(const CameraOptions&, const AnimationOptions&);
    void flyTo(const CameraOptions&, const AnimationOptions&);
    void moveBy(const ScreenCoordinate&, const AnimationOptions& = {});
    void scaleBy(double scale, const std::optional<ScreenCoordinate>& anchor, const AnimationOptions& animation = {});
    void pitchBy(double pitch, const AnimationOptions& animation = {});
    void rotateBy(const ScreenCoordinate& first, const ScreenCoordinate& second, const AnimationOptions& = {});
    CameraOptions cameraForLatLngBounds(const LatLngBounds&,
                                        const EdgeInsets&,
                                        const std::optional<double>& bearing = std::nullopt,
                                        const std::optional<double>& pitch = std::nullopt) const;
    CameraOptions cameraForLatLngs(const std::vector<LatLng>&,
                                   const EdgeInsets&,
                                   const std::optional<double>& bearing = std::nullopt,
                                   const std::optional<double>& pitch = std::nullopt) const;
    CameraOptions cameraForGeometry(const Geometry<double>&,
                                    const EdgeInsets&,
                                    const std::optional<double>& bearing = std::nullopt,
                                    const std::optional<double>& pitch = std::nullopt) const;
    LatLngBounds latLngBoundsForCamera(const CameraOptions&) const;
    LatLngBounds latLngBoundsForCameraUnwrapped(const CameraOptions&) const;

    /// @name Bounds
    /// @{

    void setBounds(const BoundOptions& options);

    /// Returns the current map bound options. All optional fields in BoundOptions are set.
    BoundOptions getBounds() const;

    /// @}

    // Map Options
    void setNorthOrientation(NorthOrientation);
    void setConstrainMode(ConstrainMode);
    void setViewportMode(ViewportMode);
    void setSize(Size);
    void setFrustumOffset(const EdgeInsets&);
    EdgeInsets getFrustumOffset();
    MapOptions getMapOptions() const;

    // Projection Mode
    void setProjectionMode(const ProjectionMode&);
    ProjectionMode getProjectionMode() const;

    // Projection
    ScreenCoordinate pixelForLatLng(const LatLng&) const;
    /// Screen position of a point `elevationMeters` above sea level. With 3D terrain, pass the
    /// (exaggerated) terrain height at that location - see `Renderer::queryTerrainElevation` -
    /// so annotations and markers land on the draped surface rather than at sea level.
    ScreenCoordinate pixelForLatLng(const LatLng&, double elevationMeters) const;
    LatLng latLngForPixel(const ScreenCoordinate&, LatLng::WrapMode = LatLng::Wrapped) const;
    /// Geographic position of the point where the screen pixel's view ray crosses the plane
    /// `elevationMeters` above sea level (exaggerated terrain units, as pixelForLatLng). A
    /// terrain-aware pick marches this along the ray - see MLNMapView convertPoint:toLatLngFromView:.
    LatLng latLngForPixel(const ScreenCoordinate&, double elevationMeters) const;
    std::vector<ScreenCoordinate> pixelsForLatLngs(const std::vector<LatLng>&) const;
    std::vector<LatLng> latLngsForPixels(const std::vector<ScreenCoordinate>&,
                                         LatLng::WrapMode = LatLng::Wrapped) const;

    // Transform
    TransformState getTransformState() const;
    [[deprecated("Use getTransformState()")]] TransformState getTransfromState() const;

    // Annotations
    void addAnnotationImage(std::unique_ptr<style::Image>);
    void removeAnnotationImage(const std::string&);
    double getTopOffsetPixelsForAnnotationImage(const std::string&);

    AnnotationID addAnnotation(const Annotation&);
    void updateAnnotation(AnnotationID, const Annotation&);
    void removeAnnotation(AnnotationID);

    // Tile prefetching
    //
    /// When loading a map, if `PrefetchZoomDelta` is set to any number greater
    /// than 0, the map will first request a tile for `zoom - delta` in a
    /// attempt to display a full map at lower resolution as quick as possible.
    /// It will get clamped at the tile source minimum zoom. The default `delta`
    /// is 4.
    void setPrefetchZoomDelta(uint8_t delta);
    uint8_t getPrefetchZoomDelta() const;

    // Debug
    void setDebug(MapDebugOptions);
    MapDebugOptions getDebug() const;

    bool isRenderingStatsViewEnabled() const;
    void enableRenderingStatsView(bool value);

    bool isFullyLoaded() const;
    void dumpDebugLogs() const;

    /// FreeCameraOptions provides more direct access to the underlying camera
    /// entity. For backwards compatibility the state set using this API must be
    /// representable with `CameraOptions` as well. Parameters are clamped to a
    /// valid range or discarded as invalid if the conversion to the pitch and
    /// bearing presentation is ambiguous. For example orientation can be
    /// invalid if it leads to the camera being upside down or the quaternion
    /// has zero length.
    void setFreeCameraOptions(const FreeCameraOptions& camera);
    FreeCameraOptions getFreeCameraOptions() const;

    // Tile LOD controls
    //
    /// The number of map tile requests can be reduced by using a lower level
    /// of details (Lower zoom level) away from the camera.
    /// This can improve performance, particularly when the camera pitch is high.
    /// The LOD calculation uses a heuristic based on the distance to the camera
    /// view point. The heuristic behavior is controlled with 3 parameters:
    /// - `TileLodMinRadius` is a radius around the view point in unit of tiles
    /// in which the fine grained zoom level tiles are always used
    /// - `TileLodScale` is a scale factor for the distance to the camera view
    /// point. A value larger than 1 increases the distance to the camera view
    /// point in which case the LOD is reduced
    /// - `TileLodPitchThreshold` is the pitch angle in radians above which LOD
    /// calculation is performed.
    /// LOD calculation is always performed if `TileLodPitchThreshold` is zero.
    /// LOD calculation is never performed if `TileLodPitchThreshold` is pi.
    /// - `TileLodZoomShift` shifts the the Zoom level used for LOD calculation
    /// A value of zero (default) does not change the Zoom level
    /// A positive value increases the Zoom level and a negative value decreases
    /// the Zoom level
    /// A negative values typically improves performance but reduces quality.
    /// For instance, a value of -1 reduces the zoom level by 1 and this
    /// reduces the number of tiles by a factor of 4 for the same camera view.
    /// - `TileLodMode` selects the tile cover algorithm. The possible algorithms
    /// are "default" and "distance". When "distance" is selected, the tile LOD is
    /// based on the distance from camera to tile. This algorithm causes tiles
    /// closer to the camera to be rendered at higher LOD, in contrast with the
    /// default algorithm, which renders the highest LOD at the center of the
    /// screen. The distance based algorithm observes `TileLodScale` and
    /// `TileLodPitchThreshold` and ignores `TileLodMinRadius` and
    /// `TileLodZoomShift`.
    /// When "adaptive" is selected, the zoom is chosen per tile from the field of
    /// view and a tile-count budget - maplibre-gl-js's tile zoom function - at every
    /// pitch rather than past a threshold. It observes none of the four settings
    /// above.
    void setTileLodMinRadius(double radius);
    double getTileLodMinRadius() const;
    void setTileLodScale(double scale);
    double getTileLodScale() const;
    void setTileLodPitchThreshold(double threshold);
    double getTileLodPitchThreshold() const;
    void setTileLodZoomShift(double shift);
    double getTileLodZoomShift() const;
    void setTileLodMode(TileLodMode mode);
    TileLodMode getTileLodMode() const;

    /// Selects the 3D-terrain progressive-loading budget (see TerrainLoadMode). Default
    /// is Quality (no budget). Balanced/Performance spread new-tile builds and drape
    /// re-renders across frames for smoother interaction on weaker GPUs, at the cost of a
    /// brief progressive fill-in. No effect when terrain is not enabled.
    void setTerrainLoadMode(TerrainLoadMode mode);
    TerrainLoadMode getTerrainLoadMode() const;

    /// Selects whether terrain tiles are skirted (see TerrainSkirtLength). Default is Auto.
    /// None suits a map drawn over a transparent background, where the skirts would show as
    /// vertical artifacts. Changing it rebuilds the terrain mesh and every tile drawable.
    /// No effect when terrain is not enabled.
    void setTerrainSkirtLength(TerrainSkirtLength length);
    TerrainSkirtLength getTerrainSkirtLength() const;

    /// Whether the map centre rides the terrain surface rather than sea level. Off by
    /// default (the clamp is applied asynchronously and can run away during a gesture);
    /// with it on the centre altitude follows the rendered terrain height under the
    /// centre, so pitching over tall ground keeps the camera above it instead of sinking
    /// in, as in maplibre-gl-js. Inert when terrain is not enabled.
    ///
    /// Gestures anchor the centre on the terrain (anchorCenterOnTerrain), and removing the
    /// terrain brings it back to sea level, whatever this is set to. maplibre-gl-js does both
    /// only with its clamp on, which is its default; gated on a clamp that is off by default
    /// here, gestures would go back to turning round a sea-level point under the terrain.
    void setCenterClampedToGround(bool clamped);
    bool getCenterClampedToGround() const;

    /// Moves the camera's centre onto the terrain under it without moving the camera: the centre
    /// slides along the line of sight up to the ground and the zoom tightens by the distance saved,
    /// so the camera does not move. The zoom does rise (about half a level over an 1,800 m
    /// mountain at pitch 60), so zoom-dependent styling and the tile zoom can step when a gesture
    /// starts, as in maplibre-gl-js. Rotating and pitching then turn around the ground being
    /// looked at, not a sea-level point behind it. Inert while the style has no terrain, so a
    /// gesture handler can call it unconditionally. Inert too at a pitch so steep (over ~84
    /// degrees) that the line of sight nearly grazes the ground, and where the zoom limits or the
    /// bounds would cut the move short. Removing the terrain brings the centre back down to sea
    /// level the same way or, where the limits refuse that, keeping its centre and zoom.
    ///
    /// A one-off that keeps the view, for the start of a gesture, where setCenterClampedToGround
    /// is the continuous follow; this runs whatever that is set to. Afterwards the camera reports
    /// another centre and zoom for the same view, so save and restore a camera together with its
    /// centerAltitude.
    void anchorCenterOnTerrain();

    /// Moves the camera's centre along the line of sight to `altitudeMeters` above sea level
    /// (exaggeration applied, as the terrain is drawn) without moving the camera: the centre and
    /// zoom change, nothing on screen does. anchorCenterOnTerrain() is this at the ground height;
    /// 0 puts the centre back at sea level. A centre and zoom describe the same view only at the
    /// same centre altitude (CameraOptions::centerAltitude); in core, jumpTo with a camera that
    /// includes its centerAltitude restores it in one step. Returns false, changing nothing, when
    /// the centre is already at that altitude, when the line of sight nearly grazes the plane at
    /// that altitude (a pitch over ~84 degrees) or meets it (nearly) at the camera, or when the
    /// zoom limits, the bounds or the constrain mode would cut the move short.
    bool setCenterAltitudeKeepingView(double altitudeMeters);

    /// Debug: when enabled, RenderTerrain logs the camera eye's clearance over the terrain
    /// ("ABOVE-GROUND ...") each frame it is near/below the surface. Off by default; the
    /// per-frame elevation sampling is skipped entirely when off, so it has no cost otherwise.
    void setDebugAboveGroundLog(bool enabled);
    bool getDebugAboveGroundLog() const;

    ClientOptions getClientOptions() const;

    const std::unique_ptr<util::ActionJournal>& getActionJournal();

    // If captureRenderedFeatures is true, these methods allow
    // access to the results of the most recently completed frame.

    using FeatureInfo = gfx::RenderingStats::FeatureInfo;
    using FrameRenderedFeaturesMap = gfx::RenderingStats::FrameRenderedFeaturesMap;
    const FrameRenderedFeaturesMap& getRenderedFeatures() const;

    /// Call the provided function for each matching feature rendered in the last frame
    void getRenderedFeatures(const std::optional<std::string>& featureId,
                             const std::optional<std::string>& layerId,
                             const std::optional<std::string>& sourceId,
                             const std::function<bool(const std::string&, const FeatureInfo&)>&) const;

    /// Returns the number of matching features rendered in the last frame.
    std::size_t getRenderedFeatureCount(const std::optional<std::string>& featureId = std::nullopt,
                                        const std::optional<std::string>& layerId = std::nullopt,
                                        const std::optional<std::string>& sourceId = std::nullopt,
                                        std::size_t limit = std::numeric_limits<std::size_t>::max()) const;

    /// Returns true if any matching feature was rendered in the last frame
    bool isFeatureRendered(const std::optional<std::string>& featureId = std::nullopt,
                           const std::optional<std::string>& layerId = std::nullopt,
                           const std::optional<std::string>& sourceId = std::nullopt) const {
        return 0 < getRenderedFeatureCount(featureId, layerId, sourceId, 1);
    }

protected:
    class Impl;
    const std::unique_ptr<Impl> impl;

    // For testing only.
    Map(std::unique_ptr<Impl>, const util::ActionJournalOptions& = {});
};

} // namespace mln
