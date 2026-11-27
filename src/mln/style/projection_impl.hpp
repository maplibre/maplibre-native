#pragma once

#include <mln/style/projection.hpp>

#include <memory>
#include <string>

namespace mln {

class CRSWorldCoordinateHelper;

namespace style {

class Projection::Impl {
public:
    /// The style's `projection.type`: `mercator`, `vertical-perspective`, `globe`, `identity` or the name of a
    /// projection registered with `addProjection`. Empty when the style sets none or sets an expression, which draws as
    /// Mercator until the globe is supported.
    std::string type;
    /// The projection registered under that name when the type was set, or null when none was. The map, its sources
    /// and annotations draw in this one, so removing the name from the registry leaves them as they are.
    std::shared_ptr<const CRSWorldCoordinateHelper> registeredProjection;
};

} // namespace style
} // namespace mln
