#include <mln/style/projection.hpp>
#include <mln/style/projection_impl.hpp>
#include <mln/style/projection_observer.hpp>
#include <mln/style/conversion_impl.hpp>
#include <mln/map/world_coordinate_helper.hpp>
#include <mln/util/logging.hpp>

#include <string_view>

namespace mln {
namespace style {

namespace {
ProjectionObserver nullObserver;

/// `identity`, or a name with a colon, which is how registered projections are named.
bool isRegisteredProjectionName(std::string_view name) {
    return name == "identity" || name.find(':') != std::string_view::npos;
}

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

std::optional<conversion::Error> Projection::setProperty(const std::string& name,
                                                         const conversion::Convertible& value) {
    if (name != "type") {
        return conversion::Error{"projection doesn't support this property"};
    }
    std::string type;
    if (auto string = toString(value)) {
        type = std::move(*string);
        if (type == "vertical-perspective" || type == "globe") {
            Log::Warning(Event::Style,
                         "The globe projection is not supported yet, so the style's projection draws as Mercator");
        } else if (type != "mercator" && !isRegisteredProjectionName(type)) {
            return conversion::Error{"projection type must be mercator, vertical-perspective or globe"};
        }
    } else if (isArray(value) || isObject(value)) {
        Log::Warning(Event::Style,
                     "Projection expressions are not supported yet, so the style's projection draws as Mercator");
    } else if (!isUndefined(value)) {
        return conversion::Error{"value must be a string or an array of [from, to, transition]"};
    }

    auto impl_ = mutableImpl();
    impl_->registeredProjection = isRegisteredProjectionName(type) ? getRegisteredProjection(type) : nullptr;
    if (isRegisteredProjectionName(type) && !impl_->registeredProjection) {
        Log::Warning(Event::General, "Unknown projection name \"" + type + "\". Falling back to mercator projection.");
    }
    impl_->type = std::move(type);
    impl = std::move(impl_);
    observer->onProjectionChanged(*this);
    return std::nullopt;
}

StyleProperty Projection::getProperty(const std::string& name) const {
    if (name == "type" && !impl->type.empty()) {
        return {impl->type, StyleProperty::Kind::Constant};
    }
    return {};
}

} // namespace style
} // namespace mln
