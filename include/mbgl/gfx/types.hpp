#pragma once

#include <mbgl/gfx/gfx_types.hpp>

#include <cstdint>

namespace mbgl {
namespace gfx {

/// Color blending equation type
enum class ColorBlendEquationType : uint8_t {
    Add,      ///< The source and destination colors are added to each other. O = sS + dD. The The s and d are blending
              ///< parameters that are multiplied into each of S and D before the addition.
    Subtract, ///< Subtracts the destination from the source. O = sS - dD. The source and dest are multiplied by
              ///< blending parameters.
    ReverseSubtract, ///< Subtracts the source from the destination. O = dD - sS. The source and dest are multiplied by
                     ///< blending parameters.
};

/// Color blending factor type
enum class ColorBlendFactorType : uint8_t {
    Zero,                  ///< Factor is (0, 0, 0, 0)
    One,                   ///< Factor is (1, 1, 1, 1)
    SrcColor,              ///< Factor is (Rs, Gs, Bs, As) where S is the source color
    OneMinusSrcColor,      ///< Factor is (1, 1, 1, 1) - (Rs, Gs, Bs, As) where S is the source color
    SrcAlpha,              ///< Factor is (As, As, As, As) where S is the source color
    OneMinusSrcAlpha,      ///< Factor is (1, 1, 1, 1) - (As, As, As, As) where S is the source color
    DstAlpha,              ///< Factor is (Ad, Ad, Ad, Ad) where D is the destination color
    OneMinusDstAlpha,      ///< Factor is (1, 1, 1, 1) - (Ad, Ad, Ad, Ad) where D is the destination color
    DstColor,              ///< Factor is (Rd, Gd, Bd, Ad) where D is the destination color
    OneMinusDstColor,      ///< Factor is (1, 1, 1, 1) - (Rd, Gd, Bd, Ad) where D is the destination color
    SrcAlphaSaturate,      ///< Factor is (f, f, f, 1) where f = min(As, 1 - Ad) and S is the source color
    ConstantColor,         ///< Factor is (Rc, Gc, Bc, Ac) where C is the constant color
    OneMinusConstantColor, ///< Factor is (1, 1, 1, 1) - (Rc, Gc, Bc, Ac)  where C is the constant color
    ConstantAlpha,         ///< Factor is (Ac, Ac, Ac, Ac) where C is the constant color
    OneMinusConstantAlpha, ///< Factor is (1, 1, 1, 1) - (Ac, Ac, Ac, Ac) where C is the constant color
};

/// Depth function type
enum class DepthFunctionType : uint8_t {
    Never,        ///< The depth test never passes.
    Less,         ///< The depth test passes if the incoming value is less than the stored value.
    Equal,        ///< The depth test passes if the incoming value is equal to the stored value.
    LessEqual,    ///< The depth test passes if the incoming value is less than or equal to the stored value.
    Greater,      ///< The depth test passes if the incoming value is greater than the stored value.
    NotEqual,     ///< The depth test passes if the incoming value is not equal to the stored value.
    GreaterEqual, ///< The depth test passes if the incoming value is greater than or equal to the stored value.
    Always,       ///< The depth test always passes.
};

/// Depth buffer masking type
enum class DepthMaskType : bool {
    ReadOnly = false, ///< The depth buffer is not written to
    ReadWrite = true, ///< The depth buffer is is updated by the new fragments' depth
};

/// Stencil function type
enum class StencilFunctionType : uint8_t {
    Never,        ///< Always fails.
    Less,         ///< Passes if ( ref & mask ) < ( stencil & mask ).
    Equal,        ///< Passes if ( ref & mask ) == ( stencil & mask ).
    LessEqual,    ///< Passes if ( ref & mask ) <= ( stencil & mask ).
    Greater,      ///< Passes if ( ref & mask ) > ( stencil & mask ).
    NotEqual,     ///< Passes if ( ref & mask ) != ( stencil & mask ).
    GreaterEqual, ///< Passes if ( ref & mask ) >= ( stencil & mask ).
    Always,       ///< Always passes.
};

/// Stencil operation type
enum class StencilOpType : uint8_t {
    Zero,          ///< Sets the stencil buffer value to 0.
    Keep,          ///< Keeps the current value.
    Replace,       ///< Replace the stencil buffer value
    Increment,     ///< Increments the current stencil buffer value. Clamps to the maximum representable unsigned value.
    Decrement,     ///< Decrements the current stencil buffer value. Clamps to 0.
    Invert,        ///< Bitwise inverts the current stencil buffer value.
    IncrementWrap, ///< Increments the current stencil buffer value. Wraps stencil buffer value to zero when
                   ///< incrementing the maximum representable unsigned value.
    DecrementWrap, ///< Decrements the current stencil buffer value. Wraps stencil buffer value to the maximum
                   ///< representable unsigned value when decrementing a stencil buffer value of zero.
};

/// Cull face side
enum CullFaceSideType : uint8_t {
    Front,        ///< Front facing facets are culled.
    Back,         ///< Back facing facets are culled.
    FrontAndBack, ///< Front and back facing facets are culled.
};

/// Define the orientation of front facing polygons
enum CullFaceWindingType : uint8_t {
    Clockwise,        ///< Front face is clockwise.
    CounterClockwise, ///< Front face is counter clockwise.
};

/// Buffer usage type
enum class BufferUsageType : uint8_t {
    StreamDraw,  ///< The data store contents will be modified once and used at most a few times.
    StaticDraw,  ///< The data store contents will be modified once and used many times.
    DynamicDraw, ///< The data store contents will be modified repeatedly and used many times.
};

/// Texture pixel type
enum class TexturePixelType : uint8_t {
    RGBA,      ///< Red, Green, Blue and Alpha
    Alpha,     ///< Alpha only
    Stencil,   ///< Stencil
    Depth,     ///< Depth component
    Luminance, ///< Luminance
};

/// Texture channel data type
enum class TextureChannelDataType : uint8_t {
    UnsignedByte, ///< 8 bit unsigned byte
    HalfFloat,    ///< 16 bit "half-float"
};

/// Texture mip map type
enum class TextureMipMapType : bool {
    No, ///< does not use mipmapping
    Yes ///< uses mipmapping
};

/// Texture filter type
enum class TextureFilterType : bool {
    Nearest, ///< Nearest pixel
    Linear,  ///< Bilinear interpolation
};

/// Texture wrap mode
enum class TextureWrapType : bool {
    Clamp,  ///< Clamp to edge
    Repeat, ///< Repeat
};

/// Render buffer pixel type
enum class RenderbufferPixelType : uint8_t {
    RGBA,         ///< Red, Green, Blue and Alpha
    Depth,        ///< Depth component
    DepthStencil, ///< Depth and stencil components
};

} // namespace gfx
} // namespace mbgl
