#include <mln/style/projection.hpp>
#include <mln/style/projection_impl.hpp>
#include <mln/style/projection_observer.hpp>
#include <mln/style/conversion/property_value.hpp>
#include <mln/style/conversion_impl.hpp>
#include <mln/renderer/property_evaluator.hpp>

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
    // `globe` is the vertical perspective up to the first zoom and Mercator from the second, blended in between.
    constexpr float globeToMercatorStartZoom = 11;
    constexpr float globeToMercatorEndZoom = 12;
    if (definition.from == ProjectionType::Globe && definition.to == ProjectionType::Globe) {
        if (zoom <= globeToMercatorStartZoom) {
            return ProjectionDefinition(ProjectionType::VerticalPerspective);
        }
        if (zoom >= globeToMercatorEndZoom) {
            return ProjectionDefinition(ProjectionType::Mercator);
        }
        return ProjectionDefinition(
            ProjectionType::VerticalPerspective, ProjectionType::Mercator, zoom - globeToMercatorStartZoom);
    }
    return definition;
}

} // namespace style
} // namespace mln
