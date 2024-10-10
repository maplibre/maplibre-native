#include <mbgl/test/util.hpp>

#include <mbgl/style/conversion/json.hpp>
#include <mbgl/style/conversion/constant.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/style/conversion/property_value.hpp>
#include <mbgl/util/variable_anchor_offset_collection.hpp>

#include <array>

using namespace mbgl;
using namespace mbgl::style;
using namespace mbgl::style::conversion;

TEST(StyleConversion, VariableAnchorOffsetCollection) {
    auto jsonToExpression = [](const char* json) {
        conversion::Error error;
        auto propertyValue = conversion::convertJSON<PropertyValue<VariableAnchorOffsetCollection>>(
            json, error, /*allowDataExpressions*/ true, /*convertTokens*/ false);
        return propertyValue->asExpression();
    };

    // interval function
    {
        auto expr = jsonToExpression(
            R"({"type": "interval", "stops": [[0, ["left", [0, 0]]], [5, ["left", [10, 20]]]]})");

        EXPECT_EQ(VariableAnchorOffsetCollection({{SymbolAnchorType::Left, {0, 0}}}), expr.evaluate(0));
        EXPECT_EQ(VariableAnchorOffsetCollection({{SymbolAnchorType::Left, {0, 0}}}), expr.evaluate(4));
        EXPECT_EQ(VariableAnchorOffsetCollection({{SymbolAnchorType::Left, {10, 20}}}), expr.evaluate(5));
    }
    // interpolate expression
    {
        auto expr = jsonToExpression(
            R"(["interpolate", ["linear"], ["zoom"], 0, ["literal", ["left", [0, 0]]], 1, ["literal",["left",
                [10, 20]]]])");
        auto result = expr.evaluate(0.5f);
        EXPECT_EQ(VariableAnchorOffsetCollection(std::vector<AnchorOffsetPair>{
                      {AnchorOffsetPair(SymbolAnchorType::Left, std::array<float, 2>{5.0f, 10.0f})}}),
                  result);
    }
}
