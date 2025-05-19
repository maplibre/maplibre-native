#include <mbgl/map/transform_active.hpp>
#include <mbgl/map/transform_state.hpp>
#include <mbgl/util/interpolate.hpp>
#include <mbgl/util/logging.hpp>

using namespace std::numbers;

namespace mbgl {

void TransformActive::easeTo(const CameraOptions& inputCamera, const AnimationOptions& animation) {
    CameraOptions camera = inputCamera;

    Duration duration = animation.duration.value_or(Duration::zero());
    if (state.getLatLngBounds() == LatLngBounds() && !isGestureInProgress() && duration != Duration::zero()) {
        // reuse flyTo, without exaggerated animation, to achieve constant ground speed.
        return flyTo(camera, animation, true);
    }

    double zoom = camera.zoom.value_or(getZoom());
    state.constrainCameraAndZoomToBounds(camera, zoom);

    const EdgeInsets& padding = camera.padding.value_or(state.getEdgeInsets());
    LatLng startLatLng = getLatLng(LatLng::Unwrapped);
    const LatLng& unwrappedLatLng = camera.center.value_or(startLatLng);
    const LatLng& latLng = state.getLatLngBounds() != LatLngBounds() ? unwrappedLatLng : unwrappedLatLng.wrapped();

    double bearing = camera.bearing ? util::deg2rad(-*camera.bearing) : getBearing();
    double pitch = camera.pitch ? util::deg2rad(*camera.pitch) : getPitch();

    if (std::isnan(zoom) || std::isnan(bearing) || std::isnan(pitch)) {
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

    const Point<double> startPoint = Projection::project(startLatLng, state.getScale());
    const Point<double> endPoint = Projection::project(latLng, state.getScale());

    // Constrain camera options.
    zoom = util::clamp(zoom, state.getMinZoom(), state.getMaxZoom());
    pitch = util::clamp(pitch, state.getMinPitch(), state.getMaxPitch());

    // Minimize rotation by taking the shorter path around the circle.
    bearing = normalizeAngle(bearing, state.getBearing());
    state.setBearing(normalizeAngle(state.getBearing(), bearing));

    const double startZoom = state.getZoom();
    const double startBearing = state.getBearing();
    const double startPitch = state.getPitch();
    const EdgeInsets startEdgeInsets = state.getEdgeInsets();

    auto pa = std::make_shared<PropertyAnimation>(
        Clock::now(), duration, animation, unwrappedLatLng != startLatLng, zoom != startZoom, bearing != startBearing);

    // NOTE: For tests only
    transitionStart = pa->start;
    transitionDuration = pa->duration;

    if (!pas.zoom.set || startZoom != zoom) {
        animationFinishFrame(pas.zoom.pa);
        pas.zoom = {
            .pa = pa,
            .current = startZoom,
            .target = zoom,
            .set = true,
            .frameZoomFunc =
                [=, this](TimePoint now) {
                    return util::interpolate(pas.zoom.current, pas.zoom.target, pas.zoom.pa->t(now));
                },
        };
    }
    if (!pas.latlng.set || startPoint != endPoint) {
        animationFinishFrame(pas.latlng.pa);
        pas.latlng = {
            .pa = pa,
            .current = startPoint,
            .target = endPoint,
            .set = true,
            .frameLatLngFunc =
                [=, this](TimePoint now) {
                    Point<double> framePoint = util::interpolate(
                        pas.latlng.current, pas.latlng.target, pas.latlng.pa->t(now));
                    return Projection::unproject(framePoint, state.zoomScale(startZoom));
                },
        };
    }
    if (!pas.bearing.set || bearing != startBearing) {
        animationFinishFrame(pas.bearing.pa);
        pas.bearing = {
            .pa = pa,
            .current = startBearing,
            .target = bearing,
            .set = true,
        };
    }
    if (!pas.padding.set || padding != startEdgeInsets) {
        animationFinishFrame(pas.padding.pa);
        pas.padding = {
            .pa = pa,
            .current = startEdgeInsets,
            .target = padding,
            .set = true,
        };
    }
    if (!pas.pitch.set || pitch != startPitch) {
        animationFinishFrame(pas.pitch.pa);
        pas.pitch = {
            .pa = pa,
            .current = startPitch,
            .target = pitch,
            .set = true,
        };
    }

    startTransition(camera, duration);
}

void TransformActive::flyTo(const CameraOptions& inputCamera,
                            const AnimationOptions& animation,
                            bool linearZoomInterpolation) {
    CameraOptions camera = inputCamera;

    double zoom = camera.zoom.value_or(getZoom());
    state.constrainCameraAndZoomToBounds(camera, zoom);

    const EdgeInsets& padding = camera.padding.value_or(state.getEdgeInsets());
    const LatLng& latLng = camera.center.value_or(getLatLng(LatLng::Unwrapped)).wrapped();

    double bearing = camera.bearing ? util::deg2rad(-*camera.bearing) : getBearing();
    double pitch = camera.pitch ? util::deg2rad(*camera.pitch) : getPitch();

    if (std::isnan(zoom) || std::isnan(bearing) || std::isnan(pitch) || state.getSize().isEmpty()) {
        if (animation.transitionFinishFn) {
            animation.transitionFinishFn();
        }
        return;
    }

    // Determine endpoints.
    LatLng startLatLng = getLatLng(LatLng::Unwrapped).wrapped();
    startLatLng.unwrapForShortestPath(latLng);

    const Point<double> startPoint = Projection::project(startLatLng, state.getScale());
    const Point<double> endPoint = Projection::project(latLng, state.getScale());

    // Constrain camera options.
    zoom = util::clamp(zoom, state.getMinZoom(), state.getMaxZoom());
    pitch = util::clamp(pitch, state.getMinPitch(), state.getMaxPitch());

    // Minimize rotation by taking the shorter path around the circle.
    bearing = normalizeAngle(bearing, state.getBearing());
    state.setBearing(normalizeAngle(state.getBearing(), bearing));
    const double startZoom = state.scaleZoom(state.getScale());
    const double startBearing = state.getBearing();
    const double startPitch = state.getPitch();

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
    if (duration == Duration::zero()) {
        // Perform an instantaneous transition.
        jumpTo(camera);
        if (animation.transitionFinishFn) {
            animation.transitionFinishFn();
        }
        return;
    }

    const double startScale = state.getScale();
    const EdgeInsets startEdgeInsets = state.getEdgeInsets();

    auto pa = std::make_shared<PropertyAnimation>(
        Clock::now(), duration, animation, true, true, bearing != startBearing);

    // NOTE: For tests only
    transitionStart = pa->start;
    transitionDuration = pa->duration;

    if (!pas.zoom.set || startZoom != zoom) {
        animationFinishFrame(pas.zoom.pa);
        pas.zoom = {
            .pa = pa,
            .current = startZoom,
            .target = zoom,
            .set = true,
            .frameZoomFunc =
                [=, this](TimePoint now) {
                    double t = pas.zoom.pa->t(now);
                    double s = t * S;
                    double frameZoom = linearZoomInterpolation ? util::interpolate(pas.zoom.current, pas.zoom.target, t)
                                                               : pas.zoom.current + state.scaleZoom(1 / w(s));

                    if (std::isnan(frameZoom)) {
                        frameZoom = pas.zoom.target;
                    }

                    return frameZoom;
                },
        };
    }
    if (!pas.latlng.set || startPoint != endPoint) {
        animationFinishFrame(pas.latlng.pa);
        pas.latlng = {
            .pa = pa,
            .current = startPoint,
            .target = endPoint,
            .set = true,
            .frameLatLngFunc =
                [=, this](TimePoint now) {
                    double t = pas.latlng.pa->t(now);
                    double s = t * S;
                    double us = t == 1.0 ? 1.0 : u(s);

                    Point<double> framePoint = util::interpolate(pas.latlng.current, pas.latlng.target, us);
                    return Projection::unproject(framePoint, startScale);
                },
        };
    }
    if (!pas.bearing.set || bearing != startBearing) {
        animationFinishFrame(pas.bearing.pa);
        pas.bearing = {
            .pa = pa,
            .current = startBearing,
            .target = bearing,
            .set = true,
        };
    }
    if (!pas.padding.set || padding != startEdgeInsets) {
        animationFinishFrame(pas.padding.pa);
        pas.padding = {.pa = pa, .current = startEdgeInsets, .target = padding, .set = true};
    }
    if (!pas.pitch.set || pitch != startPitch) {
        animationFinishFrame(pas.pitch.pa);
        pas.pitch = {
            .pa = pa,
            .current = startPitch,
            .target = pitch,
            .set = true,
        };
    }

    startTransition(camera, duration);
}

bool TransformActive::animationTransitionFrame(std::shared_ptr<PropertyAnimation>& pa, double t) {
    if (pa->ran) {
        return pa->done;
    }

    pa->ran = true;
    if (t < 1.0) {
        if (pa->animation.transitionFrameFn) {
            pa->animation.transitionFrameFn(t);
        }

        observer.onCameraIsChanging();
        pa->done = false;
    } else {
        pa->done = true;
    }

    return pa->done;
}

void TransformActive::animationFinishFrame(std::shared_ptr<PropertyAnimation>& pa) {
    if (!pa || pa->finished) {
        return;
    }

    if (pa->animation.transitionFinishFn) {
        pa->animation.transitionFinishFn();
    }

    pa->finished = true;

    observer.onCameraDidChange(pa->isAnimated() ? MapObserver::CameraChangeMode::Animated
                                                : MapObserver::CameraChangeMode::Immediate);
}

void TransformActive::startTransition(const CameraOptions& camera, const Duration& duration) {
    bool isAnimated = duration != Duration::zero();
    observer.onCameraWillChange(isAnimated ? MapObserver::CameraChangeMode::Animated
                                           : MapObserver::CameraChangeMode::Immediate);

    // Associate the anchor, if given, with a coordinate.
    // Anchor and center points are mutually exclusive, with preference for the
    // center point when both are set.
    pas.anchor = camera.center ? std::nullopt : camera.anchor;
    if (pas.anchor) {
        pas.anchor->y = state.getSize().height - pas.anchor->y;
        pas.anchorLatLng = state.screenCoordinateToLatLng(*pas.anchor);
    }

    if (!isAnimated) {
        activeAnimation = false;
        updateTransitions(Clock::now());
    }
}

bool TransformActive::inTransition() const {
    return pas.latlng.set || pas.zoom.set || pas.bearing.set || pas.padding.set || pas.pitch.set;
}

void TransformActive::updateTransitions(const TimePoint& now) {
    if (!activeAnimation) {
        activeAnimation = true;

        if (pas.latlng.frameLatLngFunc && pas.zoom.frameZoomFunc) {
            if (pas.latlng.set || pas.zoom.set) {
                state.setLatLngZoom(pas.latlng.frameLatLngFunc(now), pas.zoom.frameZoomFunc(now));
                if (animationTransitionFrame(pas.latlng.pa, pas.latlng.pa->t(now))) {
                    pas.latlng.set = false;
                }
                if (animationTransitionFrame(pas.zoom.pa, pas.zoom.pa->t(now))) {
                    pas.zoom.set = false;
                }
            }
        }

        if (pas.bearing.set) {
            double bearing_t = pas.bearing.pa->t(now);
            state.setBearing(
                util::wrap(util::interpolate(pas.bearing.current, pas.bearing.target, bearing_t), -pi, pi));
            if (animationTransitionFrame(pas.bearing.pa, bearing_t)) {
                pas.bearing.set = false;
            }
        }

        if (pas.padding.set) {
            double padding_t = pas.padding.pa->t(now);
            state.setEdgeInsets(
                {util::interpolate(pas.padding.current.top(), pas.padding.target.top(), padding_t),
                 util::interpolate(pas.padding.current.left(), pas.padding.target.left(), padding_t),
                 util::interpolate(pas.padding.current.bottom(), pas.padding.target.bottom(), padding_t),
                 util::interpolate(pas.padding.current.right(), pas.padding.target.right(), padding_t)});
            if (animationTransitionFrame(pas.padding.pa, padding_t)) {
                pas.padding.set = false;
            }
        }

        double maxPitch = getMaxPitchForEdgeInsets(state.getEdgeInsets());
        if (pas.pitch.set || maxPitch < pas.pitch.current) {
            double pitch_t = pas.pitch.pa->t(now);
            state.setPitch(std::min(maxPitch, util::interpolate(pas.pitch.current, pas.pitch.target, pitch_t)));
            if (animationTransitionFrame(pas.pitch.pa, pitch_t)) {
                pas.pitch.set = false;
            }
        }

        if (pas.anchor) {
            state.moveLatLng(pas.anchorLatLng, *pas.anchor);
        }

        bool panning = false, scaling = false, rotating = false;
        visit_pas([&](std::shared_ptr<PropertyAnimation>& pa) {
            if (pa) {
                if (pa->done) animationFinishFrame(pa);
                panning |= pa->panning;
                scaling |= pa->scaling;
                rotating |= pa->rotating;
                pa->ran = false;
            }
        });

        state.setProperties(TransformStateProperties()
                                .withPanningInProgress(panning)
                                .withScalingInProgress(scaling)
                                .withRotatingInProgress(rotating));

        activeAnimation = false;
    }
}

void TransformActive::cancelTransitions() {
    visit_pas([this](std::shared_ptr<PropertyAnimation>& pa) { animationFinishFrame(pa); });

    pas = {};
    activeAnimation = false;
}

} // namespace mbgl
