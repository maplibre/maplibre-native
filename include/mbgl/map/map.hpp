#pragma once

#include <mbgl/util/chrono.hpp>
#include <mbgl/map/bound_options.hpp>
#include <mbgl/map/map_observer.hpp>
#include <mbgl/map/map_options.hpp>
#include <mbgl/map/mode.hpp>
#include <mbgl/util/noncopyable.hpp>
#include <mbgl/util/size.hpp>
#include <mbgl/annotation/annotation.hpp>
#include <mbgl/map/camera.hpp>
#include <mbgl/util/geometry.hpp>
#include <mbgl/map/projection_mode.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/util/client_options.hpp>
#include <mbgl/util/action_journal_options.hpp>

#include <cstdint>
#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <optional>

namespace mbgl {

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
    LatLng latLngForPixel(const ScreenCoordinate&) const;
    std::vector<ScreenCoordinate> pixelsForLatLngs(const std::vector<LatLng>&) const;
    std::vector<LatLng> latLngsForPixels(const std::vector<ScreenCoordinate>&) const;

    // Transform
    TransformState getTransfromState() const;

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
    void setTileLodMinRadius(double radius);
    double getTileLodMinRadius() const;
    void setTileLodScale(double scale);
    double getTileLodScale() const;
    void setTileLodPitchThreshold(double threshold);
    double getTileLodPitchThreshold() const;
    void setTileLodZoomShift(double shift);
    double getTileLodZoomShift() const;

    ClientOptions getClientOptions() const;

    const std::unique_ptr<util::ActionJournal>& getActionJournal();

protected:
    class Impl;
    const std::unique_ptr<Impl> impl;

    // For testing only.
    Map(std::unique_ptr<Impl>, const util::ActionJournalOptions& = {});
};

} // namespace mbgl
