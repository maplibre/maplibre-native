#pragma once

#include <mbgl/text/quads.hpp>
#include <mbgl/text/glyph_atlas.hpp>
#include <mbgl/text/collision_feature.hpp>
#include <mbgl/style/layers/symbol_layer_properties.hpp>
#include <mbgl/util/bitmask_operations.hpp>

#if !defined(MLN_SYMBOL_GUARDS)
#define MLN_SYMBOL_GUARDS 1
#endif

#if MLN_SYMBOL_GUARDS
#define __SYM_GUARD_LOC__ __SOURCE_LOCATION__
#define SYM_GUARD_VALUE(N) std::uint64_t check##N = checkVal;
#else
#define __SYM_GUARD_LOC__ \
    std::string_view {}
#define SYM_GUARD_VALUE(N)
#endif

namespace mbgl {

class Anchor;
class IndexedSubfeature;

struct ShapedTextOrientations {
    Shaping horizontal;
    Shaping vertical;
    // The following are used with variable text placement on.
    Shaping& right = horizontal;
    Shaping center;
    Shaping left;
    bool singleLine = false;
};

enum class SymbolContent : uint8_t {
    None = 0,
    Text = 1 << 0,
    IconRGBA = 1 << 1,
    IconSDF = 1 << 2
};

struct SymbolInstanceSharedData {
    SymbolInstanceSharedData(GeometryCoordinates line,
                             const ShapedTextOrientations& shapedTextOrientations,
                             const std::optional<PositionedIcon>& shapedIcon,
                             const std::optional<PositionedIcon>& verticallyShapedIcon,
                             const style::SymbolLayoutProperties::Evaluated& layout,
                             style::SymbolPlacementType textPlacement,
                             const std::array<float, 2>& textOffset,
                             const ImageMap& imageMap,
                             float iconRotation,
                             SymbolContent iconType,
                             bool hasIconTextFit,
                             bool allowVerticalPlacement);
    bool empty() const;
    GeometryCoordinates line;
    // Note: When singleLine == true, only `rightJustifiedGlyphQuads` is populated.
    SymbolQuads rightJustifiedGlyphQuads;
    SymbolQuads centerJustifiedGlyphQuads;
    SymbolQuads leftJustifiedGlyphQuads;
    SymbolQuads verticalGlyphQuads;
    std::optional<SymbolQuads> iconQuads;
    std::optional<SymbolQuads> verticalIconQuads;
};

#if MLN_SYMBOL_GUARDS
// with guard values, clang-tidy complains about excessive packing
#pragma pack(push, 4)
#endif

class SymbolInstance {
public:
    SymbolInstance(Anchor& anchor_,
                   std::shared_ptr<SymbolInstanceSharedData> sharedData,
                   const ShapedTextOrientations& shapedTextOrientations,
                   const std::optional<PositionedIcon>& shapedIcon,
                   const std::optional<PositionedIcon>& verticallyShapedIcon,
                   float textBoxScale,
                   float textPadding,
                   style::SymbolPlacementType textPlacement,
                   const std::array<float, 2>& textOffset,
                   float iconBoxScale,
                   float iconPadding,
                   const std::array<float, 2>& iconOffset,
                   const RefIndexedSubfeature& indexedFeature,
                   std::size_t layoutFeatureIndex,
                   std::size_t dataFeatureIndex,
                   std::u16string key,
                   float overscaling,
                   float iconRotation,
                   float textRotation,
                   const std::array<float, 2>& variableTextOffset,
                   bool allowVerticalPlacement,
                   SymbolContent iconType = SymbolContent::None);

    std::optional<size_t> getDefaultHorizontalPlacedTextIndex() const;
    const GeometryCoordinates& line() const;
    const SymbolQuads& rightJustifiedGlyphQuads() const;
    const SymbolQuads& leftJustifiedGlyphQuads() const;
    const SymbolQuads& centerJustifiedGlyphQuads() const;
    const SymbolQuads& verticalGlyphQuads() const;
    bool hasText() const;
    bool hasIcon() const;
    bool hasSdfIcon() const;
    const std::optional<SymbolQuads>& iconQuads() const;
    const std::optional<SymbolQuads>& verticalIconQuads() const;
    void releaseSharedData();

#if MLN_SYMBOL_GUARDS
    /// Check all guard blocks
    bool check(std::string_view source = {}) const;
    /// Check that an index is in the valid range
    bool checkIndex(const std::optional<std::size_t>& index, std::size_t size, std::string_view source = {}) const;
    /// Check all indexes
    bool checkIndexes(std::size_t textCount,
                      std::size_t iconSize,
                      std::size_t sdfSize,
                      std::string_view source = {}) const;
    /// Mark this item as failed (due to some external check) so that it cannot be used later
    void forceFail() const;
#else
    bool check(std::string_view = {}) const { return true; }
    bool checkIndex(const std::optional<std::size_t>&, std::size_t, std::string_view = {}) const { return true; }
    bool checkIndexes(std::size_t, std::size_t, std::size_t, std::string_view = {}) const { return true; }
    void forceFail() const {}
#endif

    const Anchor& getAnchor() const { return anchor; }
    std::size_t getRightJustifiedGlyphQuadsSize() const { return rightJustifiedGlyphQuadsSize; }
    std::size_t getCenterJustifiedGlyphQuadsSize() const { return centerJustifiedGlyphQuadsSize; }
    std::size_t getLeftJustifiedGlyphQuadsSize() const { return leftJustifiedGlyphQuadsSize; }
    std::size_t getVerticalGlyphQuadsSize() const { return verticalGlyphQuadsSize; }
    std::size_t getIconQuadsSize() const { return iconQuadsSize; }
    const CollisionFeature& getTextCollisionFeature() const { return textCollisionFeature; }
    const CollisionFeature& getIconCollisionFeature() const { return iconCollisionFeature; }
    const std::optional<CollisionFeature>& getVerticalTextCollisionFeature() const {
        return verticalTextCollisionFeature;
    }
    const std::optional<CollisionFeature>& getVerticalIconCollisionFeature() const {
        return verticalIconCollisionFeature;
    }
    WritingModeType getWritingModes() const { return writingModes; }
    std::size_t getLayoutFeatureIndex() const { return layoutFeatureIndex; }
    std::size_t getDataFeatureIndex() const { return dataFeatureIndex; }
    std::array<float, 2> getTextOffset() const { return textOffset; }
    std::array<float, 2> getIconOffset() const { return iconOffset; }
    const std::u16string& getKey() const { return key; }
    std::optional<size_t> getPlacedRightTextIndex() const { return placedRightTextIndex; }
    std::optional<size_t> getPlacedCenterTextIndex() const { return placedCenterTextIndex; }
    std::optional<size_t> getPlacedLeftTextIndex() const { return placedLeftTextIndex; }
    std::optional<size_t> getPlacedVerticalTextIndex() const { return placedVerticalTextIndex; }
    std::optional<size_t> getPlacedIconIndex() const { return placedIconIndex; }
    std::optional<size_t> getPlacedVerticalIconIndex() const { return placedVerticalIconIndex; }
    float getTextBoxScale() const { return textBoxScale; }
    std::array<float, 2> getVariableTextOffset() const { return variableTextOffset; }
    bool getSingleLine() const { return singleLine; }

    uint32_t getCrossTileID() const { return crossTileID; }
    void setCrossTileID(uint32_t x) { crossTileID = x; }

    std::optional<size_t>& refPlacedRightTextIndex() { return placedRightTextIndex; }
    std::optional<size_t>& refPlacedCenterTextIndex() { return placedCenterTextIndex; }
    std::optional<size_t>& refPlacedLeftTextIndex() { return placedLeftTextIndex; }
    std::optional<size_t>& refPlacedVerticalTextIndex() { return placedVerticalTextIndex; }
    std::optional<size_t>& refPlacedIconIndex() { return placedIconIndex; }
    std::optional<size_t>& refPlacedVerticalIconIndex() { return placedVerticalIconIndex; }

    void setPlacedRightTextIndex(std::optional<size_t> x) { placedRightTextIndex = x; }
    void setPlacedCenterTextIndex(std::optional<size_t> x) { placedCenterTextIndex = x; }
    void setPlacedLeftTextIndex(std::optional<size_t> x) { placedLeftTextIndex = x; }

    static constexpr uint32_t invalidCrossTileID = std::numeric_limits<uint32_t>::max();

protected:
#if MLN_SYMBOL_GUARDS
    bool check(std::uint64_t v, int n, std::string_view source) const;
    bool checkKey(std::string_view source) const;
    void forceFailInternal(); // this is just to avoid warnings about the values never being set
#else
    bool checkKey(std::string_view) const { return true; }
#endif

private:
    std::shared_ptr<SymbolInstanceSharedData> sharedData;

    static constexpr std::uint64_t checkVal = 0x123456780ABCDEFFULL;

    SYM_GUARD_VALUE(01)
    Anchor anchor;
    SYM_GUARD_VALUE(02)
    SymbolContent symbolContent;
    SYM_GUARD_VALUE(03)

    std::size_t rightJustifiedGlyphQuadsSize;
    SYM_GUARD_VALUE(04)
    std::size_t centerJustifiedGlyphQuadsSize;
    SYM_GUARD_VALUE(05)
    std::size_t leftJustifiedGlyphQuadsSize;
    SYM_GUARD_VALUE(06)
    std::size_t verticalGlyphQuadsSize;
    SYM_GUARD_VALUE(07)
    std::size_t iconQuadsSize;
    SYM_GUARD_VALUE(08)

    CollisionFeature textCollisionFeature;
    SYM_GUARD_VALUE(09)
    CollisionFeature iconCollisionFeature;
    SYM_GUARD_VALUE(10)
    std::optional<CollisionFeature> verticalTextCollisionFeature = std::nullopt;
    SYM_GUARD_VALUE(11)
    std::optional<CollisionFeature> verticalIconCollisionFeature = std::nullopt;
    SYM_GUARD_VALUE(12)
    WritingModeType writingModes;
    SYM_GUARD_VALUE(13)
    std::size_t layoutFeatureIndex; // Index into the set of features included at layout time
    SYM_GUARD_VALUE(14)
    std::size_t dataFeatureIndex; // Index into the underlying tile data feature set
    SYM_GUARD_VALUE(15)
    std::array<float, 2> textOffset;
    SYM_GUARD_VALUE(16)
    std::array<float, 2> iconOffset;
    SYM_GUARD_VALUE(17)
    std::u16string key;
    SYM_GUARD_VALUE(18)
    std::optional<size_t> placedRightTextIndex;
    SYM_GUARD_VALUE(19)
    std::optional<size_t> placedCenterTextIndex;
    SYM_GUARD_VALUE(20)
    std::optional<size_t> placedLeftTextIndex;
    SYM_GUARD_VALUE(21)
    std::optional<size_t> placedVerticalTextIndex;
    SYM_GUARD_VALUE(22)
    std::optional<size_t> placedIconIndex;
    SYM_GUARD_VALUE(23)
    std::optional<size_t> placedVerticalIconIndex;
    SYM_GUARD_VALUE(24)
    float textBoxScale;
    SYM_GUARD_VALUE(25)
    std::array<float, 2> variableTextOffset;
    SYM_GUARD_VALUE(26)
    bool singleLine;
    SYM_GUARD_VALUE(27)
    uint32_t crossTileID = 0;
    SYM_GUARD_VALUE(28)
#if MLN_SYMBOL_GUARDS
    mutable bool isFailed = false;
#endif
};

#if MLN_SYMBOL_GUARDS
#pragma pack(pop)
#endif

using SymbolInstanceReferences = std::vector<std::reference_wrapper<const SymbolInstance>>;

} // namespace mbgl
