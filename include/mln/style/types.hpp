#pragma once

#include <mln/util/enum.hpp>

#include <cstdint>

namespace mln {

namespace style {

// TODO: should be in public source.hpp header
enum class SourceType : uint8_t {
    Vector,
    Raster,
    RasterDEM,
    GeoJSON,
    Video,
    Annotations,
    Image,
    CustomVector,
    CustomMVTVector
};

enum class VisibilityType : bool {
    Visible,
    None,
};

enum class LineCapType : uint8_t {
    Round,
    Butt,
    Square,
};

enum class LineJoinType : uint8_t {
    Miter,
    Bevel,
    Round,
    // the following two types are for internal use only
    FakeRound,
    FlipBevel
};

enum class RasterResamplingType : bool {
    Linear,
    Nearest
};

enum class HillshadeIlluminationAnchorType : bool {
    Map,
    Viewport
};

enum class HillshadeMethodType : uint8_t {
    Standard,
    Combined,
    Igor,
    Multidirectional,
    Basic
};

enum class TranslateAnchorType : bool {
    Map,
    Viewport
};

enum class RotateAnchorType : bool {
    Map,
    Viewport,
};

enum class CirclePitchScaleType : bool {
    Map,
    Viewport,
};

enum class SymbolPlacementType : uint8_t {
    Point,
    Line,
    LineCenter
};

enum class SymbolZOrderType : uint8_t {
    Auto,
    ViewportY,
    Source
};

enum class AlignmentType : uint8_t {
    Map,
    Viewport,
    Auto,
};

enum class TextJustifyType : uint8_t {
    Auto,
    Center,
    Left,
    Right
};

enum class SymbolAnchorType : uint8_t {
    Center,
    Left,
    Right,
    Top,
    Bottom,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

using TextVariableAnchorType = SymbolAnchorType;

enum class TextTransformType : uint8_t {
    None,
    Uppercase,
    Lowercase,
};

enum class IconTextFitType : uint8_t {
    None,
    Both,
    Width,
    Height
};

enum class TextWritingModeType : uint8_t {
    Horizontal,
    Vertical
};

enum class LightAnchorType : bool {
    Map,
    Viewport
};

} // namespace style

MLN_DECLARE_ENUM(style::SourceType);
MLN_DECLARE_ENUM(style::VisibilityType);
MLN_DECLARE_ENUM(style::TranslateAnchorType);
MLN_DECLARE_ENUM(style::RasterResamplingType);
MLN_DECLARE_ENUM(style::HillshadeIlluminationAnchorType);
MLN_DECLARE_ENUM(style::HillshadeMethodType);
MLN_DECLARE_ENUM(style::RotateAnchorType);
MLN_DECLARE_ENUM(style::CirclePitchScaleType);
MLN_DECLARE_ENUM(style::LineCapType);
MLN_DECLARE_ENUM(style::LineJoinType);
MLN_DECLARE_ENUM(style::SymbolPlacementType);
MLN_DECLARE_ENUM(style::SymbolAnchorType);
MLN_DECLARE_ENUM(style::SymbolZOrderType);
MLN_DECLARE_ENUM(style::TextJustifyType);
MLN_DECLARE_ENUM(style::TextTransformType);
MLN_DECLARE_ENUM(style::TextWritingModeType);
MLN_DECLARE_ENUM(style::AlignmentType);
MLN_DECLARE_ENUM(style::IconTextFitType);
MLN_DECLARE_ENUM(style::LightAnchorType);

} // namespace mln
