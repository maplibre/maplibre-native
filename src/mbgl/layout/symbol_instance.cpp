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
                                                   const optional<PositionedIcon>& shapedIcon,
                                                   const optional<PositionedIcon>& verticallyShapedIcon,
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
    const auto initHorizontalGlyphQuads = [&] (SymbolQuads& quads, const Shaping& shaping) {
        if (!shapedTextOrientations.singleLine) {
            quads = getGlyphQuads(shaping, textOffset, layout, textPlacement, imageMap, allowVerticalPlacement);
            return;
        }
        if (!singleLineInitialized) {
            rightJustifiedGlyphQuads =
                getGlyphQuads(shaping, textOffset, layout, textPlacement, imageMap, allowVerticalPlacement);
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
    return rightJustifiedGlyphQuads.empty() && centerJustifiedGlyphQuads.empty() && leftJustifiedGlyphQuads.empty() && verticalGlyphQuads.empty();
}

SymbolInstance::SymbolInstance(Anchor& anchor_,
                               std::shared_ptr<SymbolInstanceSharedData> sharedData_,
                               const ShapedTextOrientations& shapedTextOrientations,
                               const optional<PositionedIcon>& shapedIcon,
                               const optional<PositionedIcon>& verticallyShapedIcon,
                               const float textBoxScale_,
                               const float textPadding,
                               const SymbolPlacementType textPlacement,
                               const std::array<float, 2>& textOffset_,
                               const float iconBoxScale,
                               const float iconPadding,
                               const std::array<float, 2>& iconOffset_,
                               const IndexedSubfeature& indexedFeature,
                               const std::size_t layoutFeatureIndex_,
                               const std::size_t dataFeatureIndex_,
                               std::u16string key_,
                               const float overscaling,
                               const float iconRotation,
                               const float textRotation,
                               const std::array<float, 2>& variableTextOffset_,
                               bool allowVerticalPlacement,
                               const SymbolContent iconType) :
    sharedData(std::move(sharedData_)),
    anchor(anchor_),
    symbolContent(iconType),
    // Create the collision features that will be used to check whether this symbol instance can be placed
    // As a collision approximation, we can use either the vertical or any of the horizontal versions of the feature
    textCollisionFeature(sharedData->line, anchor, getAnyShaping(shapedTextOrientations), textBoxScale_, textPadding, textPlacement, indexedFeature, overscaling, textRotation),
    iconCollisionFeature(sharedData->line, anchor, shapedIcon, iconBoxScale, iconPadding, indexedFeature, iconRotation),
    writingModes(WritingModeType::None),
    layoutFeatureIndex(layoutFeatureIndex_),
    dataFeatureIndex(dataFeatureIndex_),
    textOffset(textOffset_),
    iconOffset(iconOffset_),
    key(std::move(key_)),
    textBoxScale(textBoxScale_),
    variableTextOffset(variableTextOffset_),
    singleLine(shapedTextOrientations.singleLine) {
    // 'hasText' depends on finding at least one glyph in the shaping that's also in the GlyphPositionMap
    if(!sharedData->empty()) symbolContent |= SymbolContent::Text;
    if (allowVerticalPlacement && shapedTextOrientations.vertical) {
        const float verticalPointLabelAngle = 90.0f;
        verticalTextCollisionFeature = CollisionFeature(line(), anchor, shapedTextOrientations.vertical, textBoxScale_, textPadding, textPlacement, indexedFeature, overscaling, textRotation + verticalPointLabelAngle);
        if (verticallyShapedIcon) {
            verticalIconCollisionFeature = CollisionFeature(sharedData->line,
                                                            anchor,
                                                            verticallyShapedIcon,
                                                            iconBoxScale, iconPadding,
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

const optional<SymbolQuads>& SymbolInstance::iconQuads() const {
    assert(sharedData);
    return sharedData->iconQuads;
}

bool SymbolInstance::hasText() const {
    return static_cast<bool>(symbolContent & SymbolContent::Text);
}
    
bool SymbolInstance::hasIcon() const {
    return static_cast<bool>(symbolContent & SymbolContent::IconRGBA) || hasSdfIcon();
}
    
bool SymbolInstance::hasSdfIcon() const {
    return static_cast<bool>(symbolContent & SymbolContent::IconSDF);
}

const optional<SymbolQuads>& SymbolInstance::verticalIconQuads() const {
    assert(sharedData);
    return sharedData->verticalIconQuads;
}

void SymbolInstance::releaseSharedData() {
    sharedData.reset();
}

optional<size_t> SymbolInstance::getDefaultHorizontalPlacedTextIndex() const {
    if (placedRightTextIndex) return placedRightTextIndex;
    if (placedCenterTextIndex) return placedCenterTextIndex;
    if (placedLeftTextIndex) return placedLeftTextIndex;
    return nullopt;
}

namespace {
    void logFailure(std::string msg) {
        // Log some info about where we found unexpected values, forcing it to happen
        // in the current thread to avoid the entry being lost if we are about to crash.
        const bool saveUseThread = Log::useLogThread();
        Log::useLogThread(false);
        Log::Error(Event::Crash, msg);
        Log::useLogThread(saveUseThread);
    }
}
bool SymbolInstance::check(std::size_t v, int n, std::string_view source) const {
    if (!isFailed && v != checkVal) {
        isFailed = true;
        logFailure("SymbolInstance corrupted at " + util::toString(n) + " with value " + util::toString(v) + " from '" + std::string(source) + "'");
    }
    return !isFailed;
}

bool SymbolInstance::checkKey() const {
    if (!isFailed && key.size() > 1000) {   // largest observed value=62
        isFailed = true;
        logFailure("SymbolInstance key corrupted with size=" + util::toString(key.size()));
    }
    return !isFailed;
}

bool SymbolInstance::checkIndex(const optional<std::size_t>& index, std::size_t size, std::string_view source) const {
    if (index.has_value() && *index >= size) {
        isFailed = true;
        logFailure("SymbolInstance index corrupted with value=" + util::toString(*index) + " size=" + util::toString(size) + " from '" + std::string(source) + "'");
    }
    return !isFailed;
}

// this is just to avoid warnings about the values never being set
void SymbolInstance::forceFail() {
    check01 =
    check02 =
    check03 =
    check04 =
    check05 =
    check06 =
    check07 =
    check08 =
    check09 =
    check10 =
    check11 =
    check12 =
    check13 =
    check14 =
    check15 =
    check16 =
    check17 =
    check18 =
    check19 =
    check20 =
    check21 =
    check22 =
    check23 =
    check24 =
    check25 =
    check26 =
    check27 =
    check28 =
    check29 = 0;
}

} // namespace mbgl
