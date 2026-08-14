#include <mbgl/test/util.hpp>

#include <mbgl/style/conversion/constant.hpp>
#include <mbgl/style/conversion/json.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/util/vertical_gradient.hpp>

using namespace mbgl;
using namespace mbgl::style;
using namespace mbgl::style::conversion;

TEST(StyleConversion, VerticalGradient) {
    Error error;
    auto parseVerticalGradient = [&](const std::string& src) {
        error = {};
        return convertJSON<VerticalGradient>(src, error);
    };

    {
        // `true` selects the legacy height-scaled gradient.
        auto gradient = parseVerticalGradient("true");
        ASSERT_TRUE(gradient.has_value());
        ASSERT_EQ(VerticalGradient::defaultDepth, gradient->depth);
        ASSERT_EQ(VerticalGradient::legacyReferenceHeight, gradient->referenceHeight);
    }
    {
        // `false` zeroes both fields, which the shader evaluates to an exact no-op.
        auto gradient = parseVerticalGradient("false");
        ASSERT_TRUE(gradient.has_value());
        ASSERT_EQ(0.0f, gradient->depth);
        ASSERT_EQ(0.0f, gradient->referenceHeight);
    }
    {
        // One element sets depth and leaves height scaling off.
        auto gradient = parseVerticalGradient("[0.25]");
        ASSERT_TRUE(gradient.has_value());
        ASSERT_EQ(0.25f, gradient->depth);
        ASSERT_EQ(0.0f, gradient->referenceHeight);
    }
    {
        // Two elements are [depth, referenceHeight].
        auto gradient = parseVerticalGradient("[0.25, 80]");
        ASSERT_TRUE(gradient.has_value());
        ASSERT_EQ(0.25f, gradient->depth);
        ASSERT_EQ(80.0f, gradient->referenceHeight);
    }
    {
        // An empty array has no depth to apply.
        ASSERT_FALSE(parseVerticalGradient("[]").has_value());
        ASSERT_FALSE(error.message.empty());
    }
    {
        // Three elements: `falloff` was removed from the API, so this must not silently pass.
        ASSERT_FALSE(parseVerticalGradient("[0.25, 80, 1]").has_value());
        ASSERT_FALSE(error.message.empty());
    }
    {
        // Non-numeric members.
        ASSERT_FALSE(parseVerticalGradient(R"([0.25, "80"])").has_value());
        ASSERT_FALSE(error.message.empty());
    }
    {
        ASSERT_FALSE(parseVerticalGradient(R"("round")").has_value());
        ASSERT_FALSE(error.message.empty());
    }
    {
        ASSERT_FALSE(parseVerticalGradient("{}").has_value());
        ASSERT_FALSE(error.message.empty());
    }
    {
        // Unlike Padding -- which this converter is modelled on -- a bare number is NOT
        // accepted. The property is boolean-or-array, and `toBool` requires a JSON boolean.
        ASSERT_FALSE(parseVerticalGradient("5").has_value());
        ASSERT_FALSE(error.message.empty());
    }
}

TEST(StyleConversion, VerticalGradientReportsErrorsWithoutThrowing) {
    Error error;

    EXPECT_NO_THROW({
        auto tooMany = convertJSON<VerticalGradient>("[0.1, 0.2, 0.3, 0.4]", error);
        EXPECT_FALSE(tooMany.has_value());
    });

    EXPECT_NO_THROW({
        auto empty = convertJSON<VerticalGradient>("[]", error);
        EXPECT_FALSE(empty.has_value());
    });
}
