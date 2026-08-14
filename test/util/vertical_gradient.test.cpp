#include <mbgl/test/util.hpp>

#include <mbgl/style/conversion/json.hpp>
#include <mbgl/style/conversion/property_value.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/style/property_expression.hpp>
#include <mbgl/util/vertical_gradient.hpp>

#include <stdexcept>
#include <vector>

using namespace mbgl;
using namespace mbgl::style;

namespace {

VerticalGradient fromArray(std::vector<float> values) {
    return VerticalGradient(values);
}

} // namespace

TEST(VerticalGradient, DefaultIsLegacy) {
    const VerticalGradient defaultConstructed;
    EXPECT_EQ(VerticalGradient::defaultDepth, defaultConstructed.depth);
    EXPECT_EQ(VerticalGradient::legacyReferenceHeight, defaultConstructed.referenceHeight);
    EXPECT_EQ(defaultConstructed, VerticalGradient(true));
}

TEST(VerticalGradient, FalseZeroesBothFields) {
    const VerticalGradient off(false);
    EXPECT_EQ(0.0f, off.depth);
    EXPECT_EQ(0.0f, off.referenceHeight);
}

TEST(VerticalGradient, ArrayDisablesHeightScalingByDefault) {
    const auto oneElement = fromArray({0.25f});
    EXPECT_EQ(0.25f, oneElement.depth);
    EXPECT_EQ(0.0f, oneElement.referenceHeight);

    const auto twoElements = fromArray({0.25f, 80.0f});
    EXPECT_EQ(0.25f, twoElements.depth);
    EXPECT_EQ(80.0f, twoElements.referenceHeight);
}

TEST(VerticalGradient, ArrayLengthIsValidated) {
    EXPECT_THROW(fromArray({}), std::invalid_argument);
    EXPECT_THROW(fromArray({0.5f, 80.0f, 1.0f}), std::invalid_argument);
}

TEST(VerticalGradient, ArrayCanReproduceLegacy) {
    EXPECT_EQ(VerticalGradient(true),
              fromArray({VerticalGradient::defaultDepth, VerticalGradient::legacyReferenceHeight}));
    EXPECT_NE(VerticalGradient(true), VerticalGradient(false));
    EXPECT_NE(fromArray({0.5f}), fromArray({0.5f, 150.0f}));
}

TEST(VerticalGradient, ToArrayOrder) {
    const auto values = fromArray({0.25f, 80.0f}).toArray();
    EXPECT_EQ(0.25f, values[0]);
    EXPECT_EQ(80.0f, values[1]);
}

TEST(VerticalGradient, Serialize) {
    EXPECT_EQ(mbgl::Value(std::vector<mbgl::Value>{0.25f, 80.0f}), fromArray({0.25f, 80.0f}).serialize());

    EXPECT_EQ(mbgl::Value(std::vector<mbgl::Value>{0.0f, 0.0f}), VerticalGradient(false).serialize());
}

TEST(VerticalGradient, InterpolateExpressionIsRejected) {
    conversion::Error error;
    auto result = conversion::convertJSON<PropertyValue<VerticalGradient>>(
        R"(["interpolate", ["linear"], ["zoom"], 0, ["to-verticalgradient", true], 16, ["to-verticalgradient", ["literal", [0.5, 0]]]])",
        error,
        /*allowDataExpressions*/ false,
        /*convertTokens*/ false);
    EXPECT_FALSE(result);
}

TEST(VerticalGradient, StepExpressionOverZoom) {
    conversion::Error error;
    auto result = conversion::convertJSON<PropertyValue<VerticalGradient>>(
        R"(["step", ["zoom"], ["to-verticalgradient", true], 16, ["to-verticalgradient", ["literal", [0.5, 0]]]])",
        error,
        /*allowDataExpressions*/ false,
        /*convertTokens*/ false);
    ASSERT_TRUE(result) << error.message;

    const auto expr = result->asExpression();
    EXPECT_EQ(VerticalGradient(true), expr.evaluate(15.0f));
    EXPECT_EQ(fromArray({0.5f, 0.0f}), expr.evaluate(16.0f));
}
