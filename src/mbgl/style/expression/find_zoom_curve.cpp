#include <mbgl/style/expression/find_zoom_curve.hpp>
#include <mbgl/style/expression/compound_expression.hpp>
#include <mbgl/style/expression/let.hpp>
#include <mbgl/style/expression/coalesce.hpp>
#include <mbgl/style/expression/is_constant.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/variant.hpp>

#include <optional>

namespace mbgl {
namespace style {
namespace expression {

ZoomCurveOrError findZoomCurve(const expression::Expression& e) {
    ZoomCurveOrError result;

    switch (e.getKind()) {
        case Kind::Let: {
            const auto& let = static_cast<const Let&>(e);
            if (const auto& child = let.getResult()) {
                result = findZoomCurve(*child);
            }
            break;
        }
        case Kind::Coalesce: {
            const auto& coalesce = static_cast<const Coalesce&>(e);
            const std::size_t length = coalesce.getLength();
            for (std::size_t i = 0; i < length; i++) {
                if (const auto* child = coalesce.getChild(i)) {
                    result = findZoomCurve(*child);
                    if (result) {
                        break;
                    }
                }
            }
            break;
        }
        case Kind::Interpolate: {
            const auto& curve = static_cast<const Interpolate&>(e);
            if (curve.getInput()->getKind() == Kind::CompoundExpression) {
                const auto* z = static_cast<CompoundExpression*>(curve.getInput().get());
                if (z && z->getOperator() == "zoom") {
                    result = {&curve};
                }
            }
            break;
        }
        case Kind::Step: {
            const auto& step = static_cast<const Step&>(e);
            if (step.getInput()->getKind() == Kind::CompoundExpression) {
                const auto* z = static_cast<CompoundExpression*>(step.getInput().get());
                if (z && z->getOperator() == "zoom") {
                    result = {&step};
                }
            }
            break;
        }
        default:
            break;
    }

    if (result && result->is<ParsingError>()) {
        return result;
    }

    e.eachChild([&](const Expression& child) {
        if (const ZoomCurveOrError childResult = findZoomCurve(child)) {
            if (childResult->is<ParsingError>()) {
                result = childResult;
            } else if (!result && childResult) {
                result = {ParsingError{
                    .message =
                        R"("zoom" expression may only be used as input to a top-level "step" or "interpolate" expression.)",
                    .key = ""}};
            } else if (result && childResult && result != childResult) {
                result = {ParsingError{
                    .message =
                        R"(Only one zoom-based "step" or "interpolate" subexpression may be used in an expression.)",
                    .key = ""}};
            }
        }
    });

    return result;
}

ZoomCurvePtr findZoomCurveChecked(const expression::Expression& e) {
    if (!e.has(Dependency::Zoom)) {
        return nullptr;
    }
    return findZoomCurve(e)->match(
        [](const ParsingError& err) -> ZoomCurvePtr {
            Log::Error(Event::Style, "Invalid Expression: " + err.message);
            return nullptr;
        },
        [](auto zoomCurve) -> ZoomCurvePtr { return zoomCurve; });
}

} // namespace expression
} // namespace style
} // namespace mbgl
