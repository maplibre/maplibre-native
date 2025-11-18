#include <mbgl/test/util.hpp>

#include <mbgl/renderer/property_evaluator.hpp>
#include <mbgl/renderer/property_evaluation_parameters.hpp>

#include <mbgl/style/types.hpp>
#include <mbgl/style/conversion/json.hpp>
#include <mbgl/style/expression/dsl.hpp>
#include <mbgl/style/property_expression.hpp>

#include <mbgl/style/conversion/function.hpp>
#include <mbgl/style/conversion/property_value.hpp>
#include <mbgl/style/conversion_impl.hpp>

#include <mbgl/test/stub_geometry_tile_feature.hpp>

using namespace mbgl;
using namespace style;
using namespace style::expression;
using namespace style::expression::dsl;
using namespace std::string_literals;

TEST(Padding, InterpolateExpression) {
    // Use DLS methods to create an expression
    {
        auto expr = PropertyExpression<Padding>(
            interpolate(linear(), zoom(), 0.0, literal(Padding(0.0)), 1.0, literal(Padding(8.0))));

        auto result = expr.evaluate(0.5f);
        EXPECT_EQ(Padding(4.0), result);
    }

    // Parse expression from JSON, verify that expansion from number/array is handled correctly.
    {
        auto json =
            R"(["interpolate", ["linear"], ["zoom"], 0, ["to-padding", 0], 1, ["to-padding", ["literal",[8, 16, -32]]]])";
        PropertyExpression<Padding> expr(createExpression(json));

        auto result = expr.evaluate(0.5f);
        EXPECT_EQ(Padding(4.0, 8, -16, 8), result);
    }
}

TEST(Padding, Function) {
    auto fromFunction = [](const char* json) {
        conversion::Error error;
        auto function = conversion::convertJSON<PropertyValue<Padding>>(
            json, error, /*allowDataExpressions*/ true, /*convertTokens*/ false);
        return function->asExpression();
    };

    auto evalInContext = [](const auto& expr, const PropertyMap& properties) {
        StubGeometryTileFeature feature{{}, FeatureType::Unknown, {}, properties};
        EvaluationContext context{0, &feature};
        auto a = expr.evaluate(context);
        return a;
    };

    // exponential
    {
        auto expr = fromFunction(R"({"type": "exponential", "stops": [[1, 2], [11, [2, 5, 2, 7]]]})");
        EXPECT_EQ(Padding(2, 2, 2, 2), expr.evaluate(0));
        EXPECT_EQ(Padding(2, 3.2, 2, 4), expr.evaluate(5));
        EXPECT_EQ(Padding(2, 5, 2, 7), expr.evaluate(11));
    }
    // interval
    {
        auto expr = fromFunction(R"({"type": "interval", "stops": [[1, 2], [11, 4]]})");
        EXPECT_EQ(Padding(2), expr.evaluate(0));
        EXPECT_EQ(Padding(2), expr.evaluate(10));
        EXPECT_EQ(Padding(4), expr.evaluate(11));
    }
    // categorical
    {
        auto expr = fromFunction(
            R"({"type": "categorical", "property": "foo", "stops": [[0, 2], [1, 4]], "default": 6})");
        EXPECT_EQ(Padding(2), evalInContext(expr, {{"foo", 0}}));
        EXPECT_EQ(Padding(4), evalInContext(expr, {{"foo", 1}}));
        EXPECT_EQ(Padding(6), evalInContext(expr, {{"foo", 2}}));
        EXPECT_EQ(Padding(6), evalInContext(expr, {{"bar", 0}}));
    }
    // identity
    {
        auto expr = fromFunction(R"({"type": "identity", "property": "foo", "default": [3, 7, 9, 11]})");
        EXPECT_EQ(Padding(2, 4, 6, 4), evalInContext(expr, {{"foo", std::vector<mbgl::Value>({2, 4, 6})}}));
        EXPECT_EQ(Padding(3), evalInContext(expr, {{"foo", 3}}));
        EXPECT_EQ(Padding(3, 7, 9, 11), evalInContext(expr, {{"bar", 3}}));
    }
}

TEST(Padding, OperatorBool) {
    Padding padding(0, 0, 0, 0);
    EXPECT_FALSE(padding);
    padding.left = 1;
    EXPECT_TRUE(padding);
}

TEST(Padding, OperatorEqual) {
    auto a = Padding({{3.5f, 9}});
    auto b = Padding(3.5, 9, 3.5, 9);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);

    a.left = 7;
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

TEST(Padding, OperatorMultiply) {
    auto padding = Padding(1, 2, 3, 4) * 2;
    EXPECT_EQ(Padding(2, 4, 6, 8), padding);

    padding = padding * 0.0;
    EXPECT_EQ(Padding(0), padding);
}
