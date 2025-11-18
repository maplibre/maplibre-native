#include <gtest/gtest.h>
#include <optional>
#include <string>

#include <mbgl/util/color.hpp>

using namespace mbgl;

void logUnexpectedValidResult(const std::string& input, const Color& color) {
    std::cerr << "Unexpected valid result for input: " << input << "\n";
    std::cerr << "Parsed Color: r = " << color.r << ", g = " << color.g << ", b = " << color.b << ", a = " << color.a
              << "\n";
}

const std::map<std::string, std::optional<Color>> testCases = {
    // Valid inputs
    {"#000000", Color(0.0f, 0.0f, 0.0f, 1.0f)},
    {"#FFFFFF", Color(1.0f, 1.0f, 1.0f, 1.0f)},
    {"#FF0000", Color(1.0f, 0.0f, 0.0f, 1.0f)},
    {"rgba(255, 0, 0, 1.0)", Color(1.0f, 0.0f, 0.0f, 1.0f)},
    {"rgb(0, 255, 0)", Color(0.0f, 1.0f, 0.0f, 1.0f)},
    {"blue", Color::blue()},
    {"red", Color::red()},
    {"rgba(0, 0, 0, 0)", Color(0.0f, 0.0f, 0.0f, 0.0f)},
    {"#123", Color(0.067f, 0.133f, 0.2f, 1.0f)},             // Short hex format
    {"rgb(-10, 0, 0)", Color(0.0f, 0.0f, 0.0f, 1.0f)},       // Clamped to 0
    {"rgba(300, 0, 0, 1.0)", Color(1.0f, 0.0f, 0.0f, 1.0f)}, // Clamped to 1
    {"rgba(100,100,100,0.2)", Color(20.0f / 255, 20.0f / 255, 20.0f / 255, 0.2f)},
    // {"#GGGGGG", Color(0.0f, 0.0f, 0.0f, 1.0f)}, // Treated as fallback black
    // not supported right now
    // {"#0F0F", Color(0.0f, 1.0f, 0.0f, 1.0f)},
    // {"#123F", Color(
    //     static_cast<float>(0x1) / (0xF + 1),
    //     static_cast<float>(0x2) / (0xF + 1),
    //     static_cast<float>(0x3) / (0xF + 1),
    //     static_cast<float>(0x4) / (0xF + 1)
    // )},

    // Invalid inputs
    {"not-a-color", std::nullopt},
    {"", std::nullopt},
};

TEST(ColorParse, AllCases) {
    constexpr float absError = 0.02f;
    for (const auto& [input, expectedResult] : testCases) {
        auto result = Color::parse(input);

        if (expectedResult.has_value()) {
            // Valid case: Check the values
            ASSERT_TRUE(result.has_value());
            EXPECT_NEAR(result->r, expectedResult->r, absError);
            EXPECT_NEAR(result->g, expectedResult->g, absError);
            EXPECT_NEAR(result->b, expectedResult->b, absError);
            EXPECT_NEAR(result->a, expectedResult->a, absError);
        } else {
            // Invalid case: Ensure no value is returned
            if (result.has_value()) {
                logUnexpectedValidResult(input, *result);
            }
            EXPECT_FALSE(result.has_value());
        }
    }
}
