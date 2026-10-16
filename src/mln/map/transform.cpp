#include <mln/map/camera.hpp>
#include <mln/map/transform.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/mat4.hpp>
#include <mln/util/math.hpp>
#include <mln/util/unitbezier.hpp>
#include <mln/util/interpolate.hpp>
#include <mln/util/chrono.hpp>
#include <mln/util/projection.hpp>
#include <mln/math/angles.hpp>
#include <mln/math/clamp.hpp>
#include <mln/util/logging.hpp>
#include <mln/util/platform.hpp>

#include <algorithm>
#include <cstdio>
#include <utility>
#include <numbers>

using namespace std::numbers;

namespace mln {

/** Converts the given angle (in radians) to be numerically close to the anchor
 * angle, allowing it to be interpolated properly without sudden jumps. */
namespace {
double _normalizeAngle(double angle, double anchorAngle) {
    if (std::isnan(angle) || std::isnan(anchorAngle)) {
        return 0;
    }

    angle = util::wrap(angle, -pi, pi);
    if (angle == -pi) angle = pi;
    double diff = std::abs(angle - anchorAngle);
    if (std::abs(angle - util::M2PI - anchorAngle) < diff) {
        angle -= util::M2PI;
    }
    if (std::abs(angle + util::M2PI - anchorAngle) < diff) {
        angle += util::M2PI;
    }

    return angle;
}
} // namespace

Transform::Transform(TransformObserver& observer_, ConstrainMode constrainMode, ViewportMode viewportMode)
    : observer(observer_),
      state(constrainMode, viewportMode) {}

// MARK: - Map View

void Transform::resize(const Size size) {
    if (size.isEmpty()) {
        throw std::runtime_error("failed to resize: size is empty");
    }

    if (state.getSize() == size) {
        return;
    }

    observer.onCameraWillChange(MapObserver::CameraChangeMode::Immediate);
    state.setSize(size);
    double scale{state.getScale()};
    double x{state.getX()};
    double y{state.getY()};
    double z{state.getZ()};

    double lat;
    double lon;
    if (state.constrainScreen(scale, lat, lon)) {
        // Turns out that if you resize during a transition any changes made to the state will be ignored :(
        // So if we have to constrain because of a resize and a transition is in progress - cancel the transition!
        if (inTransition()) {
            cancelTransitions();
        }

        // It also turns out that state.setProperties isn't enough if you change the center, you need to set Cc and Bc
        // too, which setLatLngZoom does.
        state.setLatLngZoom(mln::LatLng{lat, lon}, state.scaleZoom(scale));
        observer.onCameraDidChange(MapObserver::CameraChangeMode::Immediate);

        return;
    }
    state.constrain(scale, x, y);
    state.setProperties(TransformStateProperties().withScale(scale).withX(x).withY(y).withZ(z));

    observer.onCameraDidChange(MapObserver::CameraChangeMode::Immediate);
}

// MARK: - Camera

CameraOptions Transform::getCameraOptions(const std::optional<EdgeInsets>& padding) const {
    return state.getCameraOptions(padding);
}

/**
 * Change any combination of center, zoom, bearing, and pitch, without
 * a transition. Omitted options keep their current animations.
 */
void Transform::jumpTo(const CameraOptions& camera) {
    easeTo(camera);
}

/**
 * Change any combination of center, zoom, bearing, pitch and edgeInsets, with a
 * smooth animation between old and new values. Omitted options keep their
 * current animations and timing.
 */
void Transform::easeTo(const CameraOptions& inputCamera, const AnimationOptions& animation) {
    CameraOptions camera = inputCamera;

    Duration duration = animation.duration.value_or(Duration::zero());
    if (state.getLatLngBounds() == LatLngBounds() && !isGestureInProgress() && duration != Duration::zero()) {
        // reuse flyTo, without exaggerated animation, to achieve constant ground speed.
        flyTo(camera, animation, true);
        return;
    }

    double zoom = camera.zoom.value_or(getZoom());
    state.constrainCameraAndZoomToBounds(camera, zoom);

    const EdgeInsets& padding = camera.padding.value_or(state.getEdgeInsets());
    LatLng startLatLng = getLatLng(LatLng::Unwrapped);
    const LatLng& unwrappedLatLng = camera.center.value_or(startLatLng);
    const LatLng& latLng = state.getLatLngBounds() != LatLngBounds() ? unwrappedLatLng : unwrappedLatLng.wrapped();

    double bearing = camera.bearing ? util::deg2rad(-*camera.bearing) : getBearing();
    double pitch = camera.pitch ? util::deg2rad(*camera.pitch) : getPitch();
    double fov = camera.fov ? util::deg2rad(*camera.fov) : getFieldOfView();
    double centerAlt = camera.centerAltitude.value_or(state.getCenterAltitude());
    double roll = camera.roll ? util::deg2rad(*camera.roll) : getRoll();

    if (std::isnan(zoom) || std::isnan(bearing) || std::isnan(pitch) || std::isnan(roll) || std::isnan(fov)) {
        if (animation.transitionFinishFn) {
            animation.transitionFinishFn();
        }
        return;
    }

    if (state.getLatLngBounds() == LatLngBounds()) {
        if (isGestureInProgress()) {
            // If gesture in progress, we transfer the wrap rounds from the end
            // longitude into start, so the "scroll effect" of rounding the
            // world is the same while assuring the end longitude remains
            // wrapped.
            const double wrap = unwrappedLatLng.longitude() - latLng.longitude();
            startLatLng = LatLng(startLatLng.latitude(), startLatLng.longitude() - wrap);
        } else {
            // Find the shortest path otherwise.
            startLatLng.unwrapForShortestPath(latLng);
        }
    }
    const double startCenterAlt = state.getCenterAltitude();

    const Point<double> startPoint = Projection::project(startLatLng, state.getScale());
    const Point<double> endPoint = Projection::project(latLng, state.getScale());

    // Constrain camera options.
    zoom = util::clamp(zoom, state.getMinZoom(), state.getMaxZoom());
    pitch = util::clamp(pitch, state.getMinPitch(), state.getMaxPitch());
    fov = util::clamp(fov, state.getMinFieldOfView(), state.getMaxFieldOfView());

    // Minimize rotation by taking the shorter path around the circle.
    bearing = _normalizeAngle(bearing, state.getBearing());

    const double startZoom = state.getZoom();
    const double startBearing = _normalizeAngle(state.getBearing(), bearing);
    const double startPitch = state.getPitch();
    const double startRoll = state.getRoll();
    const double startFov = state.getFieldOfView();
    const EdgeInsets startEdgeInsets = state.getEdgeInsets();

    startTransition(
        inputCamera,
        animation,
        [=, this](double t) {
            Point<double> framePoint = util::interpolate(startPoint, endPoint, t);
            LatLng frameLatLng = Projection::unproject(framePoint, state.zoomScale(startZoom));
            double frameZoom = util::interpolate(startZoom, zoom, t);
            return CameraOptions()
                .withCenter(frameLatLng)
                .withZoom(frameZoom)
                .withCenterAltitude(util::interpolate(startCenterAlt, centerAlt, t))
                .withBearing(-util::rad2deg(util::wrap(util::interpolate(startBearing, bearing, t), -pi, pi)))
                .withPadding(EdgeInsets{util::interpolate(startEdgeInsets.top(), padding.top(), t),
                                        util::interpolate(startEdgeInsets.left(), padding.left(), t),
                                        util::interpolate(startEdgeInsets.bottom(), padding.bottom(), t),
                                        util::interpolate(startEdgeInsets.right(), padding.right(), t)})
                .withPitch(util::rad2deg(util::interpolate(startPitch, pitch, t)))
                .withRoll(util::rad2deg(util::interpolate(startRoll, roll, t)))
                .withFov(util::rad2deg(util::interpolate(startFov, fov, t)));
        },
        duration);
}

/** This method implements an “optimal path” animation, as detailed in:

    Van Wijk, Jarke J.; Nuij, Wim A. A. “Smooth and efficient zooming and
        panning.” INFOVIS ’03. pp. 15–22.
        <https://www.win.tue.nl/~vanwijk/zoompan.pdf#page=5>.

    Where applicable, local variable documentation begins with the associated
    variable or function in van Wijk (2003). */
void Transform::flyTo(const CameraOptions& inputCamera,
                      const AnimationOptions& animation,
                      bool linearZoomInterpolation) {
    CameraOptions camera = inputCamera;

    double zoom = camera.zoom.value_or(getZoom());
    state.constrainCameraAndZoomToBounds(camera, zoom);

    const EdgeInsets& padding = camera.padding.value_or(state.getEdgeInsets());
    const LatLng& latLng = camera.center.value_or(getLatLng(LatLng::Unwrapped)).wrapped();
    const double centerAlt = camera.centerAltitude.value_or(state.getCenterAltitude());
    double bearing = camera.bearing ? util::deg2rad(-*camera.bearing) : getBearing();
    double pitch = camera.pitch ? util::deg2rad(*camera.pitch) : getPitch();
    double roll = camera.roll ? util::deg2rad(*camera.roll) : getRoll();
    double fov = camera.fov ? util::deg2rad(*camera.fov) : getFieldOfView();

    if (std::isnan(zoom) || std::isnan(bearing) || std::isnan(pitch) || std::isnan(roll) || std::isnan(fov) ||
        state.getSize().isEmpty()) {
        if (animation.transitionFinishFn) {
            animation.transitionFinishFn();
        }
        return;
    }

    // Determine endpoints.
    LatLng startLatLng = getLatLng(LatLng::Unwrapped).wrapped();
    startLatLng.unwrapForShortestPath(latLng);
    const double startCenterAlt = state.getCenterAltitude();

    const Point<double> startPoint = Projection::project(startLatLng, state.getScale());
    const Point<double> endPoint = Projection::project(latLng, state.getScale());

    // Constrain camera options.
    zoom = util::clamp(zoom, state.getMinZoom(), state.getMaxZoom());
    pitch = util::clamp(pitch, state.getMinPitch(), state.getMaxPitch());
    fov = util::clamp(fov, state.getMinFieldOfView(), state.getMaxFieldOfView());

    // Minimize rotation by taking the shorter path around the circle.
    bearing = _normalizeAngle(bearing, state.getBearing());
    const double startZoom = state.scaleZoom(state.getScale());
    const double startBearing = _normalizeAngle(state.getBearing(), bearing);
    const double startPitch = state.getPitch();
    const double startRoll = state.getRoll();
    const double startFov = state.getFieldOfView();

    /// w₀: Initial visible span, measured in pixels at the initial scale.
    /// Known henceforth as a <i>screenful</i>.

    double w0 = std::max(state.getSize().width - padding.left() - padding.right(),
                         state.getSize().height - padding.top() - padding.bottom());
    /// w₁: Final visible span, measured in pixels with respect to the initial
    /// scale.
    double w1 = w0 / state.zoomScale(zoom - startZoom);
    /// Length of the flight path as projected onto the ground plane, measured
    /// in pixels from the world image origin at the initial scale.
    double u1 = ::hypot((endPoint - startPoint).x, (endPoint - startPoint).y);

    /** ρ: The relative amount of zooming that takes place along the flight
        path. A high value maximizes zooming for an exaggerated animation, while
        a low value minimizes zooming for something closer to easeTo().

        1.42 is the average value selected by participants in the user study in
        van Wijk (2003). A value of 6<sup>¼</sup> would be equivalent to the
        root mean squared average velocity, V<sub>RMS</sub>. A value of 1
        produces a circular motion. */
    double rho = 1.42;
    if (animation.minZoom || linearZoomInterpolation) {
        double minZoom = util::min(animation.minZoom.value_or(startZoom), startZoom, zoom);
        minZoom = util::clamp(minZoom, state.getMinZoom(), state.getMaxZoom());
        /// w<sub>m</sub>: Maximum visible span, measured in pixels with respect
        /// to the initial scale.
        double wMax = w0 / state.zoomScale(minZoom - startZoom);
        rho = u1 != 0 ? std::sqrt(wMax / u1 * 2) : 1.0;
    }
    /// ρ²
    double rho2 = rho * rho;

    /** rᵢ: Returns the zoom-out factor at one end of the animation.

        @param i 0 for the ascent or 1 for the descent. */
    auto r = [=](double i) {
        /// bᵢ
        double b = (w1 * w1 - w0 * w0 + (i ? -1 : 1) * rho2 * rho2 * u1 * u1) / (2 * (i ? w1 : w0) * rho2 * u1);
        return std::log(std::sqrt(b * b + 1) - b);
    };

    /// r₀: Zoom-out factor during ascent.
    double r0 = u1 != 0 ? r(0) : INFINITY; // Silence division by 0 on sanitize bot.
    double r1 = u1 != 0 ? r(1) : INFINITY;

    // When u₀ = u₁, the optimal path doesn’t require both ascent and descent.
    bool isClose = std::abs(u1) < 0.000001 || !std::isfinite(r0) || !std::isfinite(r1);

    /** w(s): Returns the visible span on the ground, measured in pixels with
        respect to the initial scale.

        Assumes an angular field of view of 2 arctan ½ ≈ 53°. */
    auto w = [=](double s) {
        return (isClose ? std::exp((w1 < w0 ? -1 : 1) * rho * s) : (std::cosh(r0) / std::cosh(r0 + rho * s)));
    };
    /// u(s): Returns the distance along the flight path as projected onto the
    /// ground plane, measured in pixels from the world image origin at the
    /// initial scale.
    auto u = [=](double s) {
        return (isClose ? 0. : (w0 * (std::cosh(r0) * std::tanh(r0 + rho * s) - std::sinh(r0)) / rho2 / u1));
    };
    /// S: Total length of the flight path, measured in ρ-screenfuls.
    double S = (isClose ? (std::abs(std::log(w1 / w0)) / rho) : ((r1 - r0) / rho));

    Duration duration;
    if (animation.duration) {
        duration = *animation.duration;
    } else {
        /// V: Average velocity, measured in ρ-screenfuls per second.
        double velocity = 1.2;
        if (animation.velocity) {
            velocity = *animation.velocity / rho;
        }
        duration = std::chrono::duration_cast<Duration>(std::chrono::duration<double>(S / velocity));
    }
    const double startScale = state.getScale();
    const EdgeInsets startEdgeInsets = state.getEdgeInsets();

    startTransition(
        inputCamera,
        animation,
        [=, this](double k) {
            /// s: The distance traveled along the flight path, measured in
            /// ρ-screenfuls.
            double s = k * S;
            double us = k == 1.0 ? 1.0 : u(s);

            // Calculate the current point and zoom level along the flight path.
            Point<double> framePoint = util::interpolate(startPoint, endPoint, us);
            double frameZoom = linearZoomInterpolation ? util::interpolate(startZoom, zoom, k)
                                                       : startZoom + state.scaleZoom(1 / w(s));

            // Zoom can be NaN if size is empty.
            if (std::isnan(frameZoom)) {
                frameZoom = zoom;
            }

            // Convert to geographic coordinates and set the new viewpoint.
            LatLng frameLatLng = Projection::unproject(framePoint, startScale);
            return CameraOptions()
                .withCenter(frameLatLng)
                .withZoom(frameZoom)
                .withCenterAltitude(util::interpolate(startCenterAlt, centerAlt, us))
                .withBearing(-util::rad2deg(util::wrap(util::interpolate(startBearing, bearing, k), -pi, pi)))
                .withPadding(EdgeInsets{util::interpolate(startEdgeInsets.top(), padding.top(), k),
                                        util::interpolate(startEdgeInsets.left(), padding.left(), k),
                                        util::interpolate(startEdgeInsets.bottom(), padding.bottom(), k),
                                        util::interpolate(startEdgeInsets.right(), padding.right(), k)})
                .withPitch(util::rad2deg(util::interpolate(startPitch, pitch, k)))
                .withRoll(util::rad2deg(util::interpolate(startRoll, roll, k)))
                .withFov(util::rad2deg(util::interpolate(startFov, fov, k)));
        },
        duration,
        !linearZoomInterpolation);
}

// MARK: - Position

void Transform::moveBy(const ScreenCoordinate& offset, const AnimationOptions& animation) {
    ScreenCoordinate centerOffset = {offset.x, offset.y};

    // Reduce the offset so that it never goes past the horizon. If it goes past
    // the horizon, the pan direction is opposite of the intended direction.
    const double pitch = state.getPitch();
    const double offsetLength = std::hypot(offset.x, offset.y);
    if (pitch > 0.0 && offsetLength > 0.0) {
        const double cameraToCenter = 0.5 * static_cast<double>(state.getSize().height) /
                                      std::tan(state.getFieldOfView() / 2.0);
        const double pixelsToHorizon = std::abs(cameraToCenter / std::tan(pitch));
        constexpr double horizonFactor = 0.75; // must be < 1 to keep the offset short of the horizon
        const double scale = pixelsToHorizon * horizonFactor / offsetLength;
        if (scale < 1.0) {
            centerOffset = {offset.x * scale, offset.y * scale};
        }
    }

    ScreenCoordinate pointOnScreen = state.getEdgeInsets().getCenter(state.getSize().width, state.getSize().height) -
                                     centerOffset;
    // Use unwrapped LatLng to carry information about moveBy direction.
    easeTo(CameraOptions().withCenter(screenCoordinateToLatLng(pointOnScreen, LatLng::Unwrapped)), animation);
}

LatLng Transform::getLatLng(LatLng::WrapMode wrap) const {
    return state.getLatLng(wrap);
}

// MARK: - Zoom

double Transform::getZoom() const {
    return state.getZoom();
}

// MARK: - Bounds

void Transform::setLatLngBounds(LatLngBounds bounds) {
    if (!bounds.valid()) {
        throw std::runtime_error("failed to set bounds: bounds are invalid");
    }
    state.setLatLngBounds(bounds);
}

void Transform::setMinZoom(const double minZoom) {
    if (std::isnan(minZoom)) return;
    state.setMinZoom(minZoom);
}

void Transform::setMaxZoom(const double maxZoom) {
    if (std::isnan(maxZoom)) return;
    state.setMaxZoom(maxZoom);
}

void Transform::setMinPitch(const double minPitch) {
    if (std::isnan(minPitch)) return;
    if (util::deg2rad(minPitch) < util::PITCH_MIN) {
        Log::Warning(Event::General,
                     "Trying to set minimum pitch below the limit (" + std::to_string(util::rad2deg(util::PITCH_MIN)) +
                         " degrees), the value will be clamped.");
    }
    state.setMinPitch(util::deg2rad(minPitch));
}

void Transform::setMaxPitch(const double maxPitch) {
    if (std::isnan(maxPitch)) return;
    if (util::deg2rad(maxPitch) > util::PITCH_MAX) {
        Log::Warning(Event::General,
                     "Trying to set maximum pitch above the limit (" + std::to_string(util::rad2deg(util::PITCH_MAX)) +
                         " degrees), the value will be clamped.");
    }
    state.setMaxPitch(util::deg2rad(maxPitch));
}

void Transform::setFrustumOffset(const EdgeInsets& frustumOffset) {
    state.setFrustumOffset(frustumOffset);
}

EdgeInsets Transform::getFrustumOffset() {
    return state.getFrustumOffset();
}

// MARK: - Bearing

void Transform::rotateBy(const ScreenCoordinate& first,
                         const ScreenCoordinate& second,
                         const AnimationOptions& animation) {
    ScreenCoordinate center = state.getEdgeInsets().getCenter(state.getSize().width, state.getSize().height);
    const ScreenCoordinate offset = first - center;
    const double distance = std::sqrt(std::pow(2, offset.x) + std::pow(2, offset.y));

    // If the first click was too close to the center, move the center of
    // rotation by 200 pixels in the direction of the click.
    if (distance < 200) {
        const double heightOffset = -200;
        const double rotateBearing = std::atan2(offset.y, offset.x);
        center.x = first.x + std::cos(rotateBearing) * heightOffset;
        center.y = first.y + std::sin(rotateBearing) * heightOffset;
    }

    const double bearing = -util::rad2deg(state.getBearing() + util::angle_between(first - center, second - center));
    easeTo(CameraOptions().withBearing(bearing), animation);
}

double Transform::getBearing() const {
    return state.getBearing();
}

// MARK: - Pitch

double Transform::getPitch() const {
    return state.getPitch();
}

double Transform::getRoll() const {
    return state.getRoll();
}

double Transform::getFieldOfView() const {
    return state.getFieldOfView();
}

// MARK: - North Orientation

void Transform::setNorthOrientation(NorthOrientation orientation) {
    state.setNorthOrientation(orientation);
    double scale{state.getScale()};
    double x{state.getX()};
    double y{state.getY()};
    double z{state.getZ()};
    state.constrain(scale, x, y);
    state.setProperties(TransformStateProperties().withScale(scale).withX(x).withY(y).withZ(z));
}

NorthOrientation Transform::getNorthOrientation() const {
    return state.getNorthOrientation();
}

// MARK: - Constrain mode

void Transform::setConstrainMode(mln::ConstrainMode mode) {
    state.setConstrainMode(mode);
    double scale{state.getScale()};
    double x{state.getX()};
    double y{state.getY()};
    double z{state.getZ()};
    state.constrain(scale, x, y);
    state.setProperties(TransformStateProperties().withScale(scale).withX(x).withY(y).withZ(z));
}

ConstrainMode Transform::getConstrainMode() const {
    return state.getConstrainMode();
}

// MARK: - Viewport mode

void Transform::setViewportMode(mln::ViewportMode mode) {
    state.setViewportMode(mode);
}

ViewportMode Transform::getViewportMode() const {
    return state.getViewportMode();
}

// MARK: - Projection mode

void Transform::setProjectionMode(const ProjectionMode& options) {
    state.setProperties(TransformStateProperties()
                            .withAxonometric(options.axonometric.value_or(state.getAxonometric()))
                            .withXSkew(options.xSkew.value_or(state.getXSkew()))
                            .withYSkew(options.ySkew.value_or(state.getYSkew())));
}

ProjectionMode Transform::getProjectionMode() const {
    return ProjectionMode()
        .withAxonometric(state.getAxonometric())
        .withXSkew(state.getXSkew())
        .withYSkew(state.getYSkew());
}

// MARK: - Transition

uint16_t Transform::cameraFields(const CameraOptions& camera) {
    return (camera.center ? Center : 0) | (camera.zoom ? Zoom : 0) | (camera.bearing ? Bearing : 0) |
           (camera.pitch ? Pitch : 0) | (camera.padding ? Padding : 0) | (camera.centerAltitude ? Altitude : 0) |
           (camera.roll ? Roll : 0) | (camera.fov ? Fov : 0);
}

void Transform::startTransition(const CameraOptions& camera,
                                const AnimationOptions& animation,
                                const std::function<CameraOptions(double)>& frame,
                                const Duration& duration,
                                bool flight) {
    auto transition = std::make_shared<Transition>(animation);
    transition->fields = cameraFields(camera);
    transition->coupledFields = flight ? Center | Zoom : 0;
    transition->fields |= transition->coupledFields;

    // An anchor controls center together with the properties it transforms.
    // An explicit center takes precedence over an anchor.
    transition->anchor = camera.center ? std::nullopt : camera.anchor;
    if (transition->anchor) {
        transition->fields |= Center;
        transition->coupledFields = transition->fields;
        transition->anchor->y = state.getSize().height - transition->anchor->y;
        transition->anchorLatLng = state.screenCoordinateToLatLng(*transition->anchor);
    }

    const auto target = frame(1.0);
    const bool panning = flight || transition->anchor || camera.center != std::optional(getLatLng(LatLng::Unwrapped));
    const bool scaling = flight || target.zoom != std::optional(getZoom());
    const bool rotating = target.bearing != std::optional(-util::rad2deg(getBearing()));
    transition->movingFields = (panning ? Center : 0) | (scaling ? Zoom : 0) | (rotating ? Bearing : 0);

    transition->start = latestTransitionStart = Clock::now();
    transition->duration = latestTransitionDuration = transition->fields ? duration : Duration::zero();
    transition->frame = frame;

    std::vector<std::shared_ptr<Transition>> finished;
    for (auto& active : transitions) {
        auto replaced = transition->fields;
        if (active->coupledFields & replaced) replaced |= active->coupledFields;
        active->fields &= ~replaced;
        if (!active->fields) finished.push_back(active);
    }
    std::erase_if(transitions, [](const auto& active) { return !active->fields; });
    transitions.push_back(transition);
    updateMovementFlags();
    finishTransitions(finished);

    // Finish callbacks may replace or cancel the newly installed command.
    if (std::find(transitions.begin(), transitions.end(), transition) == transitions.end()) return;
    const bool animated = transition->duration != Duration::zero();
    observer.onCameraWillChange(animated ? MapObserver::CameraChangeMode::Animated
                                         : MapObserver::CameraChangeMode::Immediate);
    if (std::find(transitions.begin(), transitions.end(), transition) == transitions.end()) return;

    if (!animated) {
        applyTransitions({transition}, transition->start);
        std::erase(transitions, transition);
        updateMovementFlags();
        finishTransitions({transition});
    }
}

void Transform::applyTransitions(const std::vector<std::shared_ptr<Transition>>& active, const TimePoint& now) {
    auto camera = state.getCameraOptions(std::nullopt);
    camera.center = getLatLng(LatLng::Unwrapped);

    for (const auto& transition : active) {
        const double t = transition->duration == Duration::zero()
                             ? 1.0
                             : std::clamp(std::chrono::duration<double>(now - transition->start) /
                                              std::chrono::duration<double>(transition->duration),
                                          0.0,
                                          1.0);
        auto ease = transition->animation.easing.value_or(util::DEFAULT_TRANSITION_EASE);
        const auto value = transition->frame(t == 1.0 ? 1.0 : ease.solve(t, 0.001));
        const auto fields = transition->fields;
        if (fields & Center) camera.center = value.center;
        if (fields & Zoom) camera.zoom = value.zoom;
        if (fields & Bearing) camera.bearing = value.bearing;
        if (fields & Pitch) camera.pitch = value.pitch;
        if (fields & Padding) camera.padding = value.padding;
        if (fields & Altitude) camera.centerAltitude = value.centerAltitude;
        if (fields & Roll) camera.roll = value.roll;
        if (fields & Fov) camera.fov = value.fov;
    }

    // An active anchor also applies to immediate changes of other properties.
    std::shared_ptr<Transition> anchored;
    for (const auto& transition : transitions) {
        if (transition->anchor && (transition->fields & Center)) anchored = transition;
    }

    // Compose the camera before applying constraints. In particular, padding
    // and field of view determine the pitch limit, and center and zoom jointly
    // determine the geographic bounds and altitude scale.
    state.setEdgeInsets(*camera.padding);
    state.setFieldOfView(util::deg2rad(*camera.fov));
    state.setBearing(util::deg2rad(-*camera.bearing));
    state.setRoll(util::deg2rad(*camera.roll));
    state.setPitch(std::min(getMaxPitchForEdgeInsets(*camera.padding), util::deg2rad(*camera.pitch)));
    state.constrainCameraAndZoomToBounds(camera, *camera.zoom);
    state.setLatLngZoom(*camera.center, *camera.zoom);
    if (anchored) state.moveLatLng(anchored->anchorLatLng, *anchored->anchor);
    state.setCenterAltitude(*camera.centerAltitude);
}

void Transform::updateMovementFlags() {
    uint16_t moving = 0;
    for (const auto& transition : transitions) moving |= transition->fields & transition->movingFields;
    state.setProperties(TransformStateProperties()
                            .withPanningInProgress(moving & Center)
                            .withScalingInProgress(moving & Zoom)
                            .withRotatingInProgress(moving & Bearing));
}

void Transform::finishTransitions(const std::vector<std::shared_ptr<Transition>>& finished) {
    for (const auto& transition : finished) {
        if (transition->animation.transitionFinishFn) transition->animation.transitionFinishFn();
        // This notification ends a command, not necessarily all camera motion.
        observer.onCameraDidChange(transition->duration != Duration::zero() ? MapObserver::CameraChangeMode::Animated
                                                                            : MapObserver::CameraChangeMode::Immediate);
    }
}

bool Transform::inTransition() const {
    return !transitions.empty();
}

void Transform::updateTransitions(const TimePoint& now) {
    // Observers can synchronously update the map or start another command.
    // Snapshot the commands for this frame, and detach completed commands
    // before calling user code so each command finishes exactly once.
    if (updatingTransitions || transitions.empty()) return;
    updatingTransitions = true;
    struct TransitionUpdateGuard {
        bool& updating;
        ~TransitionUpdateGuard() { updating = false; }
    } guard{updatingTransitions};

    const auto active = transitions;
    applyTransitions(active, now);

    std::vector<std::shared_ptr<Transition>> finished;
    for (const auto& transition : active) {
        if (now - transition->start >= transition->duration) {
            std::erase(transitions, transition);
            finished.push_back(transition);
        }
    }
    updateMovementFlags();

    for (const auto& transition : active) {
        if (std::find(transitions.begin(), transitions.end(), transition) == transitions.end()) continue;
        if (transition->animation.transitionFrameFn) {
            const double t = std::clamp(std::chrono::duration<double>(now - transition->start) /
                                            std::chrono::duration<double>(transition->duration),
                                        0.0,
                                        1.0);
            transition->animation.transitionFrameFn(t);
        }
    }
    if (!transitions.empty()) observer.onCameraIsChanging();

    finishTransitions(finished);
}

void Transform::cancelTransitions() {
    auto finished = std::exchange(transitions, {});
    updateMovementFlags();
    finishTransitions(finished);
}

void Transform::setGestureInProgress(bool inProgress) {
    state.setGestureInProgress(inProgress);
}

// MARK: Conversion and projection

ScreenCoordinate Transform::latLngToScreenCoordinate(const LatLng& latLng) const {
    ScreenCoordinate point = state.latLngToScreenCoordinate(latLng);
    point.y = state.getSize().height - point.y;
    return point;
}

LatLng Transform::screenCoordinateToLatLng(const ScreenCoordinate& point, LatLng::WrapMode wrapMode) const {
    ScreenCoordinate flippedPoint = point;
    flippedPoint.y = state.getSize().height - flippedPoint.y;
    return state.screenCoordinateToLatLng(flippedPoint, wrapMode);
}

double Transform::getMaxPitchForEdgeInsets(const EdgeInsets& insets) const {
    double centerOffsetY = 0.5 * (insets.top() - insets.bottom()); // See TransformState::getCenterOffset.
    if (centerOffsetY == 0.0) {
        return state.getMaxPitch();
    }

    const auto height = state.getSize().height;
    assert(height);
    // Half of fov is the field of view above perspective center.
    const double tangentOfFovAboveCenterAngle = (0.5 + centerOffsetY / height) * 2.0 * tan(getFieldOfView() / 2.0);
    const double fovAboveCenter = std::atan(tangentOfFovAboveCenterAngle);
    return std::max(state.getMinPitch(), state.getMaxPitch() + getFieldOfView() / 2.0 - fovAboveCenter);
}

FreeCameraOptions Transform::getFreeCameraOptions() const {
    return state.getFreeCameraOptions();
}

void Transform::setFreeCameraOptions(const FreeCameraOptions& options) {
    cancelTransitions();
    state.setFreeCameraOptions(options);
}

} // namespace mln
