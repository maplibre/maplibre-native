#include <mln/style/projection.hpp>
#include <mln/style/projection_impl.hpp>
#include <mln/style/projection_observer.hpp>
#include <mln/style/conversion/property_value.hpp>
#include <mln/style/conversion_impl.hpp>
#include <mln/renderer/property_evaluator.hpp>
#include <mln/util/interpolate.hpp>

#include <algorithm>

namespace mln {
namespace style {

namespace {
ProjectionObserver nullObserver;
} // namespace

Projection::Projection(Immutable<Projection::Impl> impl_)
    : impl(std::move(impl_)),
      observer(&nullObserver) {}

Projection::Projection()
    : Projection(makeMutable<Impl>()) {}

Projection::~Projection() = default;

void Projection::setObserver(ProjectionObserver* observer_) {
    observer = observer_ ? observer_ : &nullObserver;
}

Mutable<Projection::Impl> Projection::mutableImpl() const {
    return makeMutable<Impl>(*impl);
}

ProjectionDefinition Projection::getDefaultType() {
    return ProjectionDefinition();
}

PropertyValue<ProjectionDefinition> Projection::getType() const {
    return impl->type;
}

void Projection::setType(PropertyValue<ProjectionDefinition> type) {
    auto mutableImpl_ = mutableImpl();
    mutableImpl_->type = std::move(type);
    impl = std::move(mutableImpl_);
    observer->onProjectionChanged(*this);
}

std::optional<conversion::Error> Projection::setProperty(const std::string& name,
                                                         const conversion::Convertible& value) {
    if (name != "type") {
        return conversion::Error{"projection doesn't support this property"};
    }
    conversion::Error error;
    const auto type = conversion::convert<PropertyValue<ProjectionDefinition>>(value, error, false, false);
    if (!type) {
        return error;
    }
    setType(*type);
    return std::nullopt;
}

StyleProperty Projection::getProperty(const std::string& name) const {
    if (name == "type") {
        return conversion::makeStyleProperty(getType());
    }
    return {};
}

SubdivisionGranularitySetting Projection::Impl::getSubdivisionGranularity() const {
    if (type.isUndefined() ||
        (type.isConstant() && type.asConstant() == ProjectionDefinition(ProjectionType::Mercator))) {
        return SubdivisionGranularitySetting::none();
    }
    return SubdivisionGranularitySetting::globe();
}

ProjectionDefinition Projection::Impl::evaluate(float zoom) const {
    const PropertyEvaluationParameters parameters(zoom);
    const auto definition = type.evaluate(
        PropertyEvaluator<ProjectionDefinition>(parameters, Projection::getDefaultType()));
    if (definition.from != ProjectionType::Globe && definition.to != ProjectionType::Globe) {
        return definition;
    }
    // `globe` is the vertical perspective up to the first zoom and Mercator from the second, blended in between, at
    // either end of a definition: each end counts with its share of the globe at this zoom.
    constexpr float globeToMercatorStartZoom = 11;
    constexpr float globeToMercatorEndZoom = 12;
    const double globeShare = 1.0 - std::clamp(static_cast<double>(zoom - globeToMercatorStartZoom) /
                                                   (globeToMercatorEndZoom - globeToMercatorStartZoom),
                                               0.0,
                                               1.0);
    const auto share = [&](ProjectionType end) {
        return end == ProjectionType::Globe ? globeShare : end == ProjectionType::Mercator ? 0.0 : 1.0;
    };
    const double globe = definition.from == definition.to
                             ? share(definition.from)
                             : util::interpolate(share(definition.from),
                                                 share(definition.to),
                                                 std::clamp(definition.transition, 0.0, 1.0));
    if (globe >= 1) {
        return ProjectionDefinition(ProjectionType::VerticalPerspective);
    }
    if (globe <= 0) {
        return ProjectionDefinition(ProjectionType::Mercator);
    }
    return ProjectionDefinition(ProjectionType::VerticalPerspective, ProjectionType::Mercator, 1 - globe);
}

} // namespace style
} // namespace mln
