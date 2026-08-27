#pragma once

#include <mln/style/projection.hpp>

namespace mln {
namespace style {

class Projection::Impl {
public:
    PropertyValue<ProjectionDefinition> type;

    /// The definition at a zoom level; `mercator` when the style does not set one.
    ProjectionDefinition evaluate(float zoom) const;
};

} // namespace style
} // namespace mln
