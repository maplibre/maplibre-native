#pragma once

#include <mln/style/projection.hpp>
#include <mln/util/subdivision_granularity.hpp>

namespace mln {
namespace style {

class Projection::Impl {
public:
    PropertyValue<ProjectionDefinition> type;

    /// The definition at a zoom level; `mercator` when the style does not set one.
    ProjectionDefinition evaluate(float zoom) const;

    /// How finely tiles are subdivided for this projection, for the life of the style: anything that can render a
    /// globe at some zoom gets the globe granularity, so the Mercator hand-off at high zoom reuses the same tiles.
    SubdivisionGranularitySetting getSubdivisionGranularity() const;
};

} // namespace style
} // namespace mln
