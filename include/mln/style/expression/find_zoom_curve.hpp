#pragma once

#include <mln/style/expression/interpolate.hpp>
#include <mln/style/expression/parsing_context.hpp>
#include <mln/style/expression/step.hpp>
#include <mln/util/variant.hpp>

#include <optional>

namespace mln {
namespace style {
namespace expression {

using ZoomCurveOrError = std::optional<variant<const Interpolate*, const Step*, ParsingError>>;
using ZoomCurvePtr = variant<std::nullptr_t, const Interpolate*, const Step*>;

/// Find a zoom curve in the expression tree.
/// @param expr Expression root
/// @return The relevant step or interpolate item, or the error if the expression represents an error
ZoomCurveOrError findZoomCurve(const expression::Expression&);

/// Find a zoom curve in the expression tree.
/// @param expr Expression root
/// @return The relevant step or interpolate item, or null if the expression represents an error
ZoomCurvePtr findZoomCurveChecked(const expression::Expression& expr);

} // namespace expression
} // namespace style
} // namespace mln
