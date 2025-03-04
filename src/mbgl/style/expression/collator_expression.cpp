#include <mbgl/style/expression/collator.hpp>
#include <mbgl/style/expression/collator_expression.hpp>
#include <mbgl/style/expression/literal.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/util/string.hpp>

namespace mbgl {
namespace style {
namespace expression {

namespace {
/// Collators are considered feature-dependent, see `isFeatureConstant`
constexpr auto extraDependency = Dependency::Feature;
} // namespace

CollatorExpression::CollatorExpression(std::unique_ptr<Expression> caseSensitive_,
                                       std::unique_ptr<Expression> diacriticSensitive_,
                                       std::optional<std::unique_ptr<Expression>> locale_)
    : Expression(Kind::CollatorExpression,
                 type::Collator,
                 depsOf(caseSensitive_) | depsOf(diacriticSensitive_) | depsOf(locale_) | extraDependency),
      caseSensitive(std::move(caseSensitive_)),
      diacriticSensitive(std::move(diacriticSensitive_)),
      locale(std::move(locale_)) {}

using namespace mbgl::style::conversion;

ParseResult CollatorExpression::parse(const Convertible& value, ParsingContext& ctx) {
    if (arrayLength(value) != 2) {
        ctx.error("Expected one argument.");
        return ParseResult();
    }

    auto options = arrayMember(value, 1);
    if (!isObject(options)) {
        ctx.error("Collator options argument must be an object.");
        return ParseResult();
    }

    const std::optional<Convertible> caseSensitiveOption = objectMember(options, "case-sensitive");
    ParseResult caseSensitive;
    if (caseSensitiveOption) {
        caseSensitive = ctx.parse(*caseSensitiveOption, 1, {type::Boolean});
    } else {
        caseSensitive = {std::make_unique<Literal>(false)};
    }
    if (!caseSensitive) {
        return ParseResult();
    }

    const std::optional<Convertible> diacriticSensitiveOption = objectMember(options, "diacritic-sensitive");
    ParseResult diacriticSensitive;
    if (diacriticSensitiveOption) {
        diacriticSensitive = ctx.parse(*diacriticSensitiveOption, 1, {type::Boolean});
    } else {
        diacriticSensitive = {std::make_unique<Literal>(false)};
    }
    if (!diacriticSensitive) {
        return ParseResult();
    }

    const std::optional<Convertible> localeOption = objectMember(options, "locale");
    ParseResult locale;
    if (localeOption) {
        locale = ctx.parse(*localeOption, 1, {type::String});
        if (!locale || !*locale) {
            return ParseResult();
        }
    }

    return ParseResult(std::make_unique<CollatorExpression>(
        std::move(*caseSensitive), std::move(*diacriticSensitive), std::move(locale)));
}

void CollatorExpression::eachChild(const std::function<void(const Expression&)>& fn) const {
    fn(*caseSensitive);
    fn(*diacriticSensitive);
    if (locale && *locale) {
        fn(**locale);
    }
}

bool CollatorExpression::operator==(const Expression& e) const noexcept {
    if (e.getKind() == Kind::CollatorExpression) {
        const auto* rhs = static_cast<const CollatorExpression*>(&e);
        const bool lLocSet = locale && *locale;
        const bool rLocSet = rhs->locale && *rhs->locale;
        if ((!lLocSet && rLocSet) || (lLocSet && (!rLocSet || **locale != **rhs->locale))) {
            return false;
        }
        return *caseSensitive == *(rhs->caseSensitive) && *diacriticSensitive == *(rhs->diacriticSensitive);
    }
    return false;
}

mbgl::Value CollatorExpression::serialize() const {
    std::unordered_map<std::string, mbgl::Value> options;
    options["case-sensitive"] = caseSensitive->serialize();
    options["diacritic-sensitive"] = diacriticSensitive->serialize();
    if (locale && *locale) {
        options["locale"] = (*locale)->serialize();
    }
    return std::vector<mbgl::Value>{{std::string("collator"), options}};
}

EvaluationResult CollatorExpression::evaluate(const EvaluationContext& params) const {
    auto caseSensitiveResult = caseSensitive->evaluate(params);
    if (!caseSensitiveResult) {
        return caseSensitiveResult.error();
    }
    auto diacriticSensitiveResult = diacriticSensitive->evaluate(params);
    if (!diacriticSensitiveResult) {
        return diacriticSensitiveResult.error();
    }

    if (locale && *locale) {
        if (auto localeResult = (*locale)->evaluate(params)) {
            return Collator(caseSensitiveResult->get<bool>(),
                            diacriticSensitiveResult->get<bool>(),
                            localeResult->get<std::string>());
        } else {
            return localeResult.error();
        }
    } else {
        return Collator(caseSensitiveResult->get<bool>(), diacriticSensitiveResult->get<bool>());
    }
}

} // namespace expression
} // namespace style
} // namespace mbgl
