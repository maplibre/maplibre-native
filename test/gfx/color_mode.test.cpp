#include <mln/test/util.hpp>

#include <mln/gfx/color_mode.hpp>

using namespace mln;
using namespace mln::gfx;

namespace {
ColorMode withBlend(ColorMode::BlendFunction blendFunction) {
    return {.blendFunction = blendFunction, .blendColor = {}, .mask = {.r = true, .g = true, .b = true, .a = true}};
}
} // namespace

TEST(ColorMode, HashTellsBlendFactorsApart) {
    // Render pipelines are cached by the hash
    EXPECT_EQ(ColorMode::alphaBlended().hash(), ColorMode::alphaBlended().hash());
    EXPECT_NE(ColorMode::alphaBlended().hash(), ColorMode::additive().hash());
    EXPECT_NE(withBlend(ColorMode::Add{ColorBlendFactorType::One, ColorBlendFactorType::DstAlpha}).hash(),
              withBlend(ColorMode::Add{ColorBlendFactorType::One, ColorBlendFactorType::OneMinusSrcAlpha}).hash());
    EXPECT_NE(withBlend(ColorMode::Min{}).hash(), withBlend(ColorMode::Max{}).hash());
    EXPECT_NE(ColorMode::alphaBlended().hash(), ColorMode::disabled().hash());
}
