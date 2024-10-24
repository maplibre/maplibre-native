#include <mbgl/test/util.hpp>

#include <mbgl/style/conversion/json.hpp>
#include <mbgl/style/conversion/constant.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/style/conversion/layer.hpp>
#include <mbgl/style/conversion/property_value.hpp>
#include <mbgl/style/variable_anchor_offset_collection.hpp>

#include <array>

using namespace mbgl;
using namespace mbgl::style;
using namespace mbgl::style::conversion;

TEST(VariableAnchorOffsetCollection, Calculations) {
    auto parseJson = [&](const std::string& src) {
        Error error;
        return convertJSON<VariableAnchorOffsetCollection>(src, error);
    };

    mbgl::util::Interpolator<VariableAnchorOffsetCollection> interpolator;

    // valid value
    {
        auto a = parseJson(R"(["top", [0, 0], "bottom", [2, 2]])");
        auto b = parseJson(R"(["top", [1, 1], "bottom", [4, 4]])");
        ASSERT_TRUE(a.has_value());
        ASSERT_TRUE(b.has_value());
        auto c = interpolator(*a, *b, 0.5);
        EXPECT_EQ(
            VariableAnchorOffsetCollection({{SymbolAnchorType::Top, {0.5, 0.5}}, {SymbolAnchorType::Bottom, {3, 3}}}),
            c);
    }

    // should thorw with mismatched endpoints
    {
        auto a = parseJson(R"(["top", [0, 0]])");
        auto b = parseJson(R"(["bottom", [1, 1]])");
        ASSERT_TRUE(a.has_value());
        ASSERT_TRUE(b.has_value());
        EXPECT_THROW(interpolator(*a, *b, 0.5), std::runtime_error);

        a = parseJson(R"(["top", [0, 0]])");
        b = parseJson(R"(["top", [1, 1], "bottom", [2, 2]])");
        ASSERT_TRUE(a.has_value());
        ASSERT_TRUE(b.has_value());
        EXPECT_THROW(interpolator(*a, *b, 0.5), std::runtime_error);
    }
}

TEST(VariableAnchorOffsetCollection, StyleConversion) {
    Error error;

    auto parseVariableAnchorOffsetCollection = [&](const std::string& src) {
        return convertJSON<VariableAnchorOffsetCollection>(src, error);
    };

    // valid value
    {
        auto jsonValue = R"(["left",[1.0,2.0],"top",[0.0,0.0]])";
        auto variableAnchorOffset = parseVariableAnchorOffsetCollection(jsonValue);
        ASSERT_TRUE(variableAnchorOffset.has_value());
        ASSERT_EQ(variableAnchorOffset->size(), 2);
        ASSERT_EQ(variableAnchorOffset->operator[](0).anchorType, style::SymbolAnchorType::Left);
        ASSERT_EQ(variableAnchorOffset->operator[](0).offset[0], 1);
        ASSERT_EQ(variableAnchorOffset->operator[](0).offset[1], 2);

        ASSERT_EQ(variableAnchorOffset->toString(), jsonValue);
    }

    // Invalid values
    {
        // null
        auto expectedError = "value must be an array";
        auto variableAnchorOffset = parseVariableAnchorOffsetCollection("{}");
        ASSERT_FALSE(variableAnchorOffset.has_value());
        ASSERT_EQ(error.message, expectedError);

        // empty array
        expectedError = "array must contain an even number of elements";
        variableAnchorOffset = parseVariableAnchorOffsetCollection("[]");
        ASSERT_FALSE(variableAnchorOffset.has_value());
        ASSERT_EQ(error.message, expectedError);

        // invalid anchor type
        expectedError = "anchor must be a valid anchor value";
        variableAnchorOffset = parseVariableAnchorOffsetCollection(R"(["test",  [1, 2]])");
        ASSERT_FALSE(variableAnchorOffset.has_value());
        ASSERT_EQ(error.message, expectedError);

        // invalid offset array
        expectedError = "anchor offset must be an array";
        variableAnchorOffset = parseVariableAnchorOffsetCollection(R"(["top", "left"])");
        ASSERT_FALSE(variableAnchorOffset.has_value());
        ASSERT_EQ(error.message, expectedError);

        expectedError = "anchor offset must have two elements";
        variableAnchorOffset = parseVariableAnchorOffsetCollection(R"(["top",  [1, 2, 3]])");
        ASSERT_FALSE(variableAnchorOffset.has_value());
        ASSERT_EQ(error.message, expectedError);

        expectedError = "anchor offset must have two numbers";
        variableAnchorOffset = parseVariableAnchorOffsetCollection(R"(["top",  ["test", "test"]])");
        ASSERT_FALSE(variableAnchorOffset.has_value());
        ASSERT_EQ(error.message, expectedError);
    }

    auto parseLayer = [&](const std::string& src) -> std::unique_ptr<Layer> {
        auto layer = convertJSON<std::unique_ptr<Layer>>(src, error);
        if (layer) return std::move(*layer);
        return nullptr;
    };

    // parse layer
    {
        auto layer = parseLayer(R"JSON({
            "type": "symbol",
            "id": "symbol",
            "source": "composite",
            "source-layer": "landmarks",
            "minzoom": 12,
            "maxzoom": 18,
            "layout": {
                "text-field": "Test Test Test",
                "icon-image": ["image", ["get", "landmark_image"]],
                "text-size": 24,
                "text-justify": "auto",
                "text-variable-anchor-offset": ["center", [0, 0],"top", [0, 0]]
            }
        })JSON");

        EXPECT_NE(nullptr, layer);
        auto value = layer->serialize();
        EXPECT_NE(nullptr, value.getObject());
        EXPECT_EQ(7u, value.getObject()->size());
    }
}

TEST(VariableAnchorOffsetCollection, Expression) {
    Error error;
    auto jsonToExpression = [&](const std::string& json) {
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

    auto jsonToPropertyValue = [&](const std::string& json) {
        return conversion::convertJSON<PropertyValue<VariableAnchorOffsetCollection>>(
            json, error, /*allowDataExpressions*/ true, /*convertTokens*/ false);
    };

    // invalid expressions
    {
        // Invalid component size
        auto expectedError =
            "[4]: Invalid variableAnchorOffset value [\"left\"]: expected an array containing an even number of "
            "values.";
        auto parsedResult = jsonToPropertyValue(
            R"(["interpolate", ["linear"], ["zoom"], 0, ["literal", ["left"]], 1, ["literal",["left",
                [10, 20]]]])");
        ASSERT_FALSE(parsedResult.has_value());
        ASSERT_EQ(error.message, expectedError);

        // Invalid offset array size
        expectedError =
            "[4]: Invalid variableAnchorOffset value [\"left\",[10]]: expected a string value as the first and an "
            "array with two elements as the second element of each pair.";
        parsedResult = jsonToPropertyValue(
            R"(["interpolate", ["linear"], ["zoom"], 0, ["literal", ["left", [10]]], 1, ["literal",["left",
                [10, 20]]]])");
        ASSERT_FALSE(parsedResult.has_value());
        ASSERT_EQ(error.message, expectedError);

        // Invalid anchor type
        expectedError =
            "[4]: Invalid variableAnchorOffsetCollection value [\"test\",[0,0]]: unknown anchor type 'test'";
        parsedResult = jsonToPropertyValue(
            R"(["interpolate", ["linear"], ["zoom"], 0, ["literal", ["test", [0, 0]]], 1, ["literal",["left",
                [10, 20]]]])");
        ASSERT_FALSE(parsedResult.has_value());
        ASSERT_EQ(error.message, expectedError);
    }
}
