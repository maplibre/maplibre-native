#include <mln/test/util.hpp>

#include <array>
#include <cmath>
#include <numbers>

using namespace mln;

// The STANDARD hillshade method scales the slope with
//     (pow(base, slope) - 1) / (pow(base, maxValue) - 1) * maxValue,   base = 1.875 - intensity * 1.75.
// Close to intensity 0.5, base is close to 1 and both terms cancel to almost nothing. The old
// shader lost all shading for exaggeration 0.4995 to 0.49999 on some GPUs (maplibre-native issue 5095). These tests
// check the math of the formula the shaders now use, transcribed to the CPU; the render cases
// `hillshade-exaggeration/*` are the regression test.

namespace {

constexpr float kMaxValue = 0.5f * std::numbers::pi_v<float>;

// The formula in the shaders, all in highp, which is fp32 on every backend.
float log1pStable(float x) {
    if (std::fabs(x) < 0.1f) {
        return x * (1.0f - x * (0.5f - x * (1.0f / 3.0f - x * (0.25f - x * (0.2f - x / 6.0f)))));
    }
    return std::log(1.0f + x);
}

float expm1OverX(float x) {
    if (std::fabs(x) < 0.1f) {
        return 1.0f + x * (0.5f + x * (1.0f / 6.0f + x * (1.0f / 24.0f + x / 120.0f)));
    }
    return (std::exp(x) - 1.0f) / x;
}

float newShaderFormula(float intensity, float slope) {
    const float logBase = log1pStable(1.75f * (0.5f - intensity));
    return slope * expm1OverX(slope * logBase) / expm1OverX(kMaxValue * logBase);
}

// The exact value, computed in double with log1p and expm1.
double referenceFormula(float intensity, float slope) {
    const double logBase = std::log1p(1.75 * (0.5 - static_cast<double>(intensity)));
    const double maxValue = 0.5 * std::numbers::pi;
    if (logBase == 0.0) return slope;
    return std::expm1(slope * logBase) / std::expm1(maxValue * logBase) * maxValue;
}

constexpr std::array<float, 21> kIntensities{0.0f,     0.1f,     0.4f,       0.49f, 0.4994f,    0.4995f,   0.4999f,
                                             0.49997f, 0.49999f, 0.4999999f, 0.5f,  0.5000001f, 0.500001f, 0.50003f,
                                             0.50006f, 0.5001f,  0.501f,     0.51f, 0.6f,       0.9f,      1.0f};

// atan(0.625 * length(deriv)), from flat ground to a vertical-ish cliff.
constexpr std::array<float, 8> kSlopes{0.0f, 0.001f, 0.01f, 0.1f, 0.3f, 0.7f, 1.2f, 1.5f};

} // namespace

TEST(HillshadeStandardFormula, NewFormulaMatchesTheExactValue) {
    for (const float intensity : kIntensities) {
        for (const float slope : kSlopes) {
            const double expected = referenceFormula(intensity, slope);
            const float actual = newShaderFormula(intensity, slope);
            ASSERT_TRUE(std::isfinite(actual)) << "intensity " << intensity << " slope " << slope;
            EXPECT_NEAR(actual, expected, 2e-6) << "intensity " << intensity << " slope " << slope;
        }
    }
}

TEST(HillshadeStandardFormula, NewFormulaIsContinuousAcrossHalf) {
    // The curve is smooth through 0.5, where it is the identity, so a tiny step in exaggeration is a tiny step in
    // shade.
    for (const float slope : kSlopes) {
        const float atHalf = newShaderFormula(0.5f, slope);
        EXPECT_EQ(atHalf, slope);
        for (const float intensity : {0.4995f, 0.49997f, 0.49999f, 0.4999999f, 0.5000001f, 0.50003f, 0.5005f}) {
            EXPECT_NEAR(newShaderFormula(intensity, slope), atHalf, 0.002f * (1.0f + slope))
                << "intensity " << intensity << " slope " << slope;
        }
    }
}
