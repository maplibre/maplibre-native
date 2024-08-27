#include <mbgl/renderer/bucket_parameters.hpp>
#include <mbgl/renderer/buckets/symbol_bucket.hpp>
#include <mbgl/renderer/layers/render_symbol_layer.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/style/layers/symbol_layer_impl.hpp>
#include <mbgl/text/cross_tile_symbol_index.hpp>
#include <mbgl/text/glyph_atlas.hpp>
#include <mbgl/text/placement.hpp>

#include <utility>

namespace mbgl {

using namespace style;
namespace {
std::atomic<uint32_t> maxBucketInstanceId;
} // namespace

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
            std::forward_as_tuple(PaintProperties{{RenderSymbolLayer::iconPaintProperties(evaluated), zoom},
                                                  {RenderSymbolLayer::textPaintProperties(evaluated), zoom}}));
    }
}

SymbolBucket::~SymbolBucket() = default;

void SymbolBucket::upload([[maybe_unused]] gfx::UploadPass& uploadPass) {
#if MLN_LEGACY_RENDERER
    if (hasTextData()) {
        if (!staticUploaded) {
            text.indexBuffer = uploadPass.createIndexBuffer(
                std::move(text.triangles),
                sortFeaturesByY ? gfx::BufferUsageType::StreamDraw : gfx::BufferUsageType::StaticDraw);
            text.vertexBuffer = uploadPass.createVertexBuffer(text.vertices());
            for (auto& pair : paintProperties) {
                pair.second.textBinders.upload(uploadPass);
            }
        } else if (!sortUploaded) {
            uploadPass.updateIndexBuffer(*text.indexBuffer, std::move(text.triangles));
        }

        if (!dynamicUploaded) {
            if (!text.dynamicVertexBuffer) {
                text.dynamicVertexBuffer = uploadPass.createVertexBuffer(text.dynamicVertices(),
                                                                         gfx::BufferUsageType::StreamDraw);
            } else {
                uploadPass.updateVertexBuffer(*text.dynamicVertexBuffer, text.dynamicVertices());
            }
        }
        if (!placementChangesUploaded) {
            if (!text.opacityVertexBuffer) {
                text.opacityVertexBuffer = uploadPass.createVertexBuffer(text.opacityVertices(),
                                                                         gfx::BufferUsageType::StreamDraw);
            } else {
                uploadPass.updateVertexBuffer(*text.opacityVertexBuffer, text.opacityVertices());
            }
        }
    }

    auto updateIconBuffer = [&](Buffer& iconBuffer) {
        if (!staticUploaded) {
            iconBuffer.indexBuffer = uploadPass.createIndexBuffer(
                std::move(iconBuffer.triangles),
                sortFeaturesByY ? gfx::BufferUsageType::StreamDraw : gfx::BufferUsageType::StaticDraw);
            iconBuffer.vertexBuffer = uploadPass.createVertexBuffer(iconBuffer.vertices());
            for (auto& pair : paintProperties) {
                pair.second.iconBinders.upload(uploadPass);
            }
        } else if (!sortUploaded) {
            uploadPass.updateIndexBuffer(*iconBuffer.indexBuffer, std::move(iconBuffer.triangles));
        }
        if (!dynamicUploaded) {
            if (!iconBuffer.dynamicVertexBuffer) {
                iconBuffer.dynamicVertexBuffer = uploadPass.createVertexBuffer(iconBuffer.dynamicVertices(),
                                                                               gfx::BufferUsageType::StreamDraw);
            } else {
                uploadPass.updateVertexBuffer(*iconBuffer.dynamicVertexBuffer, iconBuffer.dynamicVertices());
            }
        }
        if (!placementChangesUploaded) {
            if (!iconBuffer.opacityVertexBuffer) {
                iconBuffer.opacityVertexBuffer = uploadPass.createVertexBuffer(iconBuffer.opacityVertices(),
                                                                               gfx::BufferUsageType::StreamDraw);
            } else {
                uploadPass.updateVertexBuffer(*iconBuffer.opacityVertexBuffer, iconBuffer.opacityVertices());
            }
        }
    };
    if (hasIconData()) {
        updateIconBuffer(icon);
    }
    if (hasSdfIconData()) {
        updateIconBuffer(sdfIcon);
    }

    const auto updateCollisionBox = [&](CollisionBoxBuffer& collisionBox) {
        if (!staticUploaded) {
            collisionBox.indexBuffer = uploadPass.createIndexBuffer(std::move(collisionBox.lines));
            collisionBox.vertexBuffer = uploadPass.createVertexBuffer(std::move(collisionBox.vertices()));
        }
        if (!placementChangesUploaded) {
            if (!collisionBox.dynamicVertexBuffer) {
                collisionBox.dynamicVertexBuffer = uploadPass.createVertexBuffer(
                    std::move(collisionBox.dynamicVertices()), gfx::BufferUsageType::StreamDraw);
            } else {
                uploadPass.updateVertexBuffer(*collisionBox.dynamicVertexBuffer,
                                              std::move(collisionBox.dynamicVertices()));
            }
        }
    };
    if (hasIconCollisionBoxData()) {
        updateCollisionBox(*iconCollisionBox);
    }

    if (hasTextCollisionBoxData()) {
        updateCollisionBox(*textCollisionBox);
    }

    const auto updateCollisionCircle = [&](CollisionCircleBuffer& collisionCircle) {
        if (!staticUploaded) {
            collisionCircle.indexBuffer = uploadPass.createIndexBuffer(std::move(collisionCircle.triangles));
            collisionCircle.vertexBuffer = uploadPass.createVertexBuffer(std::move(collisionCircle.vertices()));
        }
        if (!placementChangesUploaded) {
            if (!collisionCircle.dynamicVertexBuffer) {
                collisionCircle.dynamicVertexBuffer = uploadPass.createVertexBuffer(
                    std::move(collisionCircle.dynamicVertices()), gfx::BufferUsageType::StreamDraw);
            } else {
                uploadPass.updateVertexBuffer(*collisionCircle.dynamicVertexBuffer,
                                              std::move(collisionCircle.dynamicVertices()));
            }
        }
    };
    if (hasIconCollisionCircleData()) {
        updateCollisionCircle(*iconCollisionCircle);
    }

    if (hasTextCollisionCircleData()) {
        updateCollisionCircle(*textCollisionCircle);
    }
#endif // MLN_LEGACY_RENDERER

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

void addPlacedSymbol(gfx::IndexVector<gfx::Triangles>& triangles, const PlacedSymbol& placedSymbol) {
    auto endIndex = placedSymbol.vertexStartIndex + placedSymbol.glyphOffsets.size() * 4;
    for (auto vertexIndex = placedSymbol.vertexStartIndex; vertexIndex < endIndex; vertexIndex += 4) {
        triangles.emplace_back(static_cast<uint16_t>(vertexIndex + 0),
                               static_cast<uint16_t>(vertexIndex + 1),
                               static_cast<uint16_t>(vertexIndex + 2));
        triangles.emplace_back(static_cast<uint16_t>(vertexIndex + 1),
                               static_cast<uint16_t>(vertexIndex + 2),
                               static_cast<uint16_t>(vertexIndex + 3));
    }
}

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

    text.triangles.clear();
    icon.triangles.clear();
    sdfIcon.triangles.clear();

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
            addPlacedSymbol(text.triangles, text.placedSymbols[*symbolInstance.getPlacedRightTextIndex()]);
        }

        if (symbolInstance.getPlacedCenterTextIndex() && !symbolInstance.getSingleLine()) {
            addPlacedSymbol(text.triangles, text.placedSymbols[*symbolInstance.getPlacedCenterTextIndex()]);
        }

        if (symbolInstance.getPlacedLeftTextIndex() && !symbolInstance.getSingleLine()) {
            addPlacedSymbol(text.triangles, text.placedSymbols[*symbolInstance.getPlacedLeftTextIndex()]);
        }

        if (symbolInstance.getPlacedVerticalTextIndex()) {
            addPlacedSymbol(text.triangles, text.placedSymbols[*symbolInstance.getPlacedVerticalTextIndex()]);
        }

        auto& iconBuffer = symbolInstance.hasSdfIcon() ? sdfIcon : icon;
        if (symbolInstance.getPlacedIconIndex()) {
            addPlacedSymbol(iconBuffer.triangles, iconBuffer.placedSymbols[*symbolInstance.getPlacedIconIndex()]);
        }

        if (symbolInstance.getPlacedVerticalIconIndex()) {
            addPlacedSymbol(iconBuffer.triangles,
                            iconBuffer.placedSymbols[*symbolInstance.getPlacedVerticalIconIndex()]);
        }
    }

    featureSortOrder = std::move(symbolsSortOrder);
}

SymbolInstanceReferences SymbolBucket::getSortedSymbols(const float angle) const {
    SymbolInstanceReferences result(symbolInstances.begin(), symbolInstances.end());
    const float sin = std::sin(angle);
    const float cos = std::cos(angle);

    std::sort(result.begin(), result.end(), [sin, cos](const SymbolInstance& a, const SymbolInstance& b) {
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
bool SymbolBucket::check(std::source_location source) {
    if (text.vertices().elements() != text.dynamicVertices().elements() ||
        text.vertices().elements() != text.opacityVertices().elements() ||
        icon.vertices().elements() != icon.dynamicVertices().elements() ||
        icon.vertices().elements() != icon.opacityVertices().elements() ||
        sdfIcon.vertices().elements() != sdfIcon.dynamicVertices().elements() ||
        sdfIcon.vertices().elements() != sdfIcon.opacityVertices().elements()) {
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

    if (placement.updateBucketDynamicVertices(*this, state, tile)) {
        dynamicUploaded = false;
        uploaded = false;
    }

    if (!uploaded) {
        text.updateModified();
        icon.updateModified();
        sdfIcon.updateModified();
    }
}

} // namespace mbgl
