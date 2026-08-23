#include <mln/style/color_ramp_property_value.hpp>
#include <mln/style/conversion/property_value.hpp>
#include <mln/style/conversion/filter.hpp>
#include <mln/style/conversion_impl.hpp>
#include <mln/style/expression/expression.hpp>
#include <mln/style/expression/format_section_override.hpp>
#include <mln/style/expression/is_constant.hpp>
#include <mln/style/expression/parsing_context.hpp>
#include <mln/style/filter.hpp>
#include <mln/style/property_expression.hpp>
#include <mln/style/rapidjson_conversion.hpp>
#include <mln/test/util.hpp>
#include <mln/util/rapidjson.hpp>

#include <rapidjson/document.h>

using namespace mln;
using namespace mln::style;
using namespace mln::style::expression;

using ExprValue = mln::style::expression::Value;

namespace {

std::unique_ptr<Expression> parseExpression(const std::string& json, std::optional<type::Type> expected = {}) {
    rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator> document;
    document.Parse<0>(json.c_str());
    assert(!document.HasParseError());
    const JSValue* expression = &document;
    ParsingContext ctx = expected ? ParsingContext(*expected) : ParsingContext();
    ParseResult parsed = ctx.parseExpression(conversion::Convertible(expression));
    return parsed ? std::move(*parsed) : nullptr;
}

} // namespace

TEST(GlobalStateExpression, Parse) {
    auto expr = parseExpression(R"(["global-state", "showLabels"])");
    ASSERT_TRUE(expr);
    EXPECT_EQ(Kind::CompoundExpression, expr->getKind());
    EXPECT_EQ("global-state", expr->getOperator());
    EXPECT_TRUE(expr->getType().is<type::ValueType>());
    EXPECT_TRUE(expr->dependencies & Dependency::GlobalState);
}

TEST(GlobalStateExpression, ParseErrors) {
    // Missing argument.
    EXPECT_FALSE(parseExpression(R"(["global-state"])"));
    // Too many arguments.
    EXPECT_FALSE(parseExpression(R"(["global-state", "a", "b"])"));
    // Non-string argument.
    EXPECT_FALSE(parseExpression(R"(["global-state", 1])"));
    // The property name must be a string literal, not a sub-expression.
    EXPECT_FALSE(parseExpression(R"(["global-state", ["get", "key"]])"));
    EXPECT_FALSE(parseExpression(R"(["global-state", ["concat", "a", "b"]])"));
    EXPECT_FALSE(parseExpression(R"(["global-state", ["literal", "a"]])"));
}

TEST(GlobalStateExpression, IsNotConstant) {
    auto expr = parseExpression(R"(["global-state", "showLabels"])");
    ASSERT_TRUE(expr);
    // A global-state expression with constant arguments must not be folded
    // into a literal at parse time.
    EXPECT_NE(Kind::Literal, expr->getKind());
    EXPECT_TRUE(isFeatureConstant(*expr));
    EXPECT_TRUE(isZoomConstant(*expr));
    EXPECT_FALSE(isRuntimeConstant(*expr));
}

TEST(GlobalStateExpression, Evaluate) {
    auto expr = parseExpression(R"(["global-state", "size"])");
    ASSERT_TRUE(expr);

    GlobalStateMap state{{"size", 12.0}};

    EvaluationContext context;
    context.globalState = &state;
    auto result = expr->evaluate(context);
    ASSERT_TRUE(result);
    EXPECT_EQ(ExprValue(12.0), *result);

    // Missing property evaluates to null.
    auto missing = parseExpression(R"(["global-state", "missing"])");
    ASSERT_TRUE(missing);
    result = missing->evaluate(context);
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->is<NullValue>());

    // No global state provided evaluates to null.
    result = expr->evaluate(EvaluationContext());
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->is<NullValue>());
}

TEST(GlobalStateExpression, EvaluateArray) {
    auto expr = parseExpression(R"(["global-state", "categories"])");
    ASSERT_TRUE(expr);

    GlobalStateMap state{{"categories", mapbox::base::ValueArray{"restaurant", "hotel"}}};

    EvaluationContext context;
    context.globalState = &state;
    auto result = expr->evaluate(context);
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->is<std::vector<ExprValue>>());
    const auto& array = result->get<std::vector<ExprValue>>();
    ASSERT_EQ(2u, array.size());
    EXPECT_EQ(ExprValue(std::string("restaurant")), array[0]);
    EXPECT_EQ(ExprValue(std::string("hotel")), array[1]);
}

TEST(GlobalStateExpression, EvaluateInComposition) {
    auto expr = parseExpression(R"(["case", ["to-boolean", ["global-state", "enabled"]], 10.0, 5.0])",
                                {type::Number});
    ASSERT_TRUE(expr);
    EXPECT_TRUE(expr->dependencies & Dependency::GlobalState);

    GlobalStateMap enabled{{"enabled", true}};
    GlobalStateMap disabled{{"enabled", false}};

    EvaluationContext context;
    context.globalState = &enabled;
    auto result = expr->evaluate(context);
    ASSERT_TRUE(result);
    EXPECT_EQ(ExprValue(10.0), *result);

    context.globalState = &disabled;
    result = expr->evaluate(context);
    ASSERT_TRUE(result);
    EXPECT_EQ(ExprValue(5.0), *result);

    // Without state, "enabled" is null -> false.
    result = expr->evaluate(EvaluationContext());
    ASSERT_TRUE(result);
    EXPECT_EQ(ExprValue(5.0), *result);
}

TEST(GlobalStateExpression, PropertyExpressionCapturesGlobalState) {
    auto expr = parseExpression(R"(["to-number", ["global-state", "width"]])", {type::Number});
    ASSERT_TRUE(expr);

    PropertyExpression<float> propertyExpression(std::move(expr));
    EXPECT_TRUE(propertyExpression.isFeatureConstant());
    EXPECT_TRUE(propertyExpression.isZoomConstant());
    EXPECT_FALSE(propertyExpression.isRuntimeConstant());

    // Without any state, the expression falls back to the default.
    EXPECT_EQ(0.0f, propertyExpression.evaluate(EvaluationContext(0.0f), 0.0f));

    auto state = std::make_shared<const GlobalStateMap>(GlobalStateMap{{"width", 3.0}});

    // State provided via the evaluation context.
    EXPECT_EQ(3.0f,
              propertyExpression.evaluate(EvaluationContext(0.0f).withGlobalState(state.get()), 0.0f));

    // Captured state is used when the context has none.
    propertyExpression.captureGlobalState(state);
    EXPECT_EQ(3.0f, propertyExpression.evaluate(EvaluationContext(0.0f), 0.0f));

    // An explicitly provided state takes precedence over the captured one.
    GlobalStateMap other{{"width", 7.0}};
    EXPECT_EQ(7.0f, propertyExpression.evaluate(EvaluationContext(0.0f).withGlobalState(&other), 0.0f));
}

TEST(GlobalStateExpression, PropertyValueConversionKeepsExpression) {
    rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator> document;
    document.Parse<0>(R"(["to-number", ["global-state", "width"]])");
    ASSERT_FALSE(document.HasParseError());
    const JSValue* json = &document;

    conversion::Error error;
    auto converted = conversion::convert<PropertyValue<float>>(
        conversion::Convertible(json), error, /*allowDataExpressions*/ true, /*convertTokens*/ false);
    ASSERT_TRUE(converted);
    // The property value must keep the expression rather than folding it into
    // a constant, so that changes to the global state can be picked up.
    EXPECT_TRUE(converted->isExpression());
    EXPECT_TRUE(converted->getDependencies() & Dependency::GlobalState);
}

TEST(GlobalStateExpression, Filter) {
    rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator> document;
    document.Parse<0>(R"(["to-boolean", ["global-state", "showFeatures"]])");
    ASSERT_FALSE(document.HasParseError());
    const JSValue* json = &document;

    conversion::Error error;
    auto filter = conversion::convert<Filter>(conversion::Convertible(json), error);
    ASSERT_TRUE(filter);

    GlobalStateMap shown{{"showFeatures", true}};
    GlobalStateMap hidden{{"showFeatures", false}};

    EXPECT_TRUE((*filter)(EvaluationContext().withGlobalState(&shown)));
    EXPECT_FALSE((*filter)(EvaluationContext().withGlobalState(&hidden)));
    EXPECT_FALSE((*filter)(EvaluationContext()));
}

TEST(GlobalStateExpression, ColorRamp) {
    rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator> document;
    document.Parse<0>(R"(["to-color", ["global-state", "hotColor"]])");
    ASSERT_FALSE(document.HasParseError());
    const JSValue* json = &document;

    ParsingContext ctx(type::Color);
    ParseResult parsed = ctx.parseExpression(conversion::Convertible(json));
    ASSERT_TRUE(parsed);

    ColorRampPropertyValue ramp(std::move(*parsed));
    EXPECT_TRUE(ramp.getDependencies() & Dependency::GlobalState);

    GlobalStateMap state{{"hotColor", std::string("red")}};
    EXPECT_EQ(*Color::parse("red"), ramp.evaluate(0.5, &state));

    // Without global state the expression fails to produce a color; the
    // evaluation must degrade to a default color instead of crashing.
    EXPECT_EQ(Color{}, ramp.evaluate(0.5));
}

TEST(GlobalStateExpression, FormatSectionOverrideUsesCapturedState) {
    auto expr = parseExpression(R"(["to-color", ["global-state", "labelColor"]])", {type::Color});
    ASSERT_TRUE(expr);

    PropertyExpression<Color> propertyExpression(std::move(expr));
    auto state = std::make_shared<const GlobalStateMap>(GlobalStateMap{{"labelColor", std::string("red")}});
    propertyExpression.captureGlobalState(state);

    PossiblyEvaluatedPropertyValue<Color> defaultValue(std::move(propertyExpression));
    FormatSectionOverride<Color> overrideExpression(type::Color, std::move(defaultValue), "text-color");

    // Evaluating without a formatted section and without global state in the
    // context must fall back to the state captured by the wrapped expression.
    auto result = overrideExpression.evaluate(EvaluationContext());
    ASSERT_TRUE(result);
    EXPECT_EQ(ExprValue(*Color::parse("red")), *result);
}

TEST(GlobalStateExpression, Serialize) {
    auto expr = parseExpression(R"(["global-state", "showLabels"])");
    ASSERT_TRUE(expr);
    auto serialized = expr->serialize();
    ASSERT_TRUE(serialized.is<mapbox::base::ValueArray>());
    const auto& array = serialized.get<mapbox::base::ValueArray>();
    ASSERT_EQ(2u, array.size());
    EXPECT_EQ(mln::Value("global-state"), array[0]);
    EXPECT_EQ(mln::Value("showLabels"), array[1]);
}
