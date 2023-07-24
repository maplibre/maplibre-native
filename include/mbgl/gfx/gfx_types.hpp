#pragma once

#include <cstdint>

namespace mbgl {
namespace gfx {

/// Graphic primitive type enumeration
enum class PrimitiveType : uint8_t {
    Point,    /// point primitive
    Line,     /// line primive
    Triangle, /// triangle primitive
};

/// Drawing mode type
enum class DrawModeType : uint8_t {
    Points,        /// points
    Lines,         /// line segments
    LineLoop,      /// line loop, a.k.a. closed polyline
    LineStrip,     /// line strip, a.k.a. open polyline
    Triangles,     /// independent triangles
    TriangleStrip, /// triangles sharing vertices
    TriangleFan,   /// fan of triangles, with a common central point
};

/// Attribute data types
enum class AttributeDataType : uint8_t {
    Byte,  /// one signed byte (8 bit)
    Byte2, /// pack of 2 signed bytes
    Byte3, /// pack of 3 signed bytes
    Byte4, /// pack 4 signed bytes

    UByte,  /// one unsigned byte (8 bit)
    UByte2, /// pack of 2 unsigned bytes
    UByte3, /// pack of 3 unsigned bytes
    UByte4, /// pack of 4 unsigned bytes

    Short,  /// signed 16 bit integer value
    Short2, /// pack of 2 signed 16 bit integers
    Short3, /// pack of 3 signed 16 bit integers
    Short4, /// pack of 4 signed 16 bit integers

    UShort,  /// unsigned 16 bit integer value
    UShort2, /// pack of 2 unsigned 16 bit integers
    UShort3, /// pack of 3 unsigned 16 bit integers
    UShort4, /// pack of 4 unsigned 16 bit integers
    UShort8, /// pack of 6 unsigned 16 bit integers

    Int,  /// signed 32 bit integer value
    Int2, /// pack of 2 signed 32 bit integers
    Int3, /// pack of 3 signed 32 bit integers
    Int4, /// pack of 4 signed 32 bit integers

    UInt,  /// unsigned 32 bit integer value
    UInt2, /// pack of 2 unsigned 32 bit integers
    UInt3, /// pack of 3 unsigned 32 bit integers
    UInt4, /// pack of 4 unsigned 32 bit integers

    Float,  /// 32 bit floating point value
    Float2, /// pack of 2 floating point values
    Float3, /// pack of 3 floating point values
    Float4, /// pack of 4 floating point values

    Invalid = 255,
};

} // namespace gfx
} // namespace mbgl
