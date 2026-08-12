#include <mbgl/test/util.hpp>

#include <mbgl/style/conversion/json.hpp>
#include <mbgl/style/conversion/property_value.hpp>
#include <mbgl/style/property_expression.hpp>
#include <mbgl/util/color.hpp>
#include <mbgl/util/rapidjson.hpp>

#include <vector>

using namespace mbgl;
using namespace mbgl::style;
using namespace mbgl::style::conversion;

TEST(StyleConversion, PropertyValue) {
    // PropertyValue<T> accepts a constant expression:
    // https://github.com/mapbox/mapbox-gl-native/issues/11940
    Error error;
    JSDocument doc;
    doc.Parse<0>(R"(["literal", [1, 2]])");
    auto expected = std::array<float, 2>{{1, 2}};
    auto result = convert<PropertyValue<std::array<float, 2>>>(doc, error, false, false);
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->isConstant());
    ASSERT_EQ(result->asConstant(), expected);
}

// Regression: PR #3965 promoted hillshade-shadow-color and hillshade-highlight-color
// from PaintProperty<Color> to PaintProperty<std::vector<Color>> to support
// multidirectional hillshade. The style-spec types them as `colorArray` with
// single-color defaults ("#000000" / "#FFFFFF"), and maplibre-gl-js accepts both
// `color` and `array<color>` expression outputs for these properties.
//
// Before the fix in src/mbgl/style/conversion/property_value.cpp, parsing an
// `interpolate` expression over single-color outputs against a
// `PropertyValue<std::vector<Color>>` failed with
// "Expected array<color> but found string instead." — breaking nearly every
// published topographic basemap that animates hillshade color over zoom.

TEST(StyleConversion, PropertyValueVectorColor_SingleColorInterpolate) {
    Error error;
    JSDocument doc;
    doc.Parse<0>(R"(["interpolate", ["linear"], ["zoom"], 0, "#000000", 17, "#666666"])");
    auto result = convert<PropertyValue<std::vector<Color>>>(doc, error, false, false);
    ASSERT_TRUE(result) << "single-color interpolate must parse for colorArray properties: " << error.message;
    ASSERT_TRUE(result->isExpression());
}

TEST(StyleConversion, PropertyValueVectorColor_MultidirectionalConstant) {
    // Spec-defined multidirectional shape: a JSON array of color strings used as
    // a constant. Flows through the non-expression conversion path; must still
    // parse correctly after the single-color expression fallback was added.
    Error error;
    JSDocument doc;
    doc.Parse<0>(R"(["#000000", "#222222"])");
    auto result = convert<PropertyValue<std::vector<Color>>>(doc, error, false, false);
    ASSERT_TRUE(result) << "multidirectional constant array<color> must parse: " << error.message;
    ASSERT_TRUE(result->isConstant());
    ASSERT_EQ(result->asConstant().size(), 2u);
}

// PR #3965 also promoted hillshade-illumination-direction and
// hillshade-illumination-altitude from PaintProperty<float> to
// PaintProperty<std::vector<float>>. The style-spec types them as `numberArray`
// with single-number defaults (335 and 45). The same compile-time check that
// broke colorArray expressions also rejects spec-valid single-number
// interpolate expressions for these properties.

TEST(StyleConversion, PropertyValueVectorFloat_SingleNumberInterpolate) {
    Error error;
    JSDocument doc;
    doc.Parse<0>(R"(["interpolate", ["linear"], ["zoom"], 0, 335, 17, 200])");
    auto result = convert<PropertyValue<std::vector<float>>>(doc, error, false, false);
    ASSERT_TRUE(result) << "single-number interpolate must parse for numberArray properties: " << error.message;
    ASSERT_TRUE(result->isExpression());
}

TEST(StyleConversion, PropertyValueVectorFloat_MultidirectionalConstant) {
    Error error;
    JSDocument doc;
    doc.Parse<0>(R"([335, 45])");
    auto result = convert<PropertyValue<std::vector<float>>>(doc, error, false, false);
    ASSERT_TRUE(result) << "multidirectional constant array<number> must parse: " << error.message;
    ASSERT_TRUE(result->isConstant());
    ASSERT_EQ(result->asConstant().size(), 2u);
}

// Evaluation smoke tests: a parse-time success isn't useful if the runtime
// conversion path silently returns nullopt at every frame. These exercise the
// ValueConverter<std::vector<X>>::fromExpressionValue wrap for scalar outputs.

TEST(StyleConversion, PropertyValueVectorColor_SingleColorInterpolate_Evaluates) {
    Error error;
    JSDocument doc;
    doc.Parse<0>(R"(["interpolate", ["linear"], ["zoom"], 0, "#000000", 10, "#FFFFFF"])");
    auto result = convert<PropertyValue<std::vector<Color>>>(doc, error, false, false);
    ASSERT_TRUE(result) << error.message;
    ASSERT_TRUE(result->isExpression());

    const auto& expr = result->asExpression();
    auto v0 = expr.evaluate(0.0f);
    ASSERT_EQ(v0.size(), 1u);
    EXPECT_FLOAT_EQ(v0[0].r, 0.0f);
    EXPECT_FLOAT_EQ(v0[0].g, 0.0f);
    EXPECT_FLOAT_EQ(v0[0].b, 0.0f);

    auto v10 = expr.evaluate(10.0f);
    ASSERT_EQ(v10.size(), 1u);
    EXPECT_FLOAT_EQ(v10[0].r, 1.0f);
    EXPECT_FLOAT_EQ(v10[0].g, 1.0f);
    EXPECT_FLOAT_EQ(v10[0].b, 1.0f);
}

TEST(StyleConversion, PropertyValueVectorFloat_SingleNumberInterpolate_Evaluates) {
    Error error;
    JSDocument doc;
    doc.Parse<0>(R"(["interpolate", ["linear"], ["zoom"], 0, 335, 10, 200])");
    auto result = convert<PropertyValue<std::vector<float>>>(doc, error, false, false);
    ASSERT_TRUE(result) << error.message;
    ASSERT_TRUE(result->isExpression());

    const auto& expr = result->asExpression();
    auto v0 = expr.evaluate(0.0f);
    ASSERT_EQ(v0.size(), 1u);
    EXPECT_FLOAT_EQ(v0[0], 335.0f);

    auto v5 = expr.evaluate(5.0f);
    ASSERT_EQ(v5.size(), 1u);
    EXPECT_FLOAT_EQ(v5[0], 267.5f);

    auto v10 = expr.evaluate(10.0f);
    ASSERT_EQ(v10.size(), 1u);
    EXPECT_FLOAT_EQ(v10[0], 200.0f);
}
