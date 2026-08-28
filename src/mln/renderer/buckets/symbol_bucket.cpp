#include <mln/renderer/bucket_parameters.hpp>
#include <mln/renderer/buckets/symbol_bucket.hpp>
#include <mln/renderer/layers/render_symbol_layer.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/style/layers/symbol_layer_impl.hpp>
#include <mln/text/cross_tile_symbol_index.hpp>
#include <mln/text/placement.hpp>

#include <algorithm>
#include <utility>

namespace mln {

using namespace style;
namespace {
std::atomic<uint32_t> maxBucketInstanceId;
} // namespace

std::unique_ptr<SymbolSizeBinder> SymbolSizeBinder::create(const float tileZoom,
                                                           const style::PropertyValue<float>& sizeProperty,
                                                           const float defaultValue) {
    return sizeProperty.match(
        [&](const Undefined& value) -> std::unique_ptr<SymbolSizeBinder> {
            return std::make_unique<ConstantSymbolSizeBinder>(tileZoom, value, defaultValue);
        },
        [&](float value) -> std::unique_ptr<SymbolSizeBinder> {
            return std::make_unique<ConstantSymbolSizeBinder>(tileZoom, value, defaultValue);
        },
        [&](const style::PropertyExpression<float>& expression) -> std::unique_ptr<SymbolSizeBinder> {
            if (expression.isFeatureConstant()) {
                return std::make_unique<ConstantSymbolSizeBinder>(tileZoom, expression, defaultValue);
            } else if (expression.isZoomConstant()) {
                return std::make_unique<SourceFunctionSymbolSizeBinder>(tileZoom, expression, defaultValue);
            } else {
                return std::make_unique<CompositeFunctionSymbolSizeBinder>(tileZoom, expression, defaultValue);
            }
        });
}

SymbolBucket::SymbolBucket(Immutable<style::SymbolLayoutProperties::PossiblyEvaluated> layout_,
                           const std::map<std::string, Immutable<style::LayerProperties>>& paintProperties_,
                           const style::PropertyValue<float>& textSize,
                           const style::PropertyValue<float>& iconSize,
                           float zoom,
                           bool iconsNeedLinear_,
                           bool sortFeaturesByY_,
                           std::string bucketName_,
                           const std::vector<SymbolInstance>&& symbolInstances_,
                           const std::vector<SortKeyRange>&& sortKeyRanges_,
                           float tilePixelRatio_,
                           bool allowVerticalPlacement_,
                           std::vector<style::TextWritingModeType> placementModes_,
                           bool iconsInText_)
    : layout(std::move(layout_)),
      bucketLeaderID(std::move(bucketName_)),
      iconsNeedLinear(iconsNeedLinear_ || iconSize.isDataDriven() || !iconSize.isZoomConstant()),
      sortFeaturesByY(sortFeaturesByY_),
      staticUploaded(false),
      placementChangesUploaded(false),
      dynamicUploaded(false),
      sortUploaded(false),
      iconsInText(iconsInText_),
      justReloaded(false),
      hasVariablePlacement(false),
      hasUninitializedSymbols(false),
      symbolInstances(symbolInstances_),
      sortKeyRanges(sortKeyRanges_),
      textSizeBinder(SymbolSizeBinder::create(zoom, textSize, TextSize::defaultValue())),
      iconSizeBinder(SymbolSizeBinder::create(zoom, iconSize, IconSize::defaultValue())),
      tilePixelRatio(tilePixelRatio_),
      bucketInstanceId(++maxBucketInstanceId),
      allowVerticalPlacement(allowVerticalPlacement_),
      placementModes(std::move(placementModes_)) {
    for (const auto& pair : paintProperties_) {
        const auto& evaluated = getEvaluated<SymbolLayerProperties>(pair.second);
        paintProperties.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(pair.first),
            std::forward_as_tuple(PaintProperties{.iconBinders = {iconPaintProperties(evaluated), zoom},
                                                  .textBinders = {textPaintProperties(evaluated), zoom}}));
    }
}

SymbolBucket::~SymbolBucket() = default;

// static
style::IconPaintProperties::PossiblyEvaluated SymbolBucket::iconPaintProperties(
    const style::SymbolPaintProperties::PossiblyEvaluated& evaluated_) {
    return style::IconPaintProperties::PossiblyEvaluated{evaluated_.get<style::IconOpacity>(),
                                                         evaluated_.get<style::IconColor>(),
                                                         evaluated_.get<style::IconHaloColor>(),
                                                         evaluated_.get<style::IconHaloWidth>(),
                                                         evaluated_.get<style::IconHaloBlur>(),
                                                         evaluated_.get<style::IconTranslate>(),
                                                         evaluated_.get<style::IconTranslateAnchor>()};
}

// static
style::TextPaintProperties::PossiblyEvaluated SymbolBucket::textPaintProperties(
    const style::SymbolPaintProperties::PossiblyEvaluated& evaluated_) {
    return style::TextPaintProperties::PossiblyEvaluated{evaluated_.get<style::TextOpacity>(),
                                                         evaluated_.get<style::TextColor>(),
                                                         evaluated_.get<style::TextHaloColor>(),
                                                         evaluated_.get<style::TextHaloWidth>(),
                                                         evaluated_.get<style::TextHaloBlur>(),
                                                         evaluated_.get<style::TextTranslate>(),
                                                         evaluated_.get<style::TextTranslateAnchor>()};
}

void SymbolBucket::upload([[maybe_unused]] gfx::UploadPass& uploadPass) {
    uploaded = true;
    staticUploaded = true;
    placementChangesUploaded = true;
    dynamicUploaded = true;
    sortUploaded = true;
}

bool SymbolBucket::hasData() const {
    return hasTextData() || hasIconData() || hasSdfIconData() || hasIconCollisionBoxData() ||
           hasTextCollisionBoxData() || hasIconCollisionCircleData() || hasTextCollisionCircleData();
}

void SymbolBucket::update(const FeatureStates& states,
                          const GeometryTileLayer& layer,
                          const std::string& layerID,
                          const ImagePositions& imagePositions) {
    auto it = paintProperties.find(layerID);
    if (it != paintProperties.end()) {
        it->second.iconBinders.updateVertexVectors(states, layer, imagePositions);
        it->second.textBinders.updateVertexVectors(states, layer, imagePositions);
        uploaded = false;
    }
}

bool SymbolBucket::hasTextData() const {
    return !text.segments.empty();
}

bool SymbolBucket::hasIconData() const {
    return !icon.segments.empty();
}

bool SymbolBucket::hasSdfIconData() const {
    return !sdfIcon.segments.empty();
}

bool SymbolBucket::hasIconCollisionBoxData() const {
    return iconCollisionBox && !iconCollisionBox->segments.empty();
}

bool SymbolBucket::hasIconCollisionCircleData() const {
    return iconCollisionCircle && !iconCollisionCircle->segments.empty();
}

bool SymbolBucket::hasTextCollisionBoxData() const {
    return textCollisionBox && !textCollisionBox->segments.empty();
}

bool SymbolBucket::hasTextCollisionCircleData() const {
    return textCollisionCircle && !textCollisionCircle->segments.empty();
}

#if MLN_USE_SYMBOL_INSTANCING
void addPlacedSymbol(SymbolBucket::SortedInstanceVector& sortedInstances, const PlacedSymbol& placedSymbol) {
    auto endIndex = placedSymbol.startIndex + placedSymbol.glyphOffsets.size() * 1;
    for (auto instanceIndex = placedSymbol.startIndex; instanceIndex < endIndex; instanceIndex += 1) {
        sortedInstances.emplace_back(SymbolSortedInstance{static_cast<uint16_t>(instanceIndex)});
    }
}
#else
void addPlacedSymbol(SymbolBucket::TriangleIndexVector& triangles, const PlacedSymbol& placedSymbol) {
    auto endIndex = placedSymbol.startIndex + placedSymbol.glyphOffsets.size() * 4;
    for (auto vertexIndex = placedSymbol.startIndex; vertexIndex < endIndex; vertexIndex += 4) {
        triangles.emplace_back(static_cast<uint16_t>(vertexIndex + 0),
                               static_cast<uint16_t>(vertexIndex + 1),
                               static_cast<uint16_t>(vertexIndex + 2));
        triangles.emplace_back(static_cast<uint16_t>(vertexIndex + 1),
                               static_cast<uint16_t>(vertexIndex + 2),
                               static_cast<uint16_t>(vertexIndex + 3));
    }
}
#endif

void SymbolBucket::sortFeatures(const float angle) {
    if (!sortFeaturesByY) {
        return;
    }
    assert(angle != std::numeric_limits<float>::max());

    if (sortedAngle == angle) {
        return;
    }

    sortedAngle = angle;

    // The current approach to sorting doesn't sort across text and icon
    // segments so don't try. Sorting within segments separately seemed not to
    // be worth the complexity.
    if (text.segments.size() > 1 || (icon.segments.size() > 1 || sdfIcon.segments.size() > 1)) {
        return;
    }

    sortUploaded = false;
    uploaded = false;

#if MLN_USE_SYMBOL_INSTANCING
    auto& textVector = text.sortedInstances();
    auto& iconVector = icon.sortedInstances();
    auto& sdfIconVector = sdfIcon.sortedInstances();
#else
    auto& textVector = text.triangles;
    auto& iconVector = icon.triangles;
    auto& sdfIconVector = sdfIcon.triangles;
#endif

    textVector.clear();
    iconVector.clear();
    sdfIconVector.clear();

    auto symbolsSortOrder = std::make_unique<std::vector<size_t>>();
    symbolsSortOrder->reserve(symbolInstances.size());

    // If the symbols are allowed to overlap sort them by their vertical screen
    // position. The index array buffer is rewritten to reference the
    // (unchanged) vertices in the sorted order.
    for (const SymbolInstance& symbolInstance : getSortedSymbols(angle)) {
        if (!symbolInstance.check(SYM_GUARD_LOC) ||
            !symbolInstance.checkIndexes(
                text.placedSymbols.size(), icon.placedSymbols.size(), sdfIcon.placedSymbols.size(), SYM_GUARD_LOC)) {
            continue;
        }
        symbolsSortOrder->push_back(symbolInstance.getDataFeatureIndex());

        if (symbolInstance.getPlacedRightTextIndex()) {
            addPlacedSymbol(textVector, text.placedSymbols[*symbolInstance.getPlacedRightTextIndex()]);
        }

        if (symbolInstance.getPlacedCenterTextIndex() && !symbolInstance.getSingleLine()) {
            addPlacedSymbol(textVector, text.placedSymbols[*symbolInstance.getPlacedCenterTextIndex()]);
        }

        if (symbolInstance.getPlacedLeftTextIndex() && !symbolInstance.getSingleLine()) {
            addPlacedSymbol(textVector, text.placedSymbols[*symbolInstance.getPlacedLeftTextIndex()]);
        }

        if (symbolInstance.getPlacedVerticalTextIndex()) {
            addPlacedSymbol(textVector, text.placedSymbols[*symbolInstance.getPlacedVerticalTextIndex()]);
        }

        auto& iconBuffer = symbolInstance.hasSdfIcon() ? sdfIcon : icon;
        auto& iconVectorBuffer = symbolInstance.hasSdfIcon() ? sdfIconVector : iconVector;
        if (symbolInstance.getPlacedIconIndex()) {
            addPlacedSymbol(iconVectorBuffer, iconBuffer.placedSymbols[*symbolInstance.getPlacedIconIndex()]);
        }

        if (symbolInstance.getPlacedVerticalIconIndex()) {
            addPlacedSymbol(iconVectorBuffer, iconBuffer.placedSymbols[*symbolInstance.getPlacedVerticalIconIndex()]);
        }
    }

    featureSortOrder = std::move(symbolsSortOrder);
}

SymbolInstanceReferences SymbolBucket::getSortedSymbols(const float angle) const {
    SymbolInstanceReferences result(symbolInstances.begin(), symbolInstances.end());
    const float sin = std::sin(angle);
    const float cos = std::cos(angle);

    std::ranges::sort(result, [sin, cos](const SymbolInstance& a, const SymbolInstance& b) {
        const auto aRotated = std::lround(sin * a.getAnchor().point.x + cos * a.getAnchor().point.y);
        const auto bRotated = std::lround(sin * b.getAnchor().point.x + cos * b.getAnchor().point.y);
        if (aRotated != bRotated) {
            return aRotated < bRotated;
        }
        return a.getDataFeatureIndex() > b.getDataFeatureIndex(); // aRotated == bRotated
    });

    return result;
}

SymbolInstanceReferences SymbolBucket::getSymbols(const std::optional<SortKeyRange>& range) const {
    assert(!range || range->start < range->end);
    assert(!range || range->end <= symbolInstances.size());
    if (!range || range->start >= range->end || range->end > symbolInstances.size()) {
        return {symbolInstances.begin(), symbolInstances.end()};
    }
    using offset_t = decltype(symbolInstances)::difference_type;
    return {symbolInstances.begin() + static_cast<offset_t>(range->start),
            symbolInstances.begin() + static_cast<offset_t>(range->end)};
}

#if MLN_SYMBOL_GUARDS
bool SymbolBucket::check(source_location source) {
    if (text.attributeData().elements() != text.dynamicAttributeData().elements() ||
        text.attributeData().elements() != text.opacityAttributeData().elements() ||
        icon.attributeData().elements() != icon.dynamicAttributeData().elements() ||
        icon.attributeData().elements() != icon.opacityAttributeData().elements() ||
        sdfIcon.attributeData().elements() != sdfIcon.dynamicAttributeData().elements() ||
        sdfIcon.attributeData().elements() != sdfIcon.opacityAttributeData().elements()) {
        // This bucket was left in a partial state and it cannot be used
        return false;
    }

    for (std::size_t i = 0; i < symbolInstances.size(); ++i) {
        if (!symbolInstances[i].check(source)) {
            return false;
        }
    }
    return true;
}
#endif

bool SymbolBucket::hasFormatSectionOverrides() const {
    if (!hasFormatSectionOverrides_) {
        hasFormatSectionOverrides_ = SymbolLayerPaintPropertyOverrides::hasOverrides(layout->get<TextField>());
    }
    return *hasFormatSectionOverrides_;
}

bool SymbolBucket::hasVariableTextAnchors() const {
    auto hasTextVariableAnchorOffset = [&]() -> bool {
        auto tvao = layout->get<TextVariableAnchorOffset>();
        if (tvao.isConstant()) {
            const auto constValue = tvao.constant();
            return constValue && !constValue->empty();
        } else if (tvao.isExpression()) {
            return true;
        }

        return false;
    };

    return hasTextVariableAnchorOffset() || !layout->get<TextVariableAnchor>().empty();
}

std::pair<uint32_t, bool> SymbolBucket::registerAtCrossTileIndex(CrossTileSymbolLayerIndex& index,
                                                                 const RenderTile& renderTile) {
    bool firstTimeAdded = index.addBucket(renderTile.getOverscaledTileID(), renderTile.matrix, *this);
    return std::make_pair(bucketInstanceId, firstTimeAdded);
}

void SymbolBucket::place(Placement& placement, const BucketPlacementData& data, std::set<uint32_t>& seenIds) {
    placement.placeSymbolBucket(data, seenIds);
}

void SymbolBucket::updateVertices(const Placement& placement,
                                  bool updateOpacities,
                                  const TransformState& state,
                                  const RenderTile& tile,
                                  std::set<uint32_t>& seenIds) {
    if (updateOpacities) {
        placement.updateBucketOpacities(*this, state, seenIds);
        placementChangesUploaded = false;
        uploaded = false;
    }

    if (placement.updateBucketDynamicAttributeData(*this, state, tile)) {
        dynamicUploaded = false;
        uploaded = false;
    }

    if (!uploaded) {
        text.updateModified();
        icon.updateModified();
        sdfIcon.updateModified();
    }
}

} // namespace mln
