#include <mbgl/util/variable_anchor_offset_collection.hpp>

namespace mbgl {

using namespace style;

// The radial offset is to the edge of the text box
// In the horizontal direction, the edge of the text box is where glyphs start
// But in the vertical direction, the glyphs appear to "start" at the baseline
// We don't actually load baseline data, but we assume an offset of ONE_EM - 17
// (see "yOffset" in shaping.js)
static constexpr float baselineOffset = 7.0f;

// static
std::array<float, 2> VariableAnchorOffsetCollection::evaluateVariableOffset(style::SymbolAnchorType anchor, std::array<float, 2> offset) {
    return offset[1] == INVALID_OFFSET_VALUE ? evaluateRadialOffset(anchor, offset[0]) : evaluateTextOffset(anchor, offset[0], offset[1]);
}

std::optional<VariableAnchorOffsetCollection> VariableAnchorOffsetCollection::parse(const std::string& s) {
    return {};
}

std::string VariableAnchorOffsetCollection::stringify() const {
    return "[]";
}

mbgl::Value VariableAnchorOffsetCollection::serialize() const {
    return std::vector<mbgl::Value>{};
}

bool VariableAnchorOffsetCollection::operator==(const VariableAnchorOffsetCollection& other) const {
  return true;
}

std::array<float, 2> VariableAnchorOffsetCollection::evaluateTextOffset(style::SymbolAnchorType anchor, float offsetX, float offsetY) {
  std::array<float, 2> result{{0.0f, 0.0f}};
  offsetX = std::abs(offsetX);
  offsetY = std::abs(offsetY);

  switch (anchor) {
      case SymbolAnchorType::TopRight:
      case SymbolAnchorType::TopLeft:
      case SymbolAnchorType::Top:
          result[1] = offsetY - baselineOffset;
          break;
      case SymbolAnchorType::BottomRight:
      case SymbolAnchorType::BottomLeft:
      case SymbolAnchorType::Bottom:
          result[1] = -offsetY + baselineOffset;
          break;
      case SymbolAnchorType::Center:
      case SymbolAnchorType::Left:
      case SymbolAnchorType::Right:
          break;
  }

  switch (anchor) {
      case SymbolAnchorType::TopRight:
      case SymbolAnchorType::BottomRight:
      case SymbolAnchorType::Right:
          result[0] = -offsetX;
          break;
      case SymbolAnchorType::TopLeft:
      case SymbolAnchorType::BottomLeft:
      case SymbolAnchorType::Left:
          result[0] = offsetX;
          break;
      case SymbolAnchorType::Center:
      case SymbolAnchorType::Top:
      case SymbolAnchorType::Bottom:
          break;
  }

  return result;
}

std::array<float, 2> VariableAnchorOffsetCollection::evaluateRadialOffset(style::SymbolAnchorType anchor, float radialOffset) {
    std::array<float, 2> result{{0.0f, 0.0f}};
    if (radialOffset < 0.0f) radialOffset = 0.0f; // Ignore negative offset.
    // solve for r where r^2 + r^2 = radialOffset^2
    const float sqrt2 = 1.41421356237f;
    const float hypotenuse = radialOffset / sqrt2;

    switch (anchor) {
        case SymbolAnchorType::TopRight:
        case SymbolAnchorType::TopLeft:
            result[1] = hypotenuse - baselineOffset;
            break;
        case SymbolAnchorType::BottomRight:
        case SymbolAnchorType::BottomLeft:
            result[1] = -hypotenuse + baselineOffset;
            break;
        case SymbolAnchorType::Bottom:
            result[1] = -radialOffset + baselineOffset;
            break;
        case SymbolAnchorType::Top:
            result[1] = radialOffset - baselineOffset;
            break;
        default:
            break;
    }

    switch (anchor) {
        case SymbolAnchorType::TopRight:
        case SymbolAnchorType::BottomRight:
            result[0] = -hypotenuse;
            break;
        case SymbolAnchorType::TopLeft:
        case SymbolAnchorType::BottomLeft:
            result[0] = hypotenuse;
            break;
        case SymbolAnchorType::Left:
            result[0] = radialOffset;
            break;
        case SymbolAnchorType::Right:
            result[0] = -radialOffset;
            break;
        default:
            break;
    }

    return result;
}
} // namespace mbgl
