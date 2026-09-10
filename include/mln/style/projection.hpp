#pragma once

#include <mln/style/conversion.hpp>
#include <mln/style/projection_definition.hpp>
#include <mln/style/property_value.hpp>
#include <mln/style/style_property.hpp>
#include <mln/util/immutable.hpp>

#include <optional>
#include <string>

namespace mln {
namespace style {

class ProjectionObserver;

/// The style's root `projection` object.
class Projection {
public:
    Projection();
    ~Projection();

    std::optional<conversion::Error> setProperty(const std::string& name, const conversion::Convertible& value);
    StyleProperty getProperty(const std::string&) const;

    static ProjectionDefinition getDefaultType();
    PropertyValue<ProjectionDefinition> getType() const;
    void setType(PropertyValue<ProjectionDefinition>);

    class Impl;
    Immutable<Impl> impl;
    explicit Projection(Immutable<Impl>);
    Mutable<Impl> mutableImpl() const;

    ProjectionObserver* observer = nullptr;
    void setObserver(ProjectionObserver*);
};

} // namespace style
} // namespace mln
