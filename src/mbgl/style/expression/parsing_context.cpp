
#include <mbgl/style/expression/parsing_context.hpp>
#include <mbgl/style/expression/check_subtype.hpp>
#include <mbgl/style/expression/is_constant.hpp>
#include <mbgl/style/expression/type.hpp>

#include <mbgl/style/expression/assertion.hpp>
#include <mbgl/style/expression/at.hpp>
#include <mbgl/style/expression/boolean_operator.hpp>
#include <mbgl/style/expression/case.hpp>
#include <mbgl/style/expression/coalesce.hpp>
#include <mbgl/style/expression/coercion.hpp>
#include <mbgl/style/expression/comparison.hpp>
#include <mbgl/style/expression/compound_expression.hpp>
#include <mbgl/style/expression/distance.hpp>
#include <mbgl/style/expression/expression.hpp>
#include <mbgl/style/expression/format_expression.hpp>
#include <mbgl/style/expression/image_expression.hpp>
#include <mbgl/style/expression/in.hpp>
#include <mbgl/style/expression/interpolate.hpp>
#include <mbgl/style/expression/length.hpp>
#include <mbgl/style/expression/let.hpp>
#include <mbgl/style/expression/literal.hpp>
#include <mbgl/style/expression/match.hpp>
#include <mbgl/style/expression/number_format.hpp>
#include <mbgl/style/expression/step.hpp>
#include <mbgl/style/expression/within.hpp>
#include <mbgl/style/expression/index_of.hpp>
#include <mbgl/style/expression/slice.hpp>

#include <mbgl/style/expression/find_zoom_curve.hpp>
#include <mbgl/style/expression/dsl.hpp>

#include <mbgl/style/conversion/get_json_type.hpp>
#include <mbgl/style/conversion_impl.hpp>

#include <mbgl/util/string.hpp>

#include <mapbox/eternal.hpp>
#include <utility>

namespace mbgl {
namespace style {
namespace expression {

namespace {
const auto requiredProps = std::array<std::string_view, 4>{"zoom", "heatmap-density", "line-progress", "accumulated"};

bool isConstant(const Expression& expression) {
    const auto kind = expression.getKind();

    if (kind == Kind::Var) {
        const auto* varExpression = static_cast<const Var*>(&expression);
        return isConstant(*varExpression->getBoundExpression());
    } else if (kind == Kind::CompoundExpression) {
        const auto* compound = static_cast<const CompoundExpression*>(&expression);
        if (compound->getOperator() == "error") {
            return false;
        }
    }

    const bool isTypeAnnotation = kind == Kind::Coercion || kind == Kind::Assertion;

    bool childrenConstant = true;
    expression.eachChild([&](const Expression& child) {
        // We can _almost_ assume that if `expressions` children are constant,
        // they would already have been evaluated to Literal values when they
        // were parsed.  Type annotations are the exception, because they might
        // have been inferred and added after a child was parsed.

        // So we recurse into isConstant() for the children of type annotations,
        // but otherwise simply check whether they are Literals.
        if (isTypeAnnotation) {
            childrenConstant = childrenConstant && isConstant(child);
        } else {
            childrenConstant = childrenConstant && child.getKind() == Kind::Literal;
        }
    });

    return childrenConstant && isFeatureConstant(expression) && isGlobalPropertyConstant(expression, requiredProps);
}
} // namespace

using namespace mbgl::style::conversion;

ParseResult ParsingContext::parse(const Convertible& value,
                                  std::size_t index_,
                                  std::optional<type::Type> expected_,
                                  const std::optional<TypeAnnotationOption>& typeAnnotationOption) {
    ParsingContext child(key + "[" + util::toString(index_) + "]", errors, std::move(expected_), scope);
    return child.parse(value, typeAnnotationOption);
}

ParseResult ParsingContext::parse(const Convertible& value,
                                  std::size_t index_,
                                  std::optional<type::Type> expected_,
                                  const std::map<std::string, std::shared_ptr<Expression>>& bindings) {
    ParsingContext child(key + "[" + util::toString(index_) + "]",
                         errors,
                         std::move(expected_),
                         std::make_shared<detail::Scope>(bindings, scope));
    return child.parse(value);
}

using ParseFunction = ParseResult (*)(const conversion::Convertible&, ParsingContext&);
MAPBOX_ETERNAL_CONSTEXPR const auto expressionRegistry =
    mapbox::eternal::hash_map<mapbox::eternal::string, ParseFunction>({{"==", parseComparison},
                                                                       {"!=", parseComparison},
                                                                       {">", parseComparison},
                                                                       {"<", parseComparison},
                                                                       {">=", parseComparison},
                                                                       {"<=", parseComparison},
                                                                       {"all", All::parse},
                                                                       {"any", Any::parse},
                                                                       {"array", Assertion::parse},
                                                                       {"at", At::parse},
                                                                       {"in", In::parse},
                                                                       {"boolean", Assertion::parse},
                                                                       {"case", Case::parse},
                                                                       {"coalesce", Coalesce::parse},
                                                                       {"collator", CollatorExpression::parse},
                                                                       {"distance", Distance::parse},
                                                                       {"format", FormatExpression::parse},
                                                                       {"image", ImageExpression::parse},
                                                                       {"interpolate", parseInterpolate},
                                                                       {"length", Length::parse},
                                                                       {"let", Let::parse},
                                                                       {"literal", Literal::parse},
                                                                       {"match", parseMatch},
                                                                       {"number", Assertion::parse},
                                                                       {"number-format", NumberFormat::parse},
                                                                       {"object", Assertion::parse},
                                                                       {"step", Step::parse},
                                                                       {"string", Assertion::parse},
                                                                       {"to-boolean", Coercion::parse},
                                                                       {"to-color", Coercion::parse},
                                                                       {"to-number", Coercion::parse},
                                                                       {"to-string", Coercion::parse},
                                                                       {"var", Var::parse},
                                                                       {"within", Within::parse},
                                                                       {"index-of", IndexOf::parse},
                                                                       {"slice", Slice::parse}});

bool isExpression(const std::string& name) noexcept {
    return expressionRegistry.contains(name.c_str());
}

ParseResult ParsingContext::parse(const Convertible& value,
                                  const std::optional<TypeAnnotationOption>& typeAnnotationOption) {
    ParseResult parsed;

    if (isArray(value)) {
        const std::size_t length = arrayLength(value);
        if (length == 0) {
            error(
                R"(Expected an array with at least one element. If you wanted a literal array, use ["literal", []].)");
            return ParseResult();
        }

        const std::optional<std::string> op = toString(arrayMember(value, 0));
        if (!op) {
            error("Expression name must be a string, but found " + getJSONType(arrayMember(value, 0)) +
                      R"( instead. If you wanted a literal array, use ["literal", [...]].)",
                  0);
            return ParseResult();
        }

        auto parseFunction = expressionRegistry.find(op->c_str());
        if (parseFunction != expressionRegistry.end()) {
            parsed = parseFunction->second(value, *this);
        } else {
            parsed = parseCompoundExpression(*op, value, *this);
        }
    } else {
        parsed = Literal::parse(value, *this);
    }

    if (!parsed) {
        assert(errors->size() > 0);
        return parsed;
    }

    const auto annotate = [](std::unique_ptr<Expression> expression,
                             type::Type type,
                             TypeAnnotationOption typeAnnotation) -> std::unique_ptr<Expression> {
        switch (typeAnnotation) {
            case TypeAnnotationOption::assert:
                return std::make_unique<Assertion>(std::move(type), dsl::vec(std::move(expression)));
            case TypeAnnotationOption::coerce:
                return std::make_unique<Coercion>(std::move(type), dsl::vec(std::move(expression)));
            case TypeAnnotationOption::omit:
                return expression;
        }

        // Not reachable, but placate GCC.
        assert(false);
        return expression;
    };

    if (expected) {
        const type::Type actual = (*parsed)->getType();
        if ((*expected == type::String || *expected == type::Number || *expected == type::Boolean ||
             *expected == type::Object || expected->is<type::Array>()) &&
            actual == type::Value) {
            parsed = {annotate(
                std::move(*parsed), std::move(*expected), typeAnnotationOption.value_or(TypeAnnotationOption::assert))};
        } else if ((*expected == type::Color || *expected == type::Formatted || *expected == type::Image) &&
                   (actual == type::Value || actual == type::String)) {
            parsed = {annotate(
                std::move(*parsed), std::move(*expected), typeAnnotationOption.value_or(TypeAnnotationOption::coerce))};
        } else {
            checkType((*parsed)->getType());
            if (!errors->empty()) {
                return ParseResult();
            }
        }
    }

    // If an expression's arguments are all constant, we can evaluate
    // it immediately and replace it with a literal value in the
    // parsed/compiled result. Expressions that expect an image should
    // not be resolved here so we can later get the available images.
    if ((*parsed)->getKind() != Kind::Literal && (*parsed)->getType() != type::Image && isConstant(**parsed)) {
        EvaluationContext params(nullptr);
        EvaluationResult evaluated((*parsed)->evaluate(params));
        if (!evaluated) {
            error(evaluated.error().message);
            return ParseResult();
        }

        const type::Type type = (*parsed)->getType();
        if (type.is<type::Array>()) {
            // keep the original expression's array type, even if the evaluated
            // type is more specific.
            return ParseResult(
                std::make_unique<Literal>(type.get<type::Array>(), evaluated->get<std::vector<Value>>()));
        } else {
            return ParseResult(std::make_unique<Literal>(std::move(*evaluated)));
        }
    }

    return parsed;
}

ParseResult ParsingContext::parseExpression(const Convertible& value,
                                            const std::optional<TypeAnnotationOption>& typeAnnotationOption) {
    return parse(value, typeAnnotationOption);
}

ParseResult ParsingContext::parseLayerPropertyExpression(const Convertible& value) {
    const auto typeAnnotationOption = (expected && *expected == type::String)
                                          ? std::make_optional(TypeAnnotationOption::coerce)
                                          : std::nullopt;
    ParseResult parsed = parse(value, typeAnnotationOption);

    // If the expression is zoom-dependent, validate that we can resolve a zoom curve or treat it as an error
    assert(!parsed || !*parsed || !((*parsed)->dependencies & Dependency::Zoom) == isZoomConstant(**parsed));
    if (parsed && (*parsed)->has(Dependency::Zoom)) {
        if (const auto zoomCurve = findZoomCurve(**parsed)) {
            if (zoomCurve->is<ParsingError>()) {
                error(zoomCurve->get<ParsingError>().message);
                return ParseResult();
            }
        } else {
            error(R"("zoom" expression may only be used as input to a top-level "step" or "interpolate" expression.)");
            return ParseResult();
        }
    }
    return parsed;
}

std::string ParsingContext::getCombinedErrors() const {
    std::string combinedError;
    for (const ParsingError& parsingError : *errors) {
        if (!combinedError.empty()) {
            combinedError += "\n";
        }
        if (!parsingError.key.empty()) {
            combinedError += parsingError.key + ": ";
        }
        combinedError += parsingError.message;
    }
    return combinedError;
}

std::optional<std::string> ParsingContext::checkType(const type::Type& t) {
    assert(expected);
    std::optional<std::string> err = type::checkSubtype(*expected, t);
    if (err) {
        error(*err);
    }
    return err;
}

} // namespace expression
} // namespace style
} // namespace mbgl
