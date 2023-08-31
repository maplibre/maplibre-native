#pragma once

#include <cstdint>

namespace mbgl {
namespace gfx {

enum class PrimitiveType : uint8_t {
    Point,
    Line,
    Triangle,
};

enum class DrawModeType : uint8_t {
    Points,
    Lines,
    LineLoop,
    LineStrip,
    Triangles,
    TriangleStrip,
    TriangleFan,
};

enum class AttributeDataType : uint8_t {
    Byte,
    Byte2,
    Byte3,
    Byte4,

    UByte,
    UByte2,
    UByte3,
    UByte4,

    Short,
    Short2,
    Short3,
    Short4,

    UShort,
    UShort2,
    UShort3,
    UShort4,

    Int,
    Int2,
    Int3,
    Int4,

    UInt,
    UInt2,
    UInt3,
    UInt4,

    Float,
    Float2,
    Float3,
    Float4,

    Invalid = 255,
};

} // namespace gfx
} // namespace mbgl
