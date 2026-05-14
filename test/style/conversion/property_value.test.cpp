#include <mbgl/test/util.hpp>

#include <mbgl/style/conversion/json.hpp>
#include <mbgl/style/conversion/property_value.hpp>
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

TEST(StyleConversion, PropertyValueVectorColor_PlainColorConstant) {
    Error error;
    JSDocument doc;
    doc.Parse<0>(R"("#222222")");
    auto result = convert<PropertyValue<std::vector<Color>>>(doc, error, false, false);
    ASSERT_TRUE(result) << "plain color constant must parse: " << error.message;
    ASSERT_TRUE(result->isConstant());
    ASSERT_EQ(result->asConstant().size(), 1u);
}
