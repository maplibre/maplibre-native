#pragma once

#include <mbgl/map/transform.hpp>

namespace mbgl {

template <class T>
struct TransformProperty {
    std::shared_ptr<PropertyAnimation> pa;
    T current, target;
    bool set = false;

    std::function<LatLng(TimePoint)> frameLatLngFunc = nullptr;
    std::function<double(TimePoint)> frameZoomFunc = nullptr;

    // Anchor
    std::optional<ScreenCoordinate> anchor = std::nullopt;
    LatLng anchorLatLng = {};
};

struct PropertyAnimations {
    TransformProperty<Point<double>> latlng;
    TransformProperty<double> zoom, bearing, pitch;
    TransformProperty<EdgeInsets> padding;

    // Anchor
    std::optional<ScreenCoordinate> anchor;
    LatLng anchorLatLng;
};

/*
 `TransformActive` implements the same interface that `Transform`
 implements but overrides how transformations happen to allow
 for concurrent independent transformations to happen. Previously,
 any new transformation overrides the previous transformation operation.
 `TransformActive`, on the other hand, makes it so that two independetn
 transformations such as translation and zoom can happen at the same
 time without canceling the first one.

 The goal eventually is to replace the `Transform` implementation with
 this one since currently they both produce the same results but to avoid
 any potential issues that might arise we keep both implementation and allow
 switching between them through the `Map::toggleTransform` function for
 testing and to ensure we haven't overlooked any potential issues.
*/
class TransformActive : public Transform {
public:
    using Transform::Transform;

    void easeTo(const CameraOptions&, const AnimationOptions& = {}) override;
    void flyTo(const CameraOptions&, const AnimationOptions& = {}, bool linearZoomInterpolation = false) override;

    bool inTransition() const override;
    void updateTransitions(const TimePoint&) override;
    void cancelTransitions() override;

private:
    void startTransition(const CameraOptions&,
                         const AnimationOptions&,
                         const std::function<void(double)>&,
                         const Duration&) override {
        assert(false);
    };

    struct Animation {
        const TimePoint start;
        const Duration duration;
        const AnimationOptions options;
        bool ran = false;      // Did this property animation run this frame
        bool finished = false; // Did we execute the finish frame for this property animation this frame
        bool done = false;     // Did this property animation reach the end of the frame
        // The below variables keep track of the panning, scaling, and rotating transform state
        // so we can correctly set it at the end of the `updateTransitions` if more
        // than one `Animation` is running at the same time.
        const bool panning;
        const bool scaling;
        const bool rotating;

        // Anchor
        std::optional<ScreenCoordinate> anchor;
        LatLng anchorLatLng;

        Animation(TimePoint start_,
                  Duration duration_,
                  AnimationOptions options_,
                  bool panning_,
                  bool scaling_,
                  bool rotating_)
            : start(start_),
              duration(duration_),
              options(options_),
              panning(panning_),
              scaling(scaling_),
              rotating(rotating_) {}

        double interpolant(const TimePoint&) const;

        bool isAnimated() const { return duration != Duration::zero(); }
    };

    template <class T>
    struct Property {
        std::shared_ptr<Animation> animation;
        T current;
        T target;
        bool set = false;

        std::function<const LatLng(const TimePoint&)> frameLatLngFunc = nullptr;
        std::function<double(const TimePoint&)> frameZoomFunc = nullptr;
    };

    struct Properties {
        Property<Point<double>> latlng;
        Property<double> zoom, bearing, pitch;
        Property<EdgeInsets> padding;
    };

    void startTransition(const CameraOptions&, const Duration&, Animation&);
    bool animationTransitionFrame(Animation&, const double);
    void animationFinishFrame(Animation&);

    void visitProperties(const std::function<void(Animation&)>& f) {
        if (properties.zoom.animation) {
            f(*properties.zoom.animation);
        }
        if (properties.latlng.animation) {
            f(*properties.latlng.animation);
        }
        if (properties.bearing.animation) {
            f(*properties.bearing.animation);
        }
        if (properties.padding.animation) {
            f(*properties.padding.animation);
        }
        if (properties.pitch.animation) {
            f(*properties.pitch.animation);
        }
    }

    Properties properties;
    bool activeAnimation = false;
};

} // namespace mbgl
