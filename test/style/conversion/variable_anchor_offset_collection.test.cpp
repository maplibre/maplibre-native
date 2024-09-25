#include <mbgl/test/util.hpp>

#include <mbgl/style/conversion/json.hpp>
#include <mbgl/style/conversion/constant.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/util/variable_anchor_offset_collection.hpp>

// BUGBUG check headers
#include <mbgl/style/expression/dsl.hpp>
#include <mbgl/style/property_expression.hpp>
#include <mbgl/style/conversion/property_value.hpp>

#include <array>

using namespace mbgl;
using namespace mbgl::style;
using namespace mbgl::style::conversion;

// BUGBUG check if need both
using namespace style::expression;
using namespace style::expression::dsl;

TEST(StyleConversion, VariableAnchorOffsetCollection) {
    Error error;
    auto fromFunction = [](const char* json) {
        conversion::Error error;
        auto function = conversion::convertJSON<PropertyValue<VariableAnchorOffsetCollection>>(
            json, error, /*allowDataExpressions*/ true, /*convertTokens*/ false);
        return function->asExpression();
    };

    // BUGBUG
    // exponential
    {
        auto expr = fromFunction(
            R"({"type": "exponential", "stops": [[1, ["left", [0, 0]]], [11, ["left", [10, 20]]]]})");
        auto val = expr.evaluate(5);
        auto val1 = expr.evaluate(11);
        EXPECT_EQ(VariableAnchorOffsetCollection({{{SymbolAnchorType::Left}, {{0, 0}}}}), expr.evaluate(0));
        EXPECT_EQ(VariableAnchorOffsetCollection({{{SymbolAnchorType::Left}, {{4, 8}}}}), val); // BUGBUG check value
        EXPECT_EQ(VariableAnchorOffsetCollection({{{SymbolAnchorType::Left}, {{10, 20}}}}), val1);
    }

    //    // BUGBUG
    //    {
    //        auto json =
    //                R"(["interpolate", ["linear"], ["zoom"], 0, ["literal", ["left", [0, 0]]], 1, ["literal",["left",
    //                [10, 20]]]])";
    //        PropertyExpression<VariableAnchorOffsetCollection> expr(createExpression(json));
    //
    //        auto result = expr.evaluate(0.5f);
    //        EXPECT_EQ(VariableAnchorOffsetCollection(), result);
    //    }
}
