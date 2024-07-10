#pragma once

#include <mbgl/style/expression/interpolate.hpp>
#include <mbgl/style/expression/parsing_context.hpp>
#include <mbgl/style/expression/step.hpp>
#include <mbgl/util/variant.hpp>

#include <optional>

namespace mbgl {
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
} // namespace mbgl
