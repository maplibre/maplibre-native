#pragma once

#include <mln/style/projection.hpp>

#include <string>

namespace mln {
namespace style {

class Projection::Impl {
public:
    /// The style's `projection.type`: `mercator`, `vertical-perspective` or `globe`. Empty when the style sets none or
    /// sets an expression, which draws as Mercator until the globe is supported.
    std::string type;
};

} // namespace style
} // namespace mln
