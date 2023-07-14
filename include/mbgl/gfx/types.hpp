#pragma once

#include <mbgl/gfx/gfx_types.hpp>

#include <cstdint>

namespace mbgl {
namespace gfx {

enum class ColorBlendEquationType : uint8_t {
    Add,
    Subtract,
    ReverseSubtract,
};

enum class ColorBlendFactorType : uint8_t {
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    DstColor,
    OneMinusDstColor,
    SrcAlphaSaturate,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha,
};

enum class DepthFunctionType : uint8_t {
    Never,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always,
};

enum class DepthMaskType : bool {
    ReadOnly = false,
    ReadWrite = true,
};

enum class StencilFunctionType : uint8_t {
    Never,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always,
};

enum class StencilOpType : uint8_t {
    Zero,
    Keep,
    Replace,
    Increment,
    Decrement,
    Invert,
    IncrementWrap,
    DecrementWrap,
};

enum CullFaceSideType : uint8_t {
    Front,
    Back,
    FrontAndBack,
};

enum CullFaceWindingType : uint8_t {
    Clockwise,
    CounterClockwise,
};

enum class BufferUsageType : uint8_t {
    StreamDraw,
    StaticDraw,
    DynamicDraw,
};

enum class TexturePixelType : uint8_t {
    RGBA,
    Alpha,
    Stencil,
    Depth,
    Luminance,
};

enum class TextureChannelDataType : uint8_t {
    UnsignedByte,
    HalfFloat,
};

enum class TextureMipMapType : bool {
    No,
    Yes
};

enum class TextureFilterType : bool {
    Nearest,
    Linear,
};

enum class TextureWrapType : bool {
    Clamp,
    Repeat,
};

enum class RenderbufferPixelType : uint8_t {
    RGBA,
    Depth,
    DepthStencil,
};

} // namespace gfx
} // namespace mbgl
