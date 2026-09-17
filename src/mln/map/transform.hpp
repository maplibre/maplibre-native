#pragma once

#include <mln/map/camera.hpp>
#include <mln/map/projection_mode.hpp>
#include <mln/map/map_observer.hpp>
#include <mln/map/mode.hpp>
#include <mln/map/transform_state.hpp>
#include <mln/util/chrono.hpp>
#include <mln/util/geo.hpp>
#include <mln/util/noncopyable.hpp>

#include <cstdint>
#include <cmath>
#include <functional>
#include <optional>
#include <memory>
#include <vector>

namespace mln {

class TransformObserver {
public:
    virtual ~TransformObserver() = default;

    static TransformObserver& nullObserver() {
        static TransformObserver observer;
        return observer;
    }

    virtual void onCameraWillChange(MapObserver::CameraChangeMode) {}
    virtual void onCameraIsChanging() {}
    virtual void onCameraDidChange(MapObserver::CameraChangeMode) {}
};

class Transform : private util::noncopyable {
public:
    Transform(TransformObserver& = TransformObserver::nullObserver(),
              ConstrainMode = ConstrainMode::HeightOnly,
              ViewportMode = ViewportMode::Default);

    Transform(const TransformState& state_)
        : observer(TransformObserver::nullObserver()),
          state(state_) {}

    // Map view
    void resize(Size size);

    // Camera
    /** Returns the current camera options. */
    CameraOptions getCameraOptions(const std::optional<EdgeInsets>&) const;

    /** Instantaneously, synchronously applies the given camera options. */
    void jumpTo(const CameraOptions&);
    /** Asynchronously transitions all specified camera options linearly along
        an optional time curve. However, center coordinate is not transitioned
        linearly as, instead, ground speed is kept linear.*/
    void easeTo(const CameraOptions&, const AnimationOptions& = {});
    /** Asynchronously zooms out, pans, and zooms back into the given camera
        along a great circle, as though the viewer is riding a supersonic
        jetcopter.
        Parameter linearZoomInterpolation: when true, there is no additional
        zooming out as zoom is linearly interpolated from current to given
        camera zoom. This is used for easeTo.*/
    void flyTo(const CameraOptions&, const AnimationOptions& = {}, bool linearZoomInterpolation = false);

    // Position

    /** Pans the map by the given amount.
        @param offset The distance to pan the map by, measured in pixels from
            top to bottom and from left to right. */
    void moveBy(const ScreenCoordinate& offset, const AnimationOptions& = {});
    LatLng getLatLng(LatLng::WrapMode = LatLng::Wrapped) const;

    // Bounds

    void setLatLngBounds(LatLngBounds);
    void setMinZoom(double);
    void setMaxZoom(double);

    void setMinPitch(double);
    void setMaxPitch(double);

    // Zoom

    /** Returns the zoom level. */
    double getZoom() const;

    // Bearing

    void rotateBy(const ScreenCoordinate& first, const ScreenCoordinate& second, const AnimationOptions& = {});
    double getBearing() const;

    // Pitch

    double getPitch() const;
    double getRoll() const;
    double getFieldOfView() const;

    // North Orientation
    void setNorthOrientation(NorthOrientation);
    NorthOrientation getNorthOrientation() const;

    // Constrain mode
    void setConstrainMode(ConstrainMode);
    ConstrainMode getConstrainMode() const;

    // Viewport mode
    void setViewportMode(ViewportMode);
    ViewportMode getViewportMode() const;

    // Projection mode
    void setProjectionMode(const ProjectionMode&);
    ProjectionMode getProjectionMode() const;

    // Transitions
    bool inTransition() const;
    void updateTransitions(const TimePoint& now);
    // Timing of the most recently started command, even after it finishes.
    // Other commands may still be active.
    TimePoint getTransitionStart() const { return transitionStart; }
    Duration getTransitionDuration() const { return transitionDuration; }
    void cancelTransitions();

    // Gesture
    void setGestureInProgress(bool);
    bool isGestureInProgress() const { return state.isGestureInProgress(); }

    // Transform state
    const TransformState& getState() const { return state; }
    bool isRotating() const { return state.isRotating(); }
    bool isScaling() const { return state.isScaling(); }
    bool isPanning() const { return state.isPanning(); }

    // Conversion and projection
    ScreenCoordinate latLngToScreenCoordinate(const LatLng&) const;
    LatLng screenCoordinateToLatLng(const ScreenCoordinate&, LatLng::WrapMode = LatLng::Wrapped) const;

    FreeCameraOptions getFreeCameraOptions() const;
    void setFreeCameraOptions(const FreeCameraOptions& options);

    // Frustum
    void setFrustumOffset(const EdgeInsets&);
    EdgeInsets getFrustumOffset();

private:
    TransformObserver& observer;
    TransformState state;

    void startTransition(const CameraOptions&,
                         const AnimationOptions&,
                         const std::function<CameraOptions(double)>&,
                         const Duration&,
                         bool flight = false);

    enum CameraField : uint16_t {
        Center = 1 << 0,
        Zoom = 1 << 1,
        Bearing = 1 << 2,
        Pitch = 1 << 3,
        Padding = 1 << 4,
        Altitude = 1 << 5,
        Roll = 1 << 6,
        Fov = 1 << 7
    };
    struct Transition {
        explicit Transition(const AnimationOptions& options)
            : animation(options) {}
        // Remaining property ownership; replacement removes fields from this mask.
        uint16_t fields;
        // Replacing any coupled field relinquishes all coupled fields together.
        uint16_t coupledFields;
        // Properties classified as moving at command start; intersect with fields
        // when reporting movement so replaced properties no longer contribute.
        uint16_t movingFields;
        TimePoint start;
        Duration duration;
        AnimationOptions animation;
        std::function<CameraOptions(double)> frame;
        std::optional<ScreenCoordinate> anchor;
        LatLng anchorLatLng;
    };
    static uint16_t cameraFields(const CameraOptions&);
    void applyTransitions(const std::vector<std::shared_ptr<Transition>>&, const TimePoint&);
    void updateMovementFlags();
    void finishTransitions(const std::vector<std::shared_ptr<Transition>>&);

    // We don't want to show horizon: limit max pitch based on edge insets.
    double getMaxPitchForEdgeInsets(const EdgeInsets& insets) const;

    TimePoint transitionStart;
    Duration transitionDuration;
    std::vector<std::shared_ptr<Transition>> transitions;
    bool updatingTransitions = false;
};

} // namespace mln
