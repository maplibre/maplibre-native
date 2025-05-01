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
    std::optional<ScreenCoordinate> anchor;
    LatLng anchorLatLng;
};

struct PropertyAnimations {
    TransformProperty<Point<double>> latlng;
    TransformProperty<double> zoom, bearing, pitch;
    TransformProperty<EdgeInsets> padding;
};

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

    void startTransition(const CameraOptions&, const Duration&);

    bool animationTransitionFrame(std::shared_ptr<PropertyAnimation>&, double);
    void animationFinishFrame(std::shared_ptr<PropertyAnimation>&);

    void visit_pas(const std::function<void(std::shared_ptr<PropertyAnimation>&)>& f) {
        f(pas.latlng.pa);
        f(pas.zoom.pa);
        f(pas.bearing.pa);
        f(pas.pitch.pa);
        f(pas.padding.pa);
    }

    PropertyAnimations pas;
    bool activeAnimation = false;
};

} // namespace mbgl
