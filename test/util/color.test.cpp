#include <gtest/gtest.h>
#include <optional>
#include <string>

#include <mln/util/color.hpp>

using namespace mln;

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
    {"hsl(120,100%,25%)", Color(0.0f, 0.5f, 0.0f, 1.0f)},
    {"hsl(240,0%,55%,0.2)", Color(0.11f, 0.11f, 0.11f, 0.2f)},
    // 4-digit hex with alpha (#RGBA)
    {"#0F0F", Color(0.0f, 1.0f, 0.0f, 1.0f)},
    {"#F00C", Color(0.8f, 0.0f, 0.0f, 0.8f)},     // Red with ~80% alpha
    {"#123F", Color(0.067f, 0.133f, 0.2f, 1.0f)}, // Same as #123 but with full alpha
    // 8-digit hex with alpha (#RRGGBBAA)
    {"#FF0000FF", Color(1.0f, 0.0f, 0.0f, 1.0f)}, // Red with full alpha
    {"#00FF0080", Color(0.0f, 0.5f, 0.0f, 0.5f)}, // Green with 50% alpha
    {"#0000FF00", Color(0.0f, 0.0f, 0.0f, 0.0f)}, // Blue with 0% alpha (transparent)

    // Invalid inputs
    {"not-a-color", std::nullopt},
    {"", std::nullopt},
    {"hsl(120,100%)", std::nullopt},
    {"hsl(120,100%,50%,1,0)", std::nullopt},
    {"hsla(120,100%,50%)", std::nullopt},
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

TEST(ColorParse, HSLWithAlphaMatchesHSLA) {
    const auto hsl = Color::parse("hsl(114,55%,73%,0.8)");
    const auto hsla = Color::parse("hsla(114,55%,73%,0.8)");

    ASSERT_TRUE(hsl.has_value());
    ASSERT_TRUE(hsla.has_value());
    EXPECT_FLOAT_EQ(hsl->r, hsla->r);
    EXPECT_FLOAT_EQ(hsl->g, hsla->g);
    EXPECT_FLOAT_EQ(hsl->b, hsla->b);
    EXPECT_FLOAT_EQ(hsl->a, hsla->a);
}

namespace {

void expectColor(const Color& actual, float r, float g, float b, float a) {
    EXPECT_NEAR(r, actual.r, 1e-5f);
    EXPECT_NEAR(g, actual.g, 1e-5f);
    EXPECT_NEAR(b, actual.b, 1e-5f);
    EXPECT_NEAR(a, actual.a, 1e-5f);
}

} // namespace

// Expected values come from GL JS's Color.interpolate (maplibre-gl-style-spec), premultiplied.
TEST(ColorInterpolate, Lab) {
    const Color red = *Color::parse("rgba(255,0,0,0.5)");
    const Color blue = Color::blue();
    expectColor(Color::interpolate(red, blue, 0.0, ColorSpace::LAB), 0.5f, 0.0f, 0.0f, 0.5f);
    expectColor(Color::interpolate(red, blue, 0.25, ColorSpace::LAB), 0.556105f, 0.0f, 0.1941047f, 0.625f);
    expectColor(Color::interpolate(red, blue, 0.5, ColorSpace::LAB), 0.5676148f, 0.0f, 0.4005062f, 0.75f);
    expectColor(Color::interpolate(red, blue, 1.0, ColorSpace::LAB), 0.0f, 0.0f, 1.0f, 1.0f);
    expectColor(Color::interpolate(Color::black(), Color::white(), 0.5, ColorSpace::LAB),
                0.4663266f,
                0.4663266f,
                0.4663266f,
                1.0f);
}

TEST(ColorInterpolate, Hcl) {
    const Color red = *Color::parse("rgba(255,0,0,0.5)");
    expectColor(Color::interpolate(red, Color::blue(), 0.5, ColorSpace::HCL), 0.7204395f, 0.0f, 0.3944121f, 0.75f);
    // A hueless start takes the end's hue.
    expectColor(
        Color::interpolate(Color::white(), Color::red(), 0.5, ColorSpace::HCL), 1.0f, 0.6241198f, 0.5030824f, 1.0f);
    // The hue takes the short way round.
    expectColor(
        Color::interpolate(*Color::parse("rgb(255,0,128)"), *Color::parse("rgb(255,128,0)"), 0.5, ColorSpace::HCL),
        1.0f,
        0.2827256f,
        0.298255f,
        1.0f);
}

TEST(ColorInterpolate, RgbIsTheExistingInterpolation) {
    const Color red = *Color::parse("rgba(255,0,0,0.5)");
    const Color mixed = Color::interpolate(red, Color::blue(), 0.5, ColorSpace::RGB);
    expectColor(mixed, 0.25f, 0.0f, 0.5f, 0.75f);
}
