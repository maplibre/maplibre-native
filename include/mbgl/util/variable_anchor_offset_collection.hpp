#pragma once

#include <mbgl/style/types.hpp>
#include <mbgl/util/feature.hpp>

#include <string>
#include <unordered_map>
#include <vector>

namespace mbgl {

using AnchorOffsetMap = std::unordered_map<style::SymbolAnchorType, std::array<float, 2>>;

class VariableAnchorOffsetCollection {
public:
  VariableAnchorOffsetCollection() = default;
  VariableAnchorOffsetCollection(const AnchorOffsetMap& values): anchorOffsets(std::move(values)) {};

  static constexpr float INVALID_OFFSET_VALUE = std::numeric_limits<float>::max();
  /**
   * @brief Calculates variable text offset.
   *
   * @param anchor text anchor
   * @param textOffset Either `text-offset` or [ `text-radial-offset`,
   * INVALID_OFFSET_VALUE ]
   * @return std::array<float, 2> offset along x- and y- axis correspondingly.
   */
  static std::array<float, 2> evaluateVariableOffset(style::SymbolAnchorType anchor, std::array<float, 2> textOffset);
  
  static std::array<float, 2> evaluateRadialOffset(style::SymbolAnchorType anchor, float radialOffset);
  
  static std::optional<VariableAnchorOffsetCollection> parse(const std::string& s);

  std::string stringify() const;
  mbgl::Value serialize() const;
  std::size_t getSize() const { return anchorOffsets.size(); }

  bool operator==(const VariableAnchorOffsetCollection& other) const;

private:
  static std::array<float, 2> evaluateTextOffset(style::SymbolAnchorType anchor, float offsetX, float offsetY);
  
  AnchorOffsetMap anchorOffsets;
};

} // namespace mbgl
