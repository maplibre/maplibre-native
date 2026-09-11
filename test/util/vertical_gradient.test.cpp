#include <mln/test/util.hpp>

#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/property_value.hpp>
#include <mln/style/conversion_impl.hpp>
#include <mln/style/property_expression.hpp>
#include <mln/util/vertical_gradient.hpp>

#include <span>
#include <stdexcept>
#include <vector>

using namespace mln;
using namespace mln::style;

namespace {

VerticalGradient fromArray(std::vector<float> values) {
    return VerticalGradient(std::span<const float>(values));
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

TEST(VerticalGradient, IsInRange) {
    auto inRange = [](std::vector<float> values) {
        return VerticalGradient::isInRange(std::span<const float>(values));
    };

    EXPECT_TRUE(inRange({0.0f}));
    EXPECT_TRUE(inRange({1.0f}));
    EXPECT_TRUE(inRange({0.5f, 0.0f}));
    EXPECT_TRUE(inRange({0.5f, 10000.0f}));

    EXPECT_FALSE(inRange({-0.1f}));
    EXPECT_FALSE(inRange({1.1f}));
    EXPECT_FALSE(inRange({-5.0f}));
    EXPECT_FALSE(inRange({0.5f, -1.0f}));

    EXPECT_TRUE(inRange({}));

    EXPECT_TRUE(inRange(std::vector<float>{VerticalGradient(true).depth, VerticalGradient(true).referenceHeight}));
    EXPECT_TRUE(inRange(std::vector<float>{VerticalGradient(false).depth, VerticalGradient(false).referenceHeight}));
}

TEST(VerticalGradient, CoercionRejectsOutOfRangeValues) {
    conversion::Error error;
    auto parse = [&](const char* json) {
        error = {};
        return conversion::convertJSON<PropertyValue<VerticalGradient>>(
            json, error, /*allowDataExpressions*/ false, /*convertTokens*/ false);
    };

    // In range
    {
        auto result = parse(R"(["to-verticalgradient", ["literal", [0.5, 80]]])");
        ASSERT_TRUE(result) << error.message;
        ASSERT_TRUE(result->isConstant());
        EXPECT_EQ(fromArray({0.5f, 80.0f}), result->asConstant());
    }
    // Out of range
    {
        EXPECT_FALSE(parse(R"(["to-verticalgradient", ["literal", [-5, 0]]])"));
        EXPECT_FALSE(error.message.empty());
    }
    {
        EXPECT_FALSE(parse(R"(["to-verticalgradient", ["literal", [0.5, -1]]])"));
        EXPECT_FALSE(error.message.empty());
    }
    {
        EXPECT_FALSE(parse(R"(["to-verticalgradient", ["literal", [0.5, 80, 1]]])"));
        EXPECT_FALSE(error.message.empty());
    }
}

TEST(VerticalGradient, ToArrayOrder) {
    const auto values = fromArray({0.25f, 80.0f}).toArray();
    EXPECT_EQ(0.25f, values[0]);
    EXPECT_EQ(80.0f, values[1]);
}

TEST(VerticalGradient, Serialize) {
    EXPECT_EQ(mln::Value(std::vector<mln::Value>{0.25f, 80.0f}), fromArray({0.25f, 80.0f}).serialize());

    EXPECT_EQ(mln::Value(std::vector<mln::Value>{0.0f, 0.0f}), VerticalGradient(false).serialize());
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
    ASSERT_TRUE(result->isExpression());

    const auto expr = result->asExpression();
    EXPECT_EQ(VerticalGradient(true), expr.evaluate(15.0f));
    EXPECT_EQ(fromArray({0.5f, 0.0f}), expr.evaluate(16.0f));
}
