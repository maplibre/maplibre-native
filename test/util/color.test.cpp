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

// Expected values come from colorjs.io, by the CSS Color 4 editors, premultiplied.
TEST(ColorParse, Oklch) {
    expectColor(Color::parse("oklch(70% 0.1 30)").value(), 0.8367361f, 0.5266265f, 0.4751982f, 1.0f);
    expectColor(Color::parse("oklch(0.8 0.05 160 / 0.75)").value(), 0.4785853f, 0.588708f, 0.5258182f, 0.75f);
    for (const auto* input : {"oklch(0.6 0.1 90)",
                              "oklch(60% 25% 90deg)",
                              "oklch(0.6 0.1 100grad)",
                              "oklch(0.6 0.1 1.5707963267948966rad)",
                              "oklch(0.6 0.1 0.25turn)",
                              "oklch(0.6 0.1 450)",
                              "oklch(0.6 0.1 -270deg)",
                              "OKLCH(60% 0.1 90DEG)",
                              "oklch(6e-1 1E-1 +9e1)",
                              "oklch(+.6 .1 90 / 1)",
                              " oklch(  0.6\t0.1\n90  /  100% ) "}) {
        SCOPED_TRACE(input);
        expectColor(Color::parse(input).value(), 0.5937185f, 0.4908275f, 0.1901161f, 1.0f);
    }
    expectColor(Color::parse("oklch(0.6 0.1 90 / 50%)").value(), 0.2968593f, 0.2454138f, 0.09505807f, 0.5f);
}

TEST(ColorParse, OklchClampsAndReadsNoneAsZero) {
    const Color lightest = Color::parse("oklch(1 0.1 30)").value();
    const Color darkest = Color::parse("oklch(0 0.1 30)").value();
    expectColor(Color::parse("oklch(1.2 0.1 30)").value(), lightest.r, lightest.g, lightest.b, lightest.a);
    expectColor(Color::parse("oklch(-0.1 0.1 30)").value(), darkest.r, darkest.g, darkest.b, darkest.a);
    expectColor(Color::parse("oklch(0.6 -0.1 30)").value(), 0.5021107f, 0.5021107f, 0.5021107f, 1.0f);
    expectColor(Color::parse("oklch(0.6 0.1 90 / 1.5)").value(), 0.5937185f, 0.4908275f, 0.1901161f, 1.0f);
    expectColor(Color::parse("oklch(0.6 0.1 90 / -1)").value(), 0.0f, 0.0f, 0.0f, 0.0f);
    const Color zeroLightness = Color::parse("oklch(0 0.1 90)").value();
    expectColor(
        Color::parse("oklch(none 0.1 90)").value(), zeroLightness.r, zeroLightness.g, zeroLightness.b, zeroLightness.a);
    expectColor(Color::parse("oklch(0.6 none 90)").value(), 0.5021107f, 0.5021107f, 0.5021107f, 1.0f);
    const Color zeroHue = Color::parse("oklch(0.6 0.1 0)").value();
    expectColor(Color::parse("oklch(0.6 0.1 none)").value(), zeroHue.r, zeroHue.g, zeroHue.b, zeroHue.a);
    expectColor(Color::parse("oklch(0.6 0.1 90 / none)").value(), 0.0f, 0.0f, 0.0f, 0.0f);
}

TEST(ColorParse, OklchWrapsAHugeHueAngleBeforeConvertingIt) {
    const Color zeroHue = Color::parse("oklch(0.6 0.1 0)").value();
    expectColor(Color::parse("oklch(0.6 0.1 1e306turn)").value(), zeroHue.r, zeroHue.g, zeroHue.b, zeroHue.a);
    EXPECT_TRUE(Color::parse("oklch(0.6 0.1 1e307rad)"));
}

TEST(ColorParse, OklchOutsideSrgbIsClipped) {
    expectColor(Color::parse("oklch(0.7 0.3 140)").value(), 0.0f, 0.7715854f, 0.0f, 1.0f);
    expectColor(Color::parse("oklch(0.9 0.3 300)").value(), 1.0f, 0.6255093f, 1.0f, 1.0f);
    expectColor(Color::parse("oklch(0.3 0.25 30)").value(), 0.5091695f, 0.0f, 0.0f, 1.0f);
    // A huge chroma converts as chroma 1e6, which stays finite.
    expectColor(Color::parse("oklch(0.6 1e110 90)").value(), 0.0f, 1.0f, 0.0f, 1.0f);
}

TEST(ColorParse, OklchInvalid) {
    for (const auto* input : {"oklch(0.6, 0.1, 90)",
                              "oklch(0.6 0.1 90, 0.5)",
                              "oklch()",
                              "oklch(0.6 0.1)",
                              "oklch(0.6 0.1 90 0.5)",
                              "oklch(0.6 0.1 90 /)",
                              "oklch(0.6 0.1 90 / 0.5 / 0.5)",
                              "oklch(0.6 0.1 90%)",
                              "oklch(0.6 0.1 90 deg)",
                              "oklch(0.6 0.1 90grads)",
                              "oklch(0.6 0.1 nonedeg)",
                              "oklch(0.6deg 0.1 90)",
                              "oklch(0.6 0.1deg 90)",
                              "oklch(1. 0.1 90)",
                              "oklch(0.6 0.1 --90)",
                              "oklch(0.6 0.1 9e)",
                              "oklch(0.6 0.1 1e999)",
                              "oklch (0.6 0.1 90)",
                              "oklch(0.6 0.1 90",
                              "oklch(0.6 0.1 90))"}) {
        SCOPED_TRACE(input);
        EXPECT_FALSE(Color::parse(input).has_value());
    }
}

TEST(ColorParse, Oklab) {
    expectColor(Color::parse("oklab(70% 0.1 0.05)").value(), 0.8600103f, 0.5100676f, 0.4739938f, 1.0f);
    expectColor(Color::parse("oklab(0.8 -0.04 0.02 / 0.75)").value(), 0.4958492f, 0.5841786f, 0.5192557f, 0.75f);
    for (const auto* input : {"oklab(0.6 0.1 0.1)",
                              "oklab(60% 25% 25%)",
                              "OKLAB(60% 0.1 0.1)",
                              "oklab(6e-1 1E-1 +1e-1)",
                              "oklab(+.6 .1 .1 / 1)",
                              " oklab(  0.6\t0.1\n0.1  /  100% ) "}) {
        SCOPED_TRACE(input);
        expectColor(Color::parse(input).value(), 0.7646601f, 0.3764679f, 0.1815436f, 1.0f);
    }
    expectColor(Color::parse("oklab(0.6 -25% 25%)").value(), 0.3483465f, 0.5729275f, 0.1985555f, 1.0f);
    expectColor(Color::parse("oklab(0.6 0.1 0.1 / 50%)").value(), 0.3823301f, 0.188234f, 0.09077179f, 0.5f);
}

TEST(ColorParse, OklabClampsAndReadsNoneAsZero) {
    expectColor(Color::parse("oklab(1.2 0.1 0)").value(), 1.0f, 0.8913625f, 0.9894156f, 1.0f);
    const Color darkest = Color::parse("oklab(0 0.1 0.1)").value();
    expectColor(darkest, 0.004430535f, 0.007690759f, 0.0f, 1.0f);
    expectColor(Color::parse("oklab(-0.1 0.1 0.1)").value(), darkest.r, darkest.g, darkest.b, darkest.a);
    expectColor(Color::parse("oklab(none 0.1 0.1)").value(), darkest.r, darkest.g, darkest.b, darkest.a);
    expectColor(Color::parse("oklab(0.6 none none)").value(), 0.5021107f, 0.5021107f, 0.5021107f, 1.0f);
    expectColor(Color::parse("oklab(0.6 0.1 0.1 / 1.5)").value(), 0.7646601f, 0.3764679f, 0.1815436f, 1.0f);
    expectColor(Color::parse("oklab(0.6 0.1 0.1 / -1)").value(), 0.0f, 0.0f, 0.0f, 0.0f);
    expectColor(Color::parse("oklab(0.6 0.1 0.1 / none)").value(), 0.0f, 0.0f, 0.0f, 0.0f);
}

TEST(ColorParse, OklabOutsideSrgbIsClipped) {
    expectColor(Color::parse("oklab(0.7 -0.3 0.3)").value(), 0.0f, 0.7911692f, 0.0f, 1.0f);
    expectColor(Color::parse("oklab(0.4 0.1 -0.3)").value(), 0.3460255f, 0.0f, 0.8846979f, 1.0f);
    // A huge a and b convert as chroma 1e6 in their direction, which stays finite.
    expectColor(Color::parse("oklab(0.6 0 1e110)").value(), 0.0f, 1.0f, 0.0f, 1.0f);
    expectColor(Color::parse("oklab(0.6 -1e300 -1e300)").value(), 0.0f, 0.0f, 1.0f, 1.0f);
}

TEST(ColorParse, OklabInvalid) {
    for (const auto* input : {"oklab(0.6, 0.1, 0.1)",
                              "oklab()",
                              "oklab(0.6 0.1)",
                              "oklab(0.6 0.1 0.1 0.5)",
                              "oklab(0.6 0.1 0.1 /)",
                              "oklab(0.6 0.1 0.1 / 0.5 / 0.5)",
                              "oklab(0.6 0.1 0.1deg)",
                              "oklab(0.6deg 0.1 0.1)",
                              "oklab(0.6 none% 0.1)",
                              "oklab(1. 0.1 0.1)",
                              "oklab(0.6 --0.1 0.1)",
                              "oklab(0.6 0.1 9e)",
                              "oklab(0.6 1e999 0.1)",
                              "oklab (0.6 0.1 0.1)",
                              "oklab(0.6 0.1 0.1",
                              "oklab(0.6 0.1 0.1))",
                              "oklab(a b c)"}) {
        SCOPED_TRACE(input);
        EXPECT_FALSE(Color::parse(input).has_value());
    }
}

TEST(ColorInterpolate, Oklch) {
    const Color blue = Color::blue();
    const Color green = Color::parse("rgba(0,255,0,.6)").value();
    expectColor(Color::interpolate(blue, green, 0.25, ColorSpace::OKLCH), 0.0f, 0.3866815f, 0.8861813f, 0.9f);
    expectColor(Color::interpolate(blue, green, 0.5, ColorSpace::OKLCH), 0.0f, 0.5325832f, 0.6610553f, 0.8f);
    expectColor(Color::interpolate(blue, green, 0.75, ColorSpace::OKLCH), 0.0f, 0.5941883f, 0.3981223f, 0.7f);
    // The hue takes the short way round, across 0.
    expectColor(Color::interpolate(Color::parse("oklch(0.6 0.15 350)").value(),
                                   Color::parse("oklch(0.7 0.12 20)").value(),
                                   0.5,
                                   ColorSpace::OKLCH),
                0.8222712f,
                0.409868f,
                0.5177325f,
                1.0f);
}

TEST(ColorInterpolate, OklchGrayTakesTheOtherHue) {
    const Color gray = Color::parse("#808080").value();
    const Color blue = Color::parse("oklch(0.6 0.12 250)").value();
    expectColor(Color::interpolate(gray, blue, 0.5, ColorSpace::OKLCH), 0.3955842f, 0.5149412f, 0.6398697f, 1.0f);
    expectColor(Color::interpolate(blue, gray, 0.5, ColorSpace::OKLCH), 0.3955842f, 0.5149412f, 0.6398697f, 1.0f);
}

TEST(ColorInterpolate, OklchPremultipliesLightnessAndChroma) {
    expectColor(Color::interpolate(Color::parse("oklch(0.7 0.1 30 / 0.2)").value(),
                                   Color::parse("oklch(0.5 0.1 250)").value(),
                                   0.5,
                                   ColorSpace::OKLCH),
                0.315826f,
                0.2084958f,
                0.3403197f,
                0.6f);
    expectColor(Color::interpolate(Color(), Color::parse("oklch(0.7 0.1 150)").value(), 0.5, ColorSpace::OKLCH),
                0.2181294f,
                0.345293f,
                0.2453651f,
                0.5f);
    expectColor(Color::interpolate(Color(), Color(), 0.5, ColorSpace::OKLCH), 0.0f, 0.0f, 0.0f, 0.0f);
}

TEST(ColorInterpolate, Oklab) {
    const Color blue = Color::blue();
    const Color green = Color::parse("rgba(0,255,0,.6)").value();
    expectColor(Color::interpolate(blue, green, 0.25, ColorSpace::OKLAB), 0.0f, 0.3353049f, 0.8340296f, 0.9f);
    expectColor(Color::interpolate(blue, green, 0.5, ColorSpace::OKLAB), 0.0f, 0.4552392f, 0.6574173f, 0.8f);
    expectColor(Color::interpolate(blue, green, 0.75, ColorSpace::OKLAB), 0.0f, 0.5368223f, 0.4533104f, 0.7f);
    expectColor(Color::interpolate(Color::red(), Color::blue(), 0.5, ColorSpace::OKLAB),
                0.5504411f,
                0.3256207f,
                0.6365007f,
                1.0f);
}

TEST(ColorInterpolate, OklabPremultipliesLightnessAAndB) {
    expectColor(Color::interpolate(Color::parse("oklab(0.7 0.1 0.05 / 0.2)").value(),
                                   Color::parse("oklab(0.5 -0.05 -0.1)").value(),
                                   0.5,
                                   ColorSpace::OKLAB),
                0.172782f,
                0.2625925f,
                0.360832f,
                0.6f);
    expectColor(Color::interpolate(Color(), Color::parse("oklab(0.7 -0.1 0.05)").value(), 0.5, ColorSpace::OKLAB),
                0.193221f,
                0.3503176f,
                0.2459645f,
                0.5f);
    expectColor(Color::interpolate(Color(), Color(), 0.5, ColorSpace::OKLAB), 0.0f, 0.0f, 0.0f, 0.0f);
}

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
