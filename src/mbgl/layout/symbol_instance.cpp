#include <mbgl/layout/symbol_instance.hpp>
#include <mbgl/style/layers/symbol_layer_properties.hpp>
#include <mbgl/util/logging.hpp>

#include <utility>

namespace mbgl {

using namespace style;

namespace {

const Shaping& getAnyShaping(const ShapedTextOrientations& shapedTextOrientations) {
    if (shapedTextOrientations.right) return shapedTextOrientations.right;
    if (shapedTextOrientations.center) return shapedTextOrientations.center;
    if (shapedTextOrientations.left) return shapedTextOrientations.left;
    if (shapedTextOrientations.vertical) return shapedTextOrientations.vertical;
    return shapedTextOrientations.horizontal;
}

} // namespace

SymbolInstanceSharedData::SymbolInstanceSharedData(GeometryCoordinates line_,
                                                   const ShapedTextOrientations& shapedTextOrientations,
                                                   const std::optional<PositionedIcon>& shapedIcon,
                                                   const std::optional<PositionedIcon>& verticallyShapedIcon,
                                                   const style::SymbolLayoutProperties::Evaluated& layout,
                                                   const style::SymbolPlacementType textPlacement,
                                                   const std::array<float, 2>& textOffset,
                                                   const ImageMap& imageMap,
                                                   float iconRotation,
                                                   SymbolContent iconType,
                                                   bool hasIconTextFit,
                                                   bool allowVerticalPlacement)
    : line(std::move(line_)) {
    // Create the quads used for rendering the icon and glyphs.
    if (shapedIcon) {
        iconQuads = getIconQuads(*shapedIcon, iconRotation, iconType, hasIconTextFit);
        if (verticallyShapedIcon) {
            verticalIconQuads = getIconQuads(*verticallyShapedIcon, iconRotation, iconType, hasIconTextFit);
        }
    }

    bool singleLineInitialized = false;
    const auto initHorizontalGlyphQuads = [&](SymbolQuads& quads, const Shaping& shaping) {
        if (!shapedTextOrientations.singleLine) {
            quads = getGlyphQuads(shaping, textOffset, layout, textPlacement, imageMap, allowVerticalPlacement);
            return;
        }
        if (!singleLineInitialized) {
            rightJustifiedGlyphQuads = getGlyphQuads(
                shaping, textOffset, layout, textPlacement, imageMap, allowVerticalPlacement);
            singleLineInitialized = true;
        }
    };

    if (shapedTextOrientations.right) {
        initHorizontalGlyphQuads(rightJustifiedGlyphQuads, shapedTextOrientations.right);
    }

    if (shapedTextOrientations.center) {
        initHorizontalGlyphQuads(centerJustifiedGlyphQuads, shapedTextOrientations.center);
    }

    if (shapedTextOrientations.left) {
        initHorizontalGlyphQuads(leftJustifiedGlyphQuads, shapedTextOrientations.left);
    }

    if (shapedTextOrientations.vertical) {
        verticalGlyphQuads = getGlyphQuads(
            shapedTextOrientations.vertical, textOffset, layout, textPlacement, imageMap, allowVerticalPlacement);
    }
}

bool SymbolInstanceSharedData::empty() const {
    return rightJustifiedGlyphQuads.empty() && centerJustifiedGlyphQuads.empty() && leftJustifiedGlyphQuads.empty() &&
           verticalGlyphQuads.empty();
}

SymbolInstance::SymbolInstance(Anchor& anchor_,
                               std::shared_ptr<SymbolInstanceSharedData> sharedData_,
                               const ShapedTextOrientations& shapedTextOrientations,
                               const std::optional<PositionedIcon>& shapedIcon,
                               const std::optional<PositionedIcon>& verticallyShapedIcon,
                               const float textBoxScale_,
                               const float textPadding,
                               const SymbolPlacementType textPlacement,
                               const std::array<float, 2>& textOffset_,
                               const float iconBoxScale,
                               const Padding iconPadding,
                               const std::array<float, 2>& iconOffset_,
                               const RefIndexedSubfeature& indexedFeature,
                               const std::size_t layoutFeatureIndex_,
                               const std::size_t dataFeatureIndex_,
                               std::u16string key_,
                               const float overscaling,
                               const float iconRotation,
                               const float textRotation,
                               const std::optional<VariableAnchorOffsetCollection>& textVariableAnchorOffset_,
                               bool allowVerticalPlacement,
                               const SymbolContent iconType)
    : sharedData(std::move(sharedData_)),
      anchor(anchor_),
      symbolContent(iconType),
      // Create the collision features that will be used to check whether this
      // symbol instance can be placed As a collision approximation, we can use
      // either the vertical or any of the horizontal versions of the feature
      textCollisionFeature(sharedData->line,
                           anchor,
                           getAnyShaping(shapedTextOrientations),
                           textBoxScale_,
                           textPadding,
                           textPlacement,
                           indexedFeature,
                           overscaling,
                           textRotation),
      iconCollisionFeature(
          sharedData->line, anchor, shapedIcon, iconBoxScale, iconPadding, indexedFeature, iconRotation),
      writingModes(WritingModeType::None),
      layoutFeatureIndex(layoutFeatureIndex_),
      dataFeatureIndex(dataFeatureIndex_),
      textOffset(textOffset_),
      iconOffset(iconOffset_),
      key(std::move(key_)),
      textBoxScale(textBoxScale_),
      textVariableAnchorOffset(textVariableAnchorOffset_),
      singleLine(shapedTextOrientations.singleLine) {
    // 'hasText' depends on finding at least one glyph in the shaping that's also in the GlyphPositionMap
    if (!sharedData->empty()) symbolContent |= SymbolContent::Text;
    if (allowVerticalPlacement && shapedTextOrientations.vertical) {
        const float verticalPointLabelAngle = 90.0f;
        verticalTextCollisionFeature = CollisionFeature(line(),
                                                        anchor,
                                                        shapedTextOrientations.vertical,
                                                        textBoxScale_,
                                                        textPadding,
                                                        textPlacement,
                                                        indexedFeature,
                                                        overscaling,
                                                        textRotation + verticalPointLabelAngle);
        if (verticallyShapedIcon) {
            verticalIconCollisionFeature = CollisionFeature(sharedData->line,
                                                            anchor,
                                                            verticallyShapedIcon,
                                                            iconBoxScale,
                                                            iconPadding,
                                                            indexedFeature,
                                                            iconRotation + verticalPointLabelAngle);
        }
    }

    rightJustifiedGlyphQuadsSize = sharedData->rightJustifiedGlyphQuads.size();
    centerJustifiedGlyphQuadsSize = sharedData->centerJustifiedGlyphQuads.size();
    leftJustifiedGlyphQuadsSize = sharedData->leftJustifiedGlyphQuads.size();
    verticalGlyphQuadsSize = sharedData->verticalGlyphQuads.size();
    iconQuadsSize = sharedData->iconQuads ? sharedData->iconQuads->size() : 0;

    if (rightJustifiedGlyphQuadsSize || centerJustifiedGlyphQuadsSize || leftJustifiedGlyphQuadsSize) {
        writingModes |= WritingModeType::Horizontal;
    }

    if (verticalGlyphQuadsSize) {
        writingModes |= WritingModeType::Vertical;
    }
}

const GeometryCoordinates& SymbolInstance::line() const {
    assert(sharedData);
    return sharedData->line;
}

const SymbolQuads& SymbolInstance::rightJustifiedGlyphQuads() const {
    assert(sharedData);
    return sharedData->rightJustifiedGlyphQuads;
}

const SymbolQuads& SymbolInstance::leftJustifiedGlyphQuads() const {
    assert(sharedData);
    return sharedData->leftJustifiedGlyphQuads;
}

const SymbolQuads& SymbolInstance::centerJustifiedGlyphQuads() const {
    assert(sharedData);
    return sharedData->centerJustifiedGlyphQuads;
}

const SymbolQuads& SymbolInstance::verticalGlyphQuads() const {
    assert(sharedData);
    return sharedData->verticalGlyphQuads;
}

const std::optional<SymbolQuads>& SymbolInstance::iconQuads() const {
    assert(sharedData);
    return sharedData->iconQuads;
}

bool SymbolInstance::hasText() const {
    return symbolContent & SymbolContent::Text;
}

bool SymbolInstance::hasIcon() const {
    return symbolContent & SymbolContent::IconRGBA || hasSdfIcon();
}

bool SymbolInstance::hasSdfIcon() const {
    return symbolContent & SymbolContent::IconSDF;
}

std::vector<style::SymbolAnchorType> SymbolInstance::getTextAnchors() const {
    std::vector<style::SymbolAnchorType> result;
    if (textVariableAnchorOffset) {
        result.reserve(textVariableAnchorOffset->size());
        for (const auto& anchorOffset : *textVariableAnchorOffset) {
            result.push_back(anchorOffset.anchorType);
        }
    }

    return result;
}

const std::optional<SymbolQuads>& SymbolInstance::verticalIconQuads() const {
    assert(sharedData);
    return sharedData->verticalIconQuads;
}

void SymbolInstance::releaseSharedData() {
    sharedData.reset();
}

std::optional<size_t> SymbolInstance::getDefaultHorizontalPlacedTextIndex() const {
    if (placedRightTextIndex) return placedRightTextIndex;
    if (placedCenterTextIndex) return placedCenterTextIndex;
    if (placedLeftTextIndex) return placedLeftTextIndex;
    return std::nullopt;
}

#if MLN_SYMBOL_GUARDS
bool SymbolInstance::check(const std::source_location& source) const {
    return !isFailed && check(check01, 1, source) && check(check02, 2, source) && check(check03, 3, source) &&
           check(check04, 4, source) && check(check05, 5, source) && check(check06, 6, source) &&
           check(check07, 7, source) && check(check08, 8, source) && check(check09, 9, source) &&
           check(check10, 10, source) && check(check11, 11, source) && check(check12, 12, source) &&
           check(check13, 13, source) && check(check14, 14, source) && check(check15, 15, source) &&
           check(check16, 16, source) && check(check17, 17, source) && check(check18, 18, source) &&
           check(check19, 19, source) && check(check20, 20, source) && check(check21, 21, source) &&
           check(check22, 22, source) && check(check23, 23, source) && check(check24, 24, source) &&
           check(check25, 25, source) && check(check26, 26, source) && check(check27, 27, source) &&
           check(check28, 28, source) && checkKey(source);
}

bool SymbolInstance::checkIndexes(std::size_t textCount,
                                  std::size_t iconSize,
                                  std::size_t sdfSize,
                                  const std::source_location& source) const {
    return !isFailed && checkIndex(placedRightTextIndex, textCount, source) &&
           checkIndex(placedCenterTextIndex, textCount, source) && checkIndex(placedLeftTextIndex, textCount, source) &&
           checkIndex(placedVerticalTextIndex, textCount, source) &&
           checkIndex(placedIconIndex, hasSdfIcon() ? sdfSize : iconSize, source) &&
           checkIndex(placedVerticalIconIndex, hasSdfIcon() ? sdfSize : iconSize, source);
}

namespace {
inline std::string locationSuffix(const std::source_location& source) {
    return std::string(" from ") + source.function_name() + " (" + source.file_name() + ":" +
           util::toString(source.line()) + ")";
}
} // namespace
bool SymbolInstance::check(std::uint64_t v, int n, const std::source_location& source) const {
    if (!isFailed && v != checkVal) {
        isFailed = true;
        Log::Error(Event::Crash,
                   "SymbolInstance corrupted at " + util::toString(n) + " with value " + util::toString(v) +
                       locationSuffix(source));
    }
    return !isFailed;
}

bool SymbolInstance::checkKey(const std::source_location& source) const {
    if (!isFailed && key.size() > 10000) { // largest observed value=62
        isFailed = true;
        Log::Error(Event::Crash,
                   "SymbolInstance key corrupted with size=" + util::toString(key.size()) + locationSuffix(source));
    }
    return !isFailed;
}

bool SymbolInstance::checkIndex(const std::optional<std::size_t>& index,
                                std::size_t size,
                                const std::source_location& source) const {
    if (index.has_value() && *index >= size) {
        isFailed = true;
        Log::Error(Event::Crash,
                   "SymbolInstance index corrupted with value=" + util::toString(*index) +
                       " size=" + util::toString(size) + locationSuffix(source));
    }
    return !isFailed;
}

void SymbolInstance::forceFail() const {
    isFailed = true;
}

// this is just to avoid warnings about the values never being set
void SymbolInstance::forceFailInternal() {
    check01 = check02 = check03 = check04 = check05 = check06 = check07 = check08 = check09 = check10 = check11 =
        check12 = check13 = check14 = check15 = check16 = check17 = check18 = check19 = check20 = check21 = check22 =
            check23 = check24 = check25 = check26 = check27 = check28 = 0;
}

#endif // MLN_SYMBOL_GUARDS

} // namespace mbgl
