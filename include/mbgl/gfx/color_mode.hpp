#pragma once

#include <mbgl/gfx/types.hpp>
#include <mbgl/util/variant.hpp>
#include <mbgl/util/color.hpp>
#include <mbgl/util/hash.hpp>

namespace mbgl {
namespace gfx {

class ColorMode {
public:
    template <ColorBlendEquationType E>
    struct ConstantBlend {
        static constexpr ColorBlendEquationType equation = E;
        static constexpr ColorBlendFactorType srcFactor = ColorBlendFactorType::One;
        static constexpr ColorBlendFactorType dstFactor = ColorBlendFactorType::One;
    };

    template <ColorBlendEquationType E>
    struct LinearBlend {
        static constexpr ColorBlendEquationType equation = E;
        ColorBlendFactorType srcFactor;
        ColorBlendFactorType dstFactor;
    };

    struct Replace {
        static constexpr ColorBlendEquationType equation = ColorBlendEquationType::Add;
        static constexpr ColorBlendFactorType srcFactor = ColorBlendFactorType::One;
        static constexpr ColorBlendFactorType dstFactor = ColorBlendFactorType::Zero;
    };

    using Add = LinearBlend<ColorBlendEquationType::Add>;
    using Subtract = LinearBlend<ColorBlendEquationType::Subtract>;
    using ReverseSubtract = LinearBlend<ColorBlendEquationType::ReverseSubtract>;

    using BlendFunction = variant<Replace, Add, Subtract, ReverseSubtract>;

    BlendFunction blendFunction;
    Color blendColor;

    struct Mask {
        bool r;
        bool g;
        bool b;
        bool a;
    };

    Mask mask;

    static ColorMode disabled() {
        return {.blendFunction = Replace{}, .blendColor = {}, .mask = {.r = false, .g = false, .b = false, .a = false}};
    }

    static ColorMode unblended() {
        return {.blendFunction = Replace{}, .blendColor = {}, .mask = {.r = true, .g = true, .b = true, .a = true}};
    }

    static ColorMode alphaBlended() {
        return {.blendFunction = Add{ColorBlendFactorType::One, ColorBlendFactorType::OneMinusSrcAlpha},
                .blendColor = {},
                .mask = {.r = true, .g = true, .b = true, .a = true}};
    }

    static ColorMode additive() {
        return {.blendFunction = Add{ColorBlendFactorType::One, ColorBlendFactorType::One},
                .blendColor = {},
                .mask = {.r = true, .g = true, .b = true, .a = true}};
    }

    std::size_t hash() const {
        return mbgl::util::hash(blendFunction.which(),
                                blendColor.r,
                                blendColor.g,
                                blendColor.b,
                                blendColor.a,
                                mask.r,
                                mask.g,
                                mask.b,
                                mask.a);
    }
};

constexpr bool operator!=(const ColorMode::Mask& a, const ColorMode::Mask& b) {
    return a.r != b.r || a.g != b.g || a.b != b.b || a.a != b.a;
}

} // namespace gfx
} // namespace mbgl
