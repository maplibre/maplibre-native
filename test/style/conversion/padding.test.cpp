#include <mbgl/test/util.hpp>

#include <mbgl/style/conversion/json.hpp>
#include <mbgl/style/conversion/constant.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/util/padding.hpp>

#include <array>

using namespace mbgl;
using namespace mbgl::style;
using namespace mbgl::style::conversion;

TEST(StyleConversion, Padding) {
    Error error;
    auto parsePadding = [&](const std::string& src) {
        return convertJSON<Padding>(src, error);
    };

    {
        // Single value is applied to all sides
        auto padding = parsePadding("3");
        ASSERT_TRUE(padding.has_value());
        ASSERT_EQ(padding->top, 3);
        ASSERT_EQ(padding->right, 3);
        ASSERT_EQ(padding->bottom, 3);
        ASSERT_EQ(padding->left, 3);
    }
    {
        // Two values apply to [top/bottom, left/right]
        auto padding = parsePadding("[3.5, 4.5]");
        ASSERT_TRUE(padding.has_value());
        ASSERT_EQ(padding->top, 3.5);
        ASSERT_EQ(padding->bottom, 3.5);
        ASSERT_EQ(padding->left, 4.5);
        ASSERT_EQ(padding->right, 4.5);
    }
    {
        // Three values apply to [top, left/right, bottom] e.g. [2, 3, 1];
        auto padding = parsePadding("[2, 3, 1]");
        ASSERT_TRUE(padding.has_value());
        ASSERT_EQ(padding->top, 2);
        ASSERT_EQ(padding->left, 3);
        ASSERT_EQ(padding->right, 3);
        ASSERT_EQ(padding->bottom, 1);
    }
    {
        // Four values apply to [top, right, bottom, left], e.g. [2, 3, 1, 0].
        auto padding = parsePadding("[-2.5, -3.5, -1.0, -0.5]");
        ASSERT_TRUE(padding.has_value());
        ASSERT_EQ(padding->top, -2.5);
        ASSERT_EQ(padding->right, -3.5);
        ASSERT_EQ(padding->bottom, -1.0);
        ASSERT_EQ(padding->left, -0.5);
    }

    // Invalid values

    const auto expectedError = "value must be a number or an array of numbers (between 1 and 4 elements)";
    {
        // null
        auto padding = parsePadding("null");
        ASSERT_FALSE(padding.has_value());
        ASSERT_EQ(error.message, expectedError);
    }
    {
        // bool
        auto padding = parsePadding("false");
        ASSERT_FALSE(padding.has_value());
        ASSERT_EQ(error.message, expectedError);
    }
    {
        // empty array
        auto padding = parsePadding("[]");
        ASSERT_FALSE(padding.has_value());
        ASSERT_EQ(error.message, expectedError);
    }
    {
        // too long array
        auto padding = parsePadding("[1, 2, 3, 4, 5]");
        ASSERT_FALSE(padding.has_value());
        ASSERT_EQ(error.message, expectedError);
    }
    {
        // array of wrong types
        auto padding = parsePadding("[true, false]");
        ASSERT_FALSE(padding.has_value());
        ASSERT_EQ(error.message, expectedError);
    }
    {
        // array with null
        auto padding = parsePadding("[null]");
        ASSERT_FALSE(padding.has_value());
        ASSERT_EQ(error.message, expectedError);
    }
    {
        // object
        auto padding = parsePadding("{\"foo\": 1}");
        ASSERT_FALSE(padding.has_value());
        ASSERT_EQ(error.message, expectedError);
    }
}
