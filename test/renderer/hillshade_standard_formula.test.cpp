#include <mln/test/util.hpp>

#include <mln/shaders/gl/hillshade.hpp>
#if MLN_RENDER_BACKEND_VULKAN
#include <mln/shaders/vulkan/hillshade.hpp>
#endif
#if MLN_RENDER_BACKEND_WEBGPU
#include <mln/shaders/webgpu/hillshade.hpp>
#endif
#if MLN_RENDER_BACKEND_METAL
#include <mln/shaders/mtl/hillshade.hpp>
#endif

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numbers>
#include <string_view>

using namespace mln;

// The STANDARD hillshade method scales the slope with
//     (pow(base, slope) - 1) / (pow(base, maxValue) - 1) * maxValue,   base = 1.875 - intensity * 1.75.
// Close to intensity 0.5, base is close to 1 and both terms cancel to almost nothing. Adreno GPUs evaluate mediump
// floats as fp16, which has a spacing of about 0.001 near 1, so the old shader lost all shading for exaggeration
// 0.4995 to 0.49999 (maplibre-native issue 5095). These tests transcribe the shader formulas to the CPU.

namespace {

constexpr float kMaxValue = 0.5f * std::numbers::pi_v<float>;

// Round to the nearest IEEE half precision value, as a mediump operation does on Adreno.
float half(float x) {
    if (x == 0.0f || !std::isfinite(x)) return x;
    int exponent = 0;
    const double magnitude = std::fabs(static_cast<double>(x));
    std::frexp(magnitude, &exponent);
    // 11 significant bits for normal numbers, a fixed spacing of 2^-24 below the smallest normal number.
    const double quantum = std::ldexp(1.0, std::max(exponent - 11, -24));
    const double rounded = std::nearbyint(magnitude / quantum) * quantum;
    if (rounded > 65504.0) return std::copysign(std::numeric_limits<float>::infinity(), x);
    return std::copysign(static_cast<float>(rounded), x);
}

// The formula before the fix, with every operation rounded to fp16 except the highp uniform used by the guard.
float oldShaderFormulaAtHalfPrecision(float intensity, float slope) {
    if (!(std::fabs(intensity - 0.5f) > 1e-6f)) return slope;
    const float base = half(1.875f - half(intensity * 1.75f));
    const float numerator = half(half(std::pow(base, slope)) - 1.0f);
    const float denominator = half(half(std::pow(base, kMaxValue)) - 1.0f);
    return half(numerator / denominator) * kMaxValue;
}

// The formula in the shader now, all in highp, which is fp32 on every backend.
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

TEST(HillshadeStandardFormula, HalfRoundingMatchesFp16) {
    EXPECT_EQ(half(1.0f), 1.0f);
    EXPECT_EQ(half(1.0f + 0.0004f), 1.0f);                 // below half the 0.000977 spacing above 1
    EXPECT_EQ(half(1.0f + 0.0006f), 1.0f + 0.0009765625f); // above it
    EXPECT_EQ(half(1.0f - 0.0001f), 1.0f);                 // spacing below 1 is 0.000488
    EXPECT_EQ(half(0.3f), 0.300048828125f);
    EXPECT_EQ(half(70000.0f), std::numeric_limits<float>::infinity());
}

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

TEST(HillshadeStandardFormula, OldFormulaBreaksAtHalfPrecision) {
    // Control for the tests above: the same transcription of the old shader, evaluated as mediump, loses the shading.
    for (const float intensity : {0.49999f, 0.49997f, 0.4999f, 0.4995f}) {
        float worst = 0.0f;
        bool notFinite = false;
        for (const float slope : kSlopes) {
            const float actual = oldShaderFormulaAtHalfPrecision(intensity, slope);
            notFinite = notFinite || !std::isfinite(actual);
            if (std::isfinite(actual)) {
                worst = std::max(worst, std::fabs(actual - static_cast<float>(referenceFormula(intensity, slope))));
            }
        }
        EXPECT_TRUE(notFinite || worst > 0.05f) << "intensity " << intensity << " worst error " << worst;
    }
}

TEST(HillshadeStandardFormula, OldFormulaWorksAwayFromHalfAtHalfPrecision) {
    // The old shader was right for exaggeration well away from 0.5, as the issue reports, so the emulation is fair.
    for (const float intensity : {0.0f, 0.1f, 0.4f, 0.6f, 0.9f, 1.0f}) {
        for (const float slope : kSlopes) {
            EXPECT_NEAR(oldShaderFormulaAtHalfPrecision(intensity, slope), referenceFormula(intensity, slope), 0.05)
                << "intensity " << intensity << " slope " << slope;
        }
    }
}

namespace {

void expectNewFormulaInShader(std::string_view source, const char* backend) {
    EXPECT_NE(source.find("expm1_over_x"), std::string_view::npos) << backend;
    EXPECT_NE(source.find("log1p_stable"), std::string_view::npos) << backend;
    EXPECT_EQ(source.find("abs(intensity - 0.5)"), std::string_view::npos) << backend;
    EXPECT_EQ(source.find("1e-6"), std::string_view::npos) << backend;
}

} // namespace

TEST(HillshadeStandardFormula, ShadersUseTheNewFormula) {
    expectNewFormulaInShader(
        shaders::ShaderSource<shaders::BuiltIn::HillshadeShader, gfx::Backend::Type::OpenGL>::fragment, "OpenGL");
#if MLN_RENDER_BACKEND_VULKAN
    expectNewFormulaInShader(
        shaders::ShaderSource<shaders::BuiltIn::HillshadeShader, gfx::Backend::Type::Vulkan>::fragment, "Vulkan");
#endif
#if MLN_RENDER_BACKEND_WEBGPU
    expectNewFormulaInShader(
        shaders::ShaderSource<shaders::BuiltIn::HillshadeShader, gfx::Backend::Type::WebGPU>::fragment, "WebGPU");
#endif
#if MLN_RENDER_BACKEND_METAL
    expectNewFormulaInShader(
        shaders::ShaderSource<shaders::BuiltIn::HillshadeShader, gfx::Backend::Type::Metal>::source, "Metal");
#endif
}
