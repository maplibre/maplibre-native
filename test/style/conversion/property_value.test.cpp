#include <mln/test/util.hpp>

#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/property_value.hpp>
#include <mln/util/rapidjson.hpp>
#include <mln/test/stub_geometry_tile_feature.hpp>
#include <mln/style/variable_anchor_offset_collection.hpp>
#include <mln/style/expression/elevation_interpolation.hpp>
#include <mln/style/layers/fill_extrusion_layer.hpp>
#include <mln/style/layers/fill_layer.hpp>

using namespace mln;
using namespace mln::style;
using namespace mln::style::conversion;

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

TEST(StyleConversion, SemiliteralOffsets) {
    Error error;
    JSDocument doc;
    doc.Parse(R"(["semiliteral", [["number", ["get", "x"]], ["*", 2, ["get", "y"]]]])");
    auto result = convert<PropertyValue<std::array<float, 2>>>(doc, error, true, false);
    ASSERT_TRUE(result) << error.message;
    ASSERT_TRUE(result->isExpression());
    StubGeometryTileFeature feature{PropertyMap{{"x", 3.0}, {"y", 4.0}}};
    EXPECT_EQ(result->asExpression().evaluate(feature, {}), (std::array<float, 2>{3, 8}));

    doc.Parse(
        R"(["semiliteral", ["top", ["semiliteral", [["number", ["get", "x"]], 2]], "bottom", ["literal", [0, 1]]]])");
    auto anchors = convert<PropertyValue<VariableAnchorOffsetCollection>>(doc, error, true, false);
    ASSERT_TRUE(anchors) << error.message;
    ASSERT_TRUE(anchors->isExpression());
    auto evaluated = anchors->asExpression().evaluate(feature, {});
    ASSERT_EQ(evaluated.size(), 2u);
    EXPECT_EQ(evaluated[0].anchorType, SymbolAnchorType::Top);
    EXPECT_EQ(evaluated[0].offset, (std::array<float, 2>{3, 2}));
    EXPECT_EQ(evaluated[1].anchorType, SymbolAnchorType::Bottom);
    EXPECT_EQ(evaluated[1].offset, (std::array<float, 2>{0, 1}));
}

TEST(StyleConversion, ElevationColorExpression) {
    // fill-extrusion-color over ["elevation"] stays an expression so the renderer can sample it by height
    Error error;
    JSDocument doc;
    doc.Parse(R"(["interpolate", ["linear"], ["elevation"], 0, "black", 10, "white"])");
    auto result = convert<PropertyValue<Color>>(doc, error, true, false, /*allowElevation=*/true);
    ASSERT_TRUE(result) << error.message;
    ASSERT_TRUE(result->isExpression());
    const auto& expression = result->asExpression();
    EXPECT_TRUE(expression.isFeatureConstant());
    EXPECT_TRUE(expression.isZoomConstant());

    // Without an elevation it evaluates to the color at the ground
    EXPECT_EQ(Color::black(), expression.evaluate(14.0f));

    const auto atHeight = [&](double height) {
        return *expression::fromExpressionValue<Color>(
            *expression.getExpression().evaluate(expression::EvaluationContext({}, nullptr, {height})));
    };
    EXPECT_EQ(Color::black(), atHeight(0));
    EXPECT_FLOAT_EQ(0.5f, atHeight(5).r);
    EXPECT_EQ(Color::white(), atHeight(10));
}

TEST(StyleConversion, ElevationRules) {
    Error error;
    const auto parse = [&](const char* json, bool allowElevation) {
        JSDocument doc;
        doc.Parse(json);
        error = {};
        return convert<PropertyValue<Color>>(doc, error, true, false, allowElevation);
    };

    // Only properties that opt in (fill-extrusion-color) accept ["elevation"]
    const auto feature = R"(["interpolate", ["linear"], ["elevation"], 0, "black", 10, "white"])";
    EXPECT_FALSE(parse(feature, false));
    EXPECT_EQ(R"("elevation" expressions are only supported in color-relief-color and fill-extrusion-color)",
              error.message);
    EXPECT_TRUE(parse(feature, true)) << error.message;

    // Without feature data any expression of the height works, e.g. floors
    EXPECT_TRUE(parse(R"(["step", ["%", ["elevation"], 3.5], "white", 3, "black"])", true)) << error.message;
    // ... and so does zoom as the outermost input
    EXPECT_TRUE(parse(
        R"(["interpolate", ["linear"], ["zoom"], 14, ["interpolate", ["linear"], ["elevation"], 0, "black", 10, "white"],
                                                                18, "white"])",
        true))
        << error.message;

    // With feature data, ["elevation"] must be the input of a top-level interpolate
    EXPECT_TRUE(
        parse(R"(["interpolate", ["linear"], ["elevation"], 0, ["to-color", ["get", "a"]], 10, "white"])", true))
        << error.message;
    EXPECT_FALSE(parse(R"(["step", ["%", ["elevation"], 3.5], ["to-color", ["get", "a"]], 3, "black"])", true));
    EXPECT_EQ(R"(With feature data, "elevation" may only be used as input to a top-level "interpolate" expression)",
              error.message);
    EXPECT_FALSE(parse(
        R"(["interpolate", ["linear"], ["*", ["elevation"], 2], 0, ["to-color", ["get", "a"]], 10, "white"])", true));
}

TEST(StyleConversion, ElevationInterpolationStops) {
    using namespace mln::style::expression;
    Error error;
    const auto parse = [&](const char* json) {
        JSDocument doc;
        doc.Parse(json);
        auto result = convert<PropertyValue<Color>>(doc, error, true, false, true);
        EXPECT_TRUE(result && result->isExpression()) << error.message;
        return result;
    };

    const auto twoStops = parse(
        R"(["interpolate", ["linear"], ["elevation"], 2, ["to-color", ["get", "a"]], 10, "white"])");
    const auto stops = elevationInterpolationStops(twoStops->asExpression().getExpression());
    ASSERT_TRUE(stops);
    EXPECT_EQ(2.0f, stops->min);
    EXPECT_EQ(10.0f, stops->max);
    EXPECT_TRUE(isExactElevationInterpolation(twoStops->asExpression().getExpression()));

    // More stops, or a curve, are only approximated by blending the first and last stop linearly
    const auto threeStops = parse(
        R"(["interpolate", ["linear"], ["elevation"], 0, ["to-color", ["get", "a"]], 2, "gray", 10, "white"])");
    EXPECT_EQ(10.0f, elevationInterpolationStops(threeStops->asExpression().getExpression())->max);
    EXPECT_FALSE(isExactElevationInterpolation(threeStops->asExpression().getExpression()));
    const auto exponential = parse(
        R"(["interpolate", ["exponential", 2], ["elevation"], 0, ["to-color", ["get", "a"]], 10, "white"])");
    EXPECT_FALSE(isExactElevationInterpolation(exponential->asExpression().getExpression()));

    // Not an interpolation over elevation
    const auto zoom = parse(R"(["interpolate", ["linear"], ["zoom"], 0, ["to-color", ["get", "a"]], 10, "white"])");
    EXPECT_FALSE(elevationInterpolationStops(zoom->asExpression().getExpression()));
}

TEST(StyleConversion, ElevationOnlyInFillExtrusionColor) {
    JSDocument doc;
    doc.Parse(R"(["interpolate", ["linear"], ["elevation"], 0, "black", 10, "white"])");

    const JSValue* value = &doc;

    FillExtrusionLayer extrusion("extrusion", "source");
    EXPECT_FALSE(extrusion.setProperty("fill-extrusion-color", Convertible(value)));

    FillLayer fill("fill", "source");
    const auto error = fill.setProperty("fill-color", Convertible(value));
    ASSERT_TRUE(error);
    EXPECT_EQ(R"("elevation" expressions are only supported in color-relief-color and fill-extrusion-color)",
              error->message);
}
