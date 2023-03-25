#pragma once

#include <mbgl/style/expression/parsing_context.hpp>
#include <mbgl/style/expression/interpolate.hpp>
#include <mbgl/style/expression/step.hpp>

#include <mbgl/util/variant.hpp>

#include <optional>

namespace mbgl {
namespace style {
namespace expression {

std::optional<variant<const Interpolate*, const Step*, ParsingError>> findZoomCurve(const expression::Expression* e);

variant<std::nullptr_t, const Interpolate*, const Step*> findZoomCurveChecked(const expression::Expression* e);

} // namespace expression
} // namespace style
} // namespace mbgl
