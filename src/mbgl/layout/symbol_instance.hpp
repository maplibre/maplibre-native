#pragma once

#include <mbgl/text/quads.hpp>
#include <mbgl/text/glyph_atlas.hpp>
#include <mbgl/text/collision_feature.hpp>
#include <mbgl/style/layers/symbol_layer_properties.hpp>
#include <mbgl/util/bitmask_operations.hpp>

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define __LINE_STRING__ STRINGIZE(__LINE__)
#define __SOURCE_LOCATION__ __FILE__ ":" __LINE_STRING__

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

enum class SymbolContent : uint8_t { None = 0, Text = 1 << 0, IconRGBA = 1 << 1, IconSDF = 1 << 2 };

struct SymbolInstanceSharedData {
    SymbolInstanceSharedData(GeometryCoordinates line,
                             const ShapedTextOrientations& shapedTextOrientations,
                             const optional<PositionedIcon>& shapedIcon,
                             const optional<PositionedIcon>& verticallyShapedIcon,
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
    optional<SymbolQuads> iconQuads;
    optional<SymbolQuads> verticalIconQuads;
};

class SymbolInstance {
public:
    SymbolInstance(Anchor& anchor_,
                   std::shared_ptr<SymbolInstanceSharedData> sharedData,
                   const ShapedTextOrientations& shapedTextOrientations,
                   const optional<PositionedIcon>& shapedIcon,
                   const optional<PositionedIcon>& verticallyShapedIcon,
                   float textBoxScale,
                   float textPadding,
                   style::SymbolPlacementType textPlacement,
                   const std::array<float, 2>& textOffset,
                   float iconBoxScale,
                   float iconPadding,
                   const std::array<float, 2>& iconOffset,
                   const IndexedSubfeature& indexedFeature,
                   std::size_t layoutFeatureIndex,
                   std::size_t dataFeatureIndex,
                   std::u16string key,
                   float overscaling,
                   float iconRotation,
                   float textRotation,
                   const std::array<float, 2>& variableTextOffset,
                   bool allowVerticalPlacement,
                   SymbolContent iconType = SymbolContent::None);

    optional<size_t> getDefaultHorizontalPlacedTextIndex() const;
    const GeometryCoordinates& line() const;
    const SymbolQuads& rightJustifiedGlyphQuads() const;
    const SymbolQuads& leftJustifiedGlyphQuads() const;
    const SymbolQuads& centerJustifiedGlyphQuads() const;
    const SymbolQuads& verticalGlyphQuads() const;
    bool hasText() const;
    bool hasIcon() const;
    bool hasSdfIcon() const;
    const optional<SymbolQuads>& iconQuads() const;
    const optional<SymbolQuads>& verticalIconQuads() const;
    void releaseSharedData();

    bool check(std::string_view source) const {
        if (isFailed)
            return false;
        check(check01, 1, source);
        check(check02, 2, source);
        check(check03, 3, source);
        check(check04, 4, source);
        check(check05, 5, source);
        check(check06, 6, source);
        check(check07, 7, source);
        check(check08, 8, source);
        check(check09, 9, source);
        check(check10, 10, source);
        check(check11, 11, source);
        check(check12, 12, source);
        check(check13, 13, source);
        check(check14, 14, source);
        check(check15, 15, source);
        check(check16, 16, source);
        check(check17, 17, source);
        check(check18, 18, source);
        check(check19, 19, source);
        check(check20, 20, source);
        check(check21, 21, source);
        check(check22, 22, source);
        check(check23, 23, source);
        check(check24, 24, source);
        check(check25, 25, source);
        check(check26, 26, source);
        check(check27, 27, source);
        check(check28, 28, source);
        check(check29, 29, source);
        return !isFailed;
    }
    bool checkIndex(const optional<std::size_t>& index, std::size_t size, std::string_view source) const;
    bool checkIndexes(std::size_t textCount, std::size_t iconSize, std::size_t sdfSize, std::string_view source) const {
        return !isFailed &&
            checkIndex(placedRightTextIndex, textCount, source) &&
            checkIndex(placedCenterTextIndex, textCount, source) &&
            checkIndex(placedLeftTextIndex, textCount, source) &&
            checkIndex(placedVerticalTextIndex, textCount, source) &&
            checkIndex(placedIconIndex, hasSdfIcon() ? sdfSize : iconSize, source) &&
            checkIndex(placedVerticalIconIndex, hasSdfIcon() ? sdfSize : iconSize, source);
    }

    const Anchor& getAnchor(std::string_view source) const { check(source); return anchor; }
    std::size_t getRightJustifiedGlyphQuadsSize(std::string_view source) const { check(source); return rightJustifiedGlyphQuadsSize; }
    std::size_t getCenterJustifiedGlyphQuadsSize(std::string_view source) const { check(source); return centerJustifiedGlyphQuadsSize; }
    std::size_t getLeftJustifiedGlyphQuadsSize(std::string_view source) const { check(source); return leftJustifiedGlyphQuadsSize; }
    std::size_t getVerticalGlyphQuadsSize(std::string_view source) const { check(source); return verticalGlyphQuadsSize; }
    std::size_t getIconQuadsSize(std::string_view source) const { check(source); return iconQuadsSize; }
    const CollisionFeature& getTextCollisionFeature(std::string_view source) const { check(source); return textCollisionFeature; }
    const CollisionFeature& getIconCollisionFeature(std::string_view source) const { check(source); return iconCollisionFeature; }
    const optional<CollisionFeature>& getVerticalTextCollisionFeature(std::string_view source) const { check(source); return verticalTextCollisionFeature; }
    const optional<CollisionFeature>& getVerticalIconCollisionFeature(std::string_view source) const { check(source); return verticalIconCollisionFeature; }
    WritingModeType getWritingModes(std::string_view source) const { check(source); return writingModes; }
    std::size_t getLayoutFeatureIndex(std::string_view source) const { check(source); return layoutFeatureIndex; }
    std::size_t getDataFeatureIndex(std::string_view source) const { check(source); return dataFeatureIndex; }
    std::array<float, 2> getTextOffset(std::string_view source) const { check(source); return textOffset; }
    std::array<float, 2> getIconOffset(std::string_view source) const { check(source); return iconOffset; }
    const std::u16string& getKey(std::string_view source) const { check(source); checkKey(); return key; }
    optional<size_t> getPlacedRightTextIndex(std::string_view source) const { check(source); return placedRightTextIndex; }
    optional<size_t> getPlacedCenterTextIndex(std::string_view source) const { check(source); return placedCenterTextIndex; }
    optional<size_t> getPlacedLeftTextIndex(std::string_view source) const { check(source); return placedLeftTextIndex; }
    optional<size_t> getPlacedVerticalTextIndex(std::string_view source) const { check(source); return placedVerticalTextIndex; }
    optional<size_t> getPlacedIconIndex(std::string_view source) const { check(source); return placedIconIndex; }
    optional<size_t> getPlacedVerticalIconIndex(std::string_view source) const { check(source); return placedVerticalIconIndex; }
    float getTextBoxScale(std::string_view source) const { check(source); return textBoxScale; }
    std::array<float, 2> getVariableTextOffset(std::string_view source) const { check(source); return variableTextOffset; }
    bool getSingleLine(std::string_view source) const { check(source); return singleLine; }

    uint32_t getCrossTileID(std::string_view source) const { check(source); return crossTileID; }
    void setCrossTileID(uint32_t x, std::string_view source) { check(source); crossTileID = x; }

    optional<size_t>& refPlacedRightTextIndex(std::string_view source) { check(source); return placedRightTextIndex; }
    optional<size_t>& refPlacedCenterTextIndex(std::string_view source) { check(source); return placedCenterTextIndex; }
    optional<size_t>& refPlacedLeftTextIndex(std::string_view source) { check(source); return placedLeftTextIndex; }
    optional<size_t>& refPlacedVerticalTextIndex(std::string_view source) { check(source); return placedVerticalTextIndex; }
    optional<size_t>& refPlacedIconIndex(std::string_view source) { check(source); return placedIconIndex; }
    optional<size_t>& refPlacedVerticalIconIndex(std::string_view source) { check(source); return placedVerticalIconIndex; }

    void setPlacedRightTextIndex(optional<size_t> x, std::string_view source) { check(source); placedRightTextIndex = x; }
    void setPlacedCenterTextIndex(optional<size_t> x, std::string_view source) { check(source); placedCenterTextIndex = x; }
    void setPlacedLeftTextIndex(optional<size_t> x, std::string_view source) { check(source); placedLeftTextIndex = x; }

    static constexpr uint32_t invalidCrossTileID() { return std::numeric_limits<uint32_t>::max(); }

protected:
    bool check(std::size_t v, int n, std::string_view source) const;
    bool checkKey() const;
    void forceFail();  // this is just to avoid warnings about the values never being set

private:
    std::shared_ptr<SymbolInstanceSharedData> sharedData;

    static constexpr std::size_t checkVal = static_cast<std::size_t>(0x123456780ABCDEFFULL);

    std::size_t check01 = checkVal;
    Anchor anchor;
    std::size_t check02 = checkVal;
    SymbolContent symbolContent;
    std::size_t check03 = checkVal;

    std::size_t rightJustifiedGlyphQuadsSize;
    std::size_t check04 = checkVal;
    std::size_t centerJustifiedGlyphQuadsSize;
    std::size_t check05 = checkVal;
    std::size_t leftJustifiedGlyphQuadsSize;
    std::size_t check06 = checkVal;
    std::size_t verticalGlyphQuadsSize;
    std::size_t check07 = checkVal;
    std::size_t iconQuadsSize;
    std::size_t check08 = checkVal;

    CollisionFeature textCollisionFeature;
    std::size_t check09 = checkVal;
    CollisionFeature iconCollisionFeature;
    std::size_t check10 = checkVal;
    optional<CollisionFeature> verticalTextCollisionFeature = nullopt;
    std::size_t check11 = checkVal;
    optional<CollisionFeature> verticalIconCollisionFeature = nullopt;
    std::size_t check12 = checkVal;
    WritingModeType writingModes;
    std::size_t check13 = checkVal;
    std::size_t layoutFeatureIndex; // Index into the set of features included at layout time
    std::size_t check14 = checkVal;
    std::size_t dataFeatureIndex;   // Index into the underlying tile data feature set
    std::size_t check15 = checkVal;
    std::array<float, 2> textOffset;
    std::size_t check16 = checkVal;
    std::array<float, 2> iconOffset;
    std::size_t check17 = checkVal;
    std::u16string key;
    std::size_t check18 = checkVal;
    //bool isDuplicate;
    std::size_t check19 = checkVal;
    optional<std::size_t> placedRightTextIndex;
    std::size_t check20 = checkVal;
    optional<std::size_t> placedCenterTextIndex;
    std::size_t check21 = checkVal;
    optional<std::size_t> placedLeftTextIndex;
    std::size_t check22 = checkVal;
    optional<std::size_t> placedVerticalTextIndex;
    std::size_t check23 = checkVal;
    optional<std::size_t> placedIconIndex;
    std::size_t check24 = checkVal;
    optional<std::size_t> placedVerticalIconIndex;
    std::size_t check25 = checkVal;
    float textBoxScale;
    std::size_t check26 = checkVal;
    std::array<float, 2> variableTextOffset;
    std::size_t check27 = checkVal;
    bool singleLine;
    std::size_t check28 = checkVal;
    uint32_t crossTileID = 0;
    std::size_t check29 = checkVal;
    mutable bool isFailed = false;
};

using SymbolInstanceReferences = std::vector<std::reference_wrapper<const SymbolInstance>>;

} // namespace mbgl
