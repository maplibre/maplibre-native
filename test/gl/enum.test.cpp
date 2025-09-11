#if MLN_RENDER_BACKEND_OPENGL

#include <mbgl/gfx/gfx_types.hpp>
#include <mbgl/gfx/types.hpp>
#include <mbgl/gl/defines.hpp>
#include <mbgl/gl/enum.hpp>
#include <mbgl/test/util.hpp>

using namespace mbgl;
using namespace mbgl::gfx;
using namespace mbgl::gl;

TEST(GL, DrawModeType) {
    ASSERT_EQ(GL_POINTS, Enum<DrawModeType>::to(DrawModeType::Points));
    ASSERT_EQ(GL_LINES, Enum<DrawModeType>::to(DrawModeType::Lines));
    ASSERT_EQ(GL_LINE_LOOP, Enum<DrawModeType>::to(DrawModeType::LineLoop));
    ASSERT_EQ(GL_LINE_STRIP, Enum<DrawModeType>::to(DrawModeType::LineStrip));
    ASSERT_EQ(GL_TRIANGLES, Enum<DrawModeType>::to(DrawModeType::Triangles));
    ASSERT_EQ(GL_TRIANGLE_STRIP, Enum<DrawModeType>::to(DrawModeType::TriangleStrip));
    ASSERT_EQ(GL_TRIANGLE_FAN, Enum<DrawModeType>::to(DrawModeType::TriangleFan));
    ASSERT_EQ(
        GL_INVALID_ENUM,
        Enum<DrawModeType>::to(static_cast<DrawModeType>(-1))); // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
}

namespace {
template <typename T>
auto enumIdentity(T x) {
    return Enum<T>::from(Enum<T>::to(x));
}
} // namespace

TEST(GL, ColorBlendEquationType) {
    // const auto identity = [](auto x){ return Enum<ColorBlendEquationType>::from(Enum<ColorBlendEquationType>::to(x));
    // };
    ASSERT_EQ(ColorBlendEquationType::Add, enumIdentity(ColorBlendEquationType::Add));
    ASSERT_EQ(ColorBlendEquationType::Subtract, enumIdentity(ColorBlendEquationType::Subtract));
    ASSERT_EQ(ColorBlendEquationType::ReverseSubtract, enumIdentity(ColorBlendEquationType::ReverseSubtract));

    ASSERT_EQ(GL_INVALID_ENUM,
              Enum<ColorBlendEquationType>::to(
                  static_cast<ColorBlendEquationType>(-1))); // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
    ASSERT_EQ(ColorBlendEquationType{}, Enum<ColorBlendEquationType>::from(GL_RGBA8));
}

TEST(GL, ColorBlendFactorType) {
    ASSERT_EQ(ColorBlendFactorType::Zero, enumIdentity(ColorBlendFactorType::Zero));
    ASSERT_EQ(ColorBlendFactorType::One, enumIdentity(ColorBlendFactorType::One));
    ASSERT_EQ(ColorBlendFactorType::SrcColor, enumIdentity(ColorBlendFactorType::SrcColor));
    ASSERT_EQ(ColorBlendFactorType::OneMinusSrcColor, enumIdentity(ColorBlendFactorType::OneMinusSrcColor));
    ASSERT_EQ(ColorBlendFactorType::DstColor, enumIdentity(ColorBlendFactorType::DstColor));
    ASSERT_EQ(ColorBlendFactorType::OneMinusDstColor, enumIdentity(ColorBlendFactorType::OneMinusDstColor));
    ASSERT_EQ(ColorBlendFactorType::SrcAlpha, enumIdentity(ColorBlendFactorType::SrcAlpha));
    ASSERT_EQ(ColorBlendFactorType::OneMinusSrcAlpha, enumIdentity(ColorBlendFactorType::OneMinusSrcAlpha));
    ASSERT_EQ(ColorBlendFactorType::DstAlpha, enumIdentity(ColorBlendFactorType::DstAlpha));
    ASSERT_EQ(ColorBlendFactorType::OneMinusDstAlpha, enumIdentity(ColorBlendFactorType::OneMinusDstAlpha));
    ASSERT_EQ(ColorBlendFactorType::ConstantColor, enumIdentity(ColorBlendFactorType::ConstantColor));
    ASSERT_EQ(ColorBlendFactorType::OneMinusConstantColor, enumIdentity(ColorBlendFactorType::OneMinusConstantColor));
    ASSERT_EQ(ColorBlendFactorType::ConstantAlpha, enumIdentity(ColorBlendFactorType::ConstantAlpha));
    ASSERT_EQ(ColorBlendFactorType::OneMinusConstantAlpha, enumIdentity(ColorBlendFactorType::OneMinusConstantAlpha));
    ASSERT_EQ(ColorBlendFactorType::SrcAlphaSaturate, enumIdentity(ColorBlendFactorType::SrcAlphaSaturate));

    ASSERT_EQ(GL_INVALID_ENUM,
              Enum<ColorBlendFactorType>::to(
                  static_cast<ColorBlendFactorType>(-1))); // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
    ASSERT_EQ(ColorBlendFactorType{}, Enum<ColorBlendFactorType>::from(GL_RGBA8));
}

TEST(GL, DepthFunctionType) {
    ASSERT_EQ(DepthFunctionType::Never, enumIdentity(DepthFunctionType::Never));
    ASSERT_EQ(DepthFunctionType::Less, enumIdentity(DepthFunctionType::Less));
    ASSERT_EQ(DepthFunctionType::Equal, enumIdentity(DepthFunctionType::Equal));
    ASSERT_EQ(DepthFunctionType::LessEqual, enumIdentity(DepthFunctionType::LessEqual));
    ASSERT_EQ(DepthFunctionType::Greater, enumIdentity(DepthFunctionType::Greater));
    ASSERT_EQ(DepthFunctionType::NotEqual, enumIdentity(DepthFunctionType::NotEqual));
    ASSERT_EQ(DepthFunctionType::GreaterEqual, enumIdentity(DepthFunctionType::GreaterEqual));
    ASSERT_EQ(DepthFunctionType::Always, enumIdentity(DepthFunctionType::Always));

    ASSERT_EQ(GL_INVALID_ENUM,
              Enum<DepthFunctionType>::to(
                  static_cast<DepthFunctionType>(-1))); // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
    ASSERT_EQ(DepthFunctionType{}, Enum<DepthFunctionType>::from(GL_RGBA8));
}

TEST(GL, DepthMaskType) {
    ASSERT_EQ(DepthMaskType::ReadOnly, enumIdentity(DepthMaskType::ReadOnly));
    ASSERT_EQ(DepthMaskType::ReadWrite, enumIdentity(DepthMaskType::ReadWrite));

    // anything non-zero is true, there's no way to represent invalid
    ASSERT_EQ(GL_TRUE, Enum<DepthMaskType>::to(static_cast<DepthMaskType>(-1)));
}

TEST(GL, StencilFunctionType) {
    ASSERT_EQ(StencilFunctionType::Never, enumIdentity(StencilFunctionType::Never));
    ASSERT_EQ(StencilFunctionType::Less, enumIdentity(StencilFunctionType::Less));
    ASSERT_EQ(StencilFunctionType::Equal, enumIdentity(StencilFunctionType::Equal));
    ASSERT_EQ(StencilFunctionType::LessEqual, enumIdentity(StencilFunctionType::LessEqual));
    ASSERT_EQ(StencilFunctionType::Greater, enumIdentity(StencilFunctionType::Greater));
    ASSERT_EQ(StencilFunctionType::NotEqual, enumIdentity(StencilFunctionType::NotEqual));
    ASSERT_EQ(StencilFunctionType::GreaterEqual, enumIdentity(StencilFunctionType::GreaterEqual));
    ASSERT_EQ(StencilFunctionType::Always, enumIdentity(StencilFunctionType::Always));

    ASSERT_EQ(GL_INVALID_ENUM,
              Enum<StencilFunctionType>::to(
                  static_cast<StencilFunctionType>(-1))); // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
    ASSERT_EQ(StencilFunctionType{}, Enum<StencilFunctionType>::from(GL_RGBA8));
}

TEST(GL, StencilOpType) {
    ASSERT_EQ(StencilOpType::Keep, enumIdentity(StencilOpType::Keep));
    ASSERT_EQ(StencilOpType::Zero, enumIdentity(StencilOpType::Zero));
    ASSERT_EQ(StencilOpType::Replace, enumIdentity(StencilOpType::Replace));
    ASSERT_EQ(StencilOpType::Increment, enumIdentity(StencilOpType::Increment));
    ASSERT_EQ(StencilOpType::IncrementWrap, enumIdentity(StencilOpType::IncrementWrap));
    ASSERT_EQ(StencilOpType::Decrement, enumIdentity(StencilOpType::Decrement));
    ASSERT_EQ(StencilOpType::DecrementWrap, enumIdentity(StencilOpType::DecrementWrap));
    ASSERT_EQ(StencilOpType::Invert, enumIdentity(StencilOpType::Invert));

    ASSERT_EQ(GL_INVALID_ENUM,
              Enum<StencilOpType>::to(
                  static_cast<StencilOpType>(-1))); // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
    ASSERT_EQ(StencilOpType{}, Enum<StencilOpType>::from(GL_RGBA8));
}

TEST(GL, CullFaceSideType) {
    ASSERT_EQ(CullFaceSideType::Front, enumIdentity(CullFaceSideType::Front));
    ASSERT_EQ(CullFaceSideType::Back, enumIdentity(CullFaceSideType::Back));
    ASSERT_EQ(CullFaceSideType::FrontAndBack, enumIdentity(CullFaceSideType::FrontAndBack));

    ASSERT_EQ(GL_INVALID_ENUM,
              Enum<CullFaceSideType>::to(
                  static_cast<CullFaceSideType>(-1))); // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
    ASSERT_EQ(CullFaceSideType{}, Enum<CullFaceSideType>::from(GL_RGBA8));
}

TEST(GL, CullFaceWindingType) {
    ASSERT_EQ(CullFaceWindingType::Clockwise, enumIdentity(CullFaceWindingType::Clockwise));
    ASSERT_EQ(CullFaceWindingType::CounterClockwise, enumIdentity(CullFaceWindingType::CounterClockwise));

    ASSERT_EQ(GL_INVALID_ENUM,
              Enum<CullFaceWindingType>::to(
                  static_cast<CullFaceWindingType>(-1))); // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
    ASSERT_EQ(CullFaceWindingType{}, Enum<CullFaceWindingType>::from(GL_RGBA8));
}

TEST(GL, BufferUsageType) {
    ASSERT_EQ(BufferUsageType::StreamDraw, enumIdentity(BufferUsageType::StreamDraw));
    ASSERT_EQ(BufferUsageType::StaticDraw, enumIdentity(BufferUsageType::StaticDraw));
    ASSERT_EQ(BufferUsageType::DynamicDraw, enumIdentity(BufferUsageType::DynamicDraw));

    ASSERT_EQ(GL_INVALID_ENUM,
              Enum<BufferUsageType>::to(
                  static_cast<BufferUsageType>(-1))); // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
    ASSERT_EQ(BufferUsageType{}, Enum<BufferUsageType>::from(GL_RGBA8));
}

TEST(GL, TexturePixelType) {
    ASSERT_EQ(TexturePixelType::RGBA, enumIdentity(TexturePixelType::RGBA));
    ASSERT_EQ(TexturePixelType::Alpha, enumIdentity(TexturePixelType::Alpha));
    ASSERT_EQ(TexturePixelType::Stencil, enumIdentity(TexturePixelType::Stencil));
    ASSERT_EQ(TexturePixelType::Depth, enumIdentity(TexturePixelType::Depth));
    ASSERT_EQ(TexturePixelType::Luminance, enumIdentity(TexturePixelType::Luminance));

    ASSERT_EQ(GL_INVALID_ENUM,
              Enum<TexturePixelType>::to(
                  static_cast<TexturePixelType>(-1))); // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
    ASSERT_EQ(TexturePixelType{}, Enum<TexturePixelType>::from(GL_RGBA8));
}

TEST(GL, TextureChannelDataType) {
    ASSERT_EQ(TextureChannelDataType::UnsignedByte, enumIdentity(TextureChannelDataType::UnsignedByte));
    ASSERT_EQ(TextureChannelDataType::HalfFloat, enumIdentity(TextureChannelDataType::HalfFloat));
    ASSERT_EQ(TextureChannelDataType::Float, enumIdentity(TextureChannelDataType::Float));

    ASSERT_EQ(GL_INVALID_ENUM,
              Enum<TextureChannelDataType>::to(
                  static_cast<TextureChannelDataType>(-1))); // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
    ASSERT_EQ(TextureChannelDataType{}, Enum<TextureChannelDataType>::from(GL_RGBA8));
}

TEST(GL, RenderbufferPixelType) {
    ASSERT_EQ(RenderbufferPixelType::RGBA, enumIdentity(RenderbufferPixelType::RGBA));
    ASSERT_EQ(RenderbufferPixelType::Depth, enumIdentity(RenderbufferPixelType::Depth));
    ASSERT_EQ(RenderbufferPixelType::DepthStencil, enumIdentity(RenderbufferPixelType::DepthStencil));

    ASSERT_EQ(GL_INVALID_ENUM,
              Enum<RenderbufferPixelType>::to(
                  static_cast<RenderbufferPixelType>(-1))); // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
    ASSERT_EQ(RenderbufferPixelType{}, Enum<RenderbufferPixelType>::from(GL_RGBA8));
}

TEST(GL, sizedFor) {
    ASSERT_EQ(GL_RGBA,
              Enum<gfx::TexturePixelType>::sizedFor(TexturePixelType::RGBA, TextureChannelDataType::UnsignedByte));
    ASSERT_EQ(GL_RGBA16F,
              Enum<gfx::TexturePixelType>::sizedFor(TexturePixelType::RGBA, TextureChannelDataType::HalfFloat));
    ASSERT_EQ(GL_INVALID_ENUM,
              Enum<gfx::TexturePixelType>::sizedFor(TexturePixelType::Alpha, TextureChannelDataType::HalfFloat));
    ASSERT_EQ(GL_RGBA32F, Enum<gfx::TexturePixelType>::sizedFor(TexturePixelType::RGBA, TextureChannelDataType::Float));
    ASSERT_EQ(GL_INVALID_ENUM,
              Enum<gfx::TexturePixelType>::sizedFor(TexturePixelType::Alpha, TextureChannelDataType::Float));
    ASSERT_EQ(GL_INVALID_ENUM,
              Enum<gfx::TexturePixelType>::sizedFor(
                  TexturePixelType::RGBA,
                  static_cast<TextureChannelDataType>(-1))); // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
}

#endif
