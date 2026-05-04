#include <mbgl/text/placement.hpp>

#include <mbgl/layout/symbol_layout.hpp>
#include <mbgl/renderer/bucket.hpp>
#include <mbgl/renderer/buckets/symbol_bucket.hpp>
#include <mbgl/renderer/render_layer.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/tile/geometry_tile.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/math.hpp>

#include <list>
#include <utility>

namespace mbgl {

OpacityState::OpacityState(bool placed_, bool skipFade)
    : opacity((skipFade && placed_) ? 1.0f : 0.0f),
      placed(placed_) {}

OpacityState::OpacityState(const OpacityState& prevState, float increment, bool placed_)
    : opacity(std::fmax(0.0f, std::fmin(1.0f, prevState.opacity + (prevState.placed ? increment : -increment)))),
      placed(placed_) {}

bool OpacityState::isHidden() const {
    return opacity == 0 && !placed;
}

JointOpacityState::JointOpacityState(bool placedText, bool placedIcon, bool skipFade)
    : icon(OpacityState(placedIcon, skipFade)),
      text(OpacityState(placedText, skipFade)) {}

JointOpacityState::JointOpacityState(const JointOpacityState& prevOpacityState,
                                     float increment,
                                     bool placedText,
                                     bool placedIcon)
    : icon(OpacityState(prevOpacityState.icon, increment, placedIcon)),
      text(OpacityState(prevOpacityState.text, increment, placedText)) {}

bool JointOpacityState::isHidden() const {
    return icon.isHidden() && text.isHidden();
}

const CollisionGroups::CollisionGroup& CollisionGroups::get(const std::string& sourceID) {
    // The predicate/groupID mechanism allows for arbitrary grouping,
    // but the current interface defines one source == one group when
    // crossSourceCollisions == true.
    if (!crossSourceCollisions) {
        if (!collisionGroups.contains(sourceID)) {
            uint16_t nextGroupID = ++maxGroupID;
            collisionGroups.emplace(
                sourceID,
                CollisionGroup(nextGroupID,
                               std::optional<Predicate>([nextGroupID](const RefIndexedSubfeature& feature) -> bool {
                                   return feature.getCollisionGroupId() == nextGroupID;
                               })));
        }
        return collisionGroups[sourceID];
    } else {
        static CollisionGroup nullGroup{0, std::nullopt};
        return nullGroup;
    }
}

using namespace style;

// PlacementContext implementation
class PlacementContext {
    std::reference_wrapper<const SymbolBucket> bucket;
    std::reference_wrapper<const RenderTile> renderTile;
    std::reference_wrapper<const TransformState> state;

public:
    PlacementContext(const SymbolBucket& bucket_,
                     const RenderTile& renderTile_,
                     const TransformState& state_,
                     float placementZoom,
                     CollisionGroups::CollisionGroup collisionGroup_,
                     std::optional<CollisionBoundaries> avoidEdges_ = std::nullopt)
        : bucket(bucket_),
          renderTile(renderTile_),
          state(state_),
          pixelsToTileUnits(renderTile_.id.pixelsToTileUnits(1, placementZoom)),
          scale(static_cast<float>(std::pow(2, placementZoom - getOverscaledID().overscaledZ))),
          pixelRatio(static_cast<float>(util::tileSize_D * getOverscaledID().overscaleFactor() / util::EXTENT)),
          collisionGroup(std::move(collisionGroup_)),
          partiallyEvaluatedTextSize(bucket_.textSizeBinder->evaluateForZoom(placementZoom)),
          partiallyEvaluatedIconSize(bucket_.iconSizeBinder->evaluateForZoom(placementZoom)),
          avoidEdges(std::move(avoidEdges_)) {}

    const SymbolBucket& getBucket() const { return bucket.get(); }
    const style::SymbolLayoutProperties::PossiblyEvaluated& getLayout() const { return *getBucket().layout; }
    const RenderTile& getRenderTile() const { return renderTile.get(); }

    const OverscaledTileID& getOverscaledID() const { return renderTile.get().getOverscaledTileID(); }

    const TransformState& getTransformState() const { return state; }

    float pixelsToTileUnits;
    float scale;
    float pixelRatio;

    bool rotateTextWithMap = getLayout().get<TextRotationAlignment>() == AlignmentType::Map;
    bool pitchTextWithMap = getLayout().get<TextPitchAlignment>() == AlignmentType::Map;
    bool rotateIconWithMap = getLayout().get<IconRotationAlignment>() == AlignmentType::Map;
    bool pitchIconWithMap = getLayout().get<IconPitchAlignment>() == AlignmentType::Map;
    SymbolPlacementType placementType = getLayout().get<SymbolPlacement>();

    mat4 textLabelPlaneMatrix = getLabelPlaneMatrix(
        renderTile.get().matrix, pitchTextWithMap, rotateTextWithMap, state, pixelsToTileUnits);
    mat4 iconLabelPlaneMatrix =
        (rotateTextWithMap == rotateIconWithMap && pitchTextWithMap == pitchIconWithMap)
            ? textLabelPlaneMatrix
            : getLabelPlaneMatrix(
                  renderTile.get().matrix, pitchIconWithMap, rotateIconWithMap, state, pixelsToTileUnits);

    CollisionGroups::CollisionGroup collisionGroup;
    ZoomEvaluatedSize partiallyEvaluatedTextSize;
    ZoomEvaluatedSize partiallyEvaluatedIconSize;

    bool textAllowOverlap = getLayout().get<style::TextAllowOverlap>();
    bool iconAllowOverlap = getLayout().get<style::IconAllowOverlap>();
    // This logic is similar to the "defaultOpacityState" logic below in
    // updateBucketOpacities If we know a symbol is always supposed to show,
    // force it to be marked visible even if it wasn't placed into the collision
    // index (because some or all of it was outside the range of the collision
    // grid). There is a subtle edge case here we're accepting:
    //  Symbol A has text-allow-overlap: true, icon-allow-overlap: true,
    //  icon-optional: false A's icon is outside the grid, so doesn't get placed
    //  A's text would be inside grid, but doesn't get placed because of
    //  icon-optional: false We still show A because of the allow-overlap
    //  settings. Symbol B has allow-overlap: false, and gets placed where A's
    //  text would be On panning in, there is a short period when Symbol B and
    //  Symbol A will overlap This is the reverse of our normal policy of "fade
    //  in on pan", but should look like any other collision and hopefully not
    //  be too noticeable.
    // See https://github.com/mapbox/mapbox-gl-native/issues/12683
    bool alwaysShowText = textAllowOverlap &&
                          (iconAllowOverlap || !(getBucket().hasIconData() || getBucket().hasSdfIconData()) ||
                           getLayout().get<style::IconOptional>());
    bool alwaysShowIcon = iconAllowOverlap &&
                          (textAllowOverlap || !getBucket().hasTextData() || getLayout().get<style::TextOptional>());

    bool hasIconTextFit = getLayout().get<IconTextFit>() != IconTextFitType::None;

    std::optional<CollisionBoundaries> avoidEdges;
};

// PlacementController implementation

PlacementController::PlacementController()
    : placement(makeMutable<Placement>()) {}

void PlacementController::setPlacement(Immutable<Placement> placement_) {
    placement = std::move(placement_);
    stale = false;
}

bool PlacementController::placementIsRecent(TimePoint now,
                                            const float zoom,
                                            std::optional<Duration> periodOverride) const {
    if (!placement->transitionsEnabled()) return false;

    auto updatePeriod = periodOverride ? *periodOverride : placement->getUpdatePeriod(zoom);

    return placement->getCommitTime() + updatePeriod > now;
}

bool PlacementController::hasTransitions(TimePoint now) const {
    if (!placement->transitionsEnabled()) return false;

    if (stale) return true;

    return placement->hasTransitions(now);
}

// Placement implementation

Placement::Placement(std::shared_ptr<const UpdateParameters> updateParameters_,
                     std::optional<Immutable<Placement>> prevPlacement_)
    : updateParameters(std::move(updateParameters_)),
      collisionIndex(updateParameters->transformState, updateParameters->mode),
      transitionOptions(updateParameters->transitionOptions),
      commitTime(updateParameters->timePoint),
      placementZoom(static_cast<float>(updateParameters->transformState.getZoom())),
      collisionGroups(updateParameters->crossSourceCollisions),
      prevPlacement(std::move(prevPlacement_)),
      showCollisionBoxes(updateParameters->debugOptions & MapDebugOptions::Collision) {
    if (prevPlacement) {
        prevPlacement->get()->prevPlacement = std::nullopt; // Only hold on to one placement back
    }
}

Placement::Placement()
    : collisionIndex({}, MapMode::Static),
      collisionGroups(true) {}

Placement::~Placement() = default;

void Placement::placeLayers(const RenderLayerReferences& layers) {
    for (auto it = layers.crbegin(); it != layers.crend(); ++it) {
        std::set<uint32_t> seenCrossTileIDs;
        placeLayer(*it, seenCrossTileIDs);
    }
    commit();
}

void Placement::placeLayer(const RenderLayer& layer, std::set<uint32_t>& seenCrossTileIDs) {
    for (const BucketPlacementData& data : layer.getPlacementData()) {
        Bucket& bucket = data.bucket;
        bucket.place(*this, data, seenCrossTileIDs);
    }
}

namespace {
Point<float> calculateVariableLayoutOffset(style::SymbolAnchorType anchor,
                                           float width,
                                           float height,
                                           std::array<float, 2> offset,
                                           float textBoxScale,
                                           bool rotateWithMap,
                                           bool pitchWithMap,
                                           float bearing) {
    AnchorAlignment alignment = AnchorAlignment::getAnchorAlignment(anchor);
    float shiftX = -(alignment.horizontalAlign - 0.5f) * width;
    float shiftY = -(alignment.verticalAlign - 0.5f) * height;
    Point<float> shift{shiftX + offset[0] * textBoxScale, shiftY + offset[1] * textBoxScale};
    if (rotateWithMap) {
        shift = util::rotate(shift, pitchWithMap ? bearing : -bearing);
    }
    return shift;
}
} // namespace

void Placement::placeSymbolBucket(const BucketPlacementData& params, std::set<uint32_t>& seenCrossTileIDs) {
    assert(updateParameters);
    const auto& symbolBucket = static_cast<const SymbolBucket&>(params.bucket.get());
    const RenderTile& renderTile = params.tile;
    PlacementContext ctx{symbolBucket,
                         params.tile,
                         collisionIndex.getTransformState(),
                         placementZoom,
                         collisionGroups.get(params.sourceId),
                         getAvoidEdges(symbolBucket, renderTile.matrix)};
    for (const SymbolInstance& symbol : getSortedSymbols(params, ctx.pixelRatio)) {
        if (!symbol.check(SYM_GUARD_LOC)) continue;
        if (seenCrossTileIDs.contains(symbol.getCrossTileID())) continue;
        placeSymbol(symbol, ctx);

        // Prevent a flickering issue while zooming out.
        if (symbol.getCrossTileID() != SymbolInstance::invalidCrossTileID && !ctx.getRenderTile().holdForFade()) {
            seenCrossTileIDs.insert(symbol.getCrossTileID());
        }
    }

    // Prevent a flickering issue when a symbol is moved.
    symbolBucket.justReloaded = false;

    // As long as this placement lives, we have to hold onto this bucket's
    // matching FeatureIndex/data for querying purposes
    retainedQueryData.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(symbolBucket.bucketInstanceId),
        std::forward_as_tuple(symbolBucket.bucketInstanceId, params.featureIndex, ctx.getOverscaledID()));
}

JointPlacement Placement::placeSymbol(const SymbolInstance& symbolInstance, const PlacementContext& ctx) {
    static const JointPlacement kUnplaced(false, false, false);
    if (!symbolInstance.check(SYM_GUARD_LOC)) return kUnplaced;
    if (symbolInstance.getCrossTileID() == SymbolInstance::invalidCrossTileID) return kUnplaced;

    if (ctx.getRenderTile().holdForFade()) {
        // Mark all symbols from this tile as "not placed", but don't add to
        // seenCrossTileIDs, because we don't know yet if we have a duplicate in
        // a parent tile that _should_ be placed.
        return kUnplaced;
    }
    const SymbolBucket& bucket = ctx.getBucket();
    const mat4& posMatrix = ctx.getRenderTile().matrix;
    const auto& collisionGroup = ctx.collisionGroup;
    auto variableTextAnchors = symbolInstance.getTextAnchors();
    textBoxes.clear();
    iconBoxes.clear();

    bool placeText = false;
    bool placeIcon = false;
    bool offscreen = true;
    std::pair<bool, bool> placed{false, false};
    std::pair<bool, bool> placedVerticalText{false, false};
    std::pair<bool, bool> placedVerticalIcon{false, false};
    Point<float> shift{0.0f, 0.0f};
    std::optional<size_t> horizontalTextIndex = symbolInstance.getDefaultHorizontalPlacedTextIndex();
    if (horizontalTextIndex) {
        const PlacedSymbol& placedSymbol = bucket.text.placedSymbols.at(*horizontalTextIndex);
        const float fontSize = evaluateSizeForFeature(ctx.partiallyEvaluatedTextSize, placedSymbol);

        const auto updatePreviousOrientationIfNotPlaced = [&](bool isPlaced) {
            if (bucket.allowVerticalPlacement && !isPlaced && getPrevPlacement()) {
                auto prevOrientation = getPrevPlacement()->placedOrientations.find(symbolInstance.getCrossTileID());
                if (prevOrientation != getPrevPlacement()->placedOrientations.end()) {
                    placedOrientations[symbolInstance.getCrossTileID()] = prevOrientation->second;
                }
            }
        };

        const auto placeTextForPlacementModes = [&](auto& placeHorizontalFn, auto& placeVerticalFn) {
            if (bucket.allowVerticalPlacement && symbolInstance.getWritingModes() & WritingModeType::Vertical) {
                assert(!bucket.placementModes.empty());
                for (auto& placementMode : bucket.placementModes) {
                    if (placementMode == TextWritingModeType::Vertical) {
                        placedVerticalText = placed = placeVerticalFn();
                    } else {
                        placed = placeHorizontalFn();
                    }

                    if (placed.first) {
                        break;
                    }
                }
            } else {
                placed = placeHorizontalFn();
            }
        };

        // Line or point label placement
        if (variableTextAnchors.empty()) {
            const auto placeFeature = [&](const CollisionFeature& collisionFeature,
                                          style::TextWritingModeType orientation) {
                textBoxes.clear();
                auto placedFeature = collisionIndex.placeFeature(collisionFeature,
                                                                 {},
                                                                 posMatrix,
                                                                 ctx.textLabelPlaneMatrix,
                                                                 ctx.pixelRatio,
                                                                 placedSymbol,
                                                                 ctx.scale,
                                                                 fontSize,
                                                                 ctx.textAllowOverlap,
                                                                 ctx.pitchTextWithMap,
                                                                 showCollisionBoxes,
                                                                 ctx.avoidEdges,
                                                                 collisionGroup.second,
                                                                 textBoxes);
                if (placedFeature.first) {
                    placedOrientations.emplace(symbolInstance.getCrossTileID(), orientation);
                }
                return placedFeature;
            };

            const auto placeHorizontal = [&] {
                return placeFeature(symbolInstance.getTextCollisionFeature(), style::TextWritingModeType::Horizontal);
            };

            const auto placeVertical = [&] {
                if (bucket.allowVerticalPlacement && symbolInstance.getVerticalTextCollisionFeature()) {
                    return placeFeature(*symbolInstance.getVerticalTextCollisionFeature(),
                                        style::TextWritingModeType::Vertical);
                }
                return std::pair<bool, bool>{false, false};
            };

            placeTextForPlacementModes(placeHorizontal, placeVertical);
            updatePreviousOrientationIfNotPlaced(placed.first);

            placeText = placed.first;
            offscreen &= placed.second;
        } else if (!symbolInstance.getTextCollisionFeature().alongLine &&
                   !symbolInstance.getTextCollisionFeature().boxes.empty()) {
            // If this symbol was in the last placement, shift the previously
            // used anchor to the front of the anchor list, only if the previous
            // anchor is still in the anchor list.
            if (getPrevPlacement()) {
                auto prevOffset = getPrevPlacement()->variableOffsets.find(symbolInstance.getCrossTileID());
                if (prevOffset != getPrevPlacement()->variableOffsets.end()) {
                    const auto prevAnchor = prevOffset->second.anchor;
                    auto found = std::find(variableTextAnchors.begin(), variableTextAnchors.end(), prevAnchor);
                    if (found != variableTextAnchors.begin() && found != variableTextAnchors.end()) {
                        std::vector<style::TextVariableAnchorType> filtered{prevAnchor};
                        if (!isTiltedView()) {
                            for (auto anchor : variableTextAnchors) {
                                if (anchor != prevAnchor) {
                                    filtered.push_back(anchor);
                                }
                            }
                        }
                        variableTextAnchors = std::move(filtered);
                    }
                }
            }

            const bool doVariableIconPlacement = ctx.hasIconTextFit && !ctx.iconAllowOverlap &&
                                                 symbolInstance.getPlacedIconIndex();
            const auto placeFeatureForVariableAnchors = [&](const CollisionFeature& textCollisionFeature,
                                                            style::TextWritingModeType orientation,
                                                            const CollisionFeature& iconCollisionFeature) {
                const CollisionBox& textBox = textCollisionFeature.boxes[0];
                const float width = textBox.x2 - textBox.x1;
                const float height = textBox.y2 - textBox.y1;
                const float textBoxScale = symbolInstance.getTextBoxScale();
                std::pair<bool, bool> placedFeature = {false, false};
                const size_t anchorsSize = variableTextAnchors.size();
                const size_t placementAttempts = ctx.textAllowOverlap ? anchorsSize * 2 : anchorsSize;
                for (size_t i = 0u; i < placementAttempts; ++i) {
                    // when anchorsSize is 0, placementAttempts is also 0,
                    // so this code would not be reached
                    // NOLINTNEXTLINE(clang-analyzer-core.DivideZero)
                    auto anchor = variableTextAnchors[i % anchorsSize];
                    auto variableTextOffset = symbolInstance.getTextVariableAnchorOffset()->getOffsetByAnchor(anchor);
                    const bool allowOverlap = (i >= anchorsSize);
                    shift = calculateVariableLayoutOffset(anchor,
                                                          width,
                                                          height,
                                                          variableTextOffset,
                                                          textBoxScale,
                                                          ctx.rotateTextWithMap,
                                                          ctx.pitchTextWithMap,
                                                          static_cast<float>(ctx.getTransformState().getBearing()));
                    textBoxes.clear();
                    if (!canPlaceAtVariableAnchor(
                            textBox, anchor, shift, variableTextAnchors, posMatrix, ctx.pixelRatio)) {
                        continue;
                    }

                    placedFeature = collisionIndex.placeFeature(textCollisionFeature,
                                                                shift,
                                                                posMatrix,
                                                                mat4(),
                                                                ctx.pixelRatio,
                                                                placedSymbol,
                                                                ctx.scale,
                                                                fontSize,
                                                                allowOverlap,
                                                                ctx.pitchTextWithMap,
                                                                showCollisionBoxes,
                                                                ctx.avoidEdges,
                                                                collisionGroup.second,
                                                                textBoxes);

                    if (doVariableIconPlacement) {
                        auto placedIconFeature = collisionIndex.placeFeature(
                            iconCollisionFeature,
                            shift,
                            posMatrix,
                            ctx.iconLabelPlaneMatrix,
                            ctx.pixelRatio,
                            placedSymbol,
                            ctx.scale,
                            fontSize,
                            ctx.iconAllowOverlap,
                            ctx.pitchTextWithMap, // TODO: shall it be pitchIconWithMap?
                            showCollisionBoxes,
                            ctx.avoidEdges,
                            collisionGroup.second,
                            iconBoxes);
                        iconBoxes.clear();
                        if (!placedIconFeature.first) continue;
                    }

                    if (placedFeature.first) {
                        assert(symbolInstance.getCrossTileID() != 0u);
                        std::optional<style::TextVariableAnchorType> prevAnchor;

                        // If this label was placed in the previous
                        // placement, record the anchor position to allow us
                        // to animate the transition
                        if (getPrevPlacement()) {
                            auto prevOffset = getPrevPlacement()->variableOffsets.find(symbolInstance.getCrossTileID());
                            auto prevPlacements = getPrevPlacement()->placements.find(symbolInstance.getCrossTileID());
                            if (prevOffset != getPrevPlacement()->variableOffsets.end() &&
                                prevPlacements != getPrevPlacement()->placements.end() && prevPlacements->second.text) {
                                // TODO: The prevAnchor seems to be unused, needs to be fixed.
                                prevAnchor = prevOffset->second.anchor;
                            }
                        }

                        variableOffsets.insert(std::make_pair(symbolInstance.getCrossTileID(),
                                                              VariableOffset{.offset = variableTextOffset,
                                                                             .width = width,
                                                                             .height = height,
                                                                             .anchor = anchor,
                                                                             .textBoxScale = textBoxScale,
                                                                             .prevAnchor = prevAnchor}));

                        if (bucket.allowVerticalPlacement) {
                            placedOrientations.emplace(symbolInstance.getCrossTileID(), orientation);
                        }
                        break;
                    }
                }

                return placedFeature;
            };

            const auto placeHorizontal = [&] {
                return placeFeatureForVariableAnchors(symbolInstance.getTextCollisionFeature(),
                                                      style::TextWritingModeType::Horizontal,
                                                      symbolInstance.getIconCollisionFeature());
            };

            const auto placeVertical = [&] {
                if (bucket.allowVerticalPlacement && !placed.first &&
                    symbolInstance.getVerticalTextCollisionFeature()) {
                    return placeFeatureForVariableAnchors(*symbolInstance.getVerticalTextCollisionFeature(),
                                                          style::TextWritingModeType::Vertical,
                                                          symbolInstance.getVerticalTextCollisionFeature()
                                                              ? *symbolInstance.getVerticalTextCollisionFeature()
                                                              : symbolInstance.getIconCollisionFeature());
                }
                return std::pair<bool, bool>{false, false};
            };

            placeTextForPlacementModes(placeHorizontal, placeVertical);

            placeText = placed.first;
            offscreen &= placed.second;

            updatePreviousOrientationIfNotPlaced(placed.first);

            // If we didn't get placed, we still need to copy our position from
            // the last placement for fade animations
            if (!placeText && getPrevPlacement()) {
                auto prevOffset = getPrevPlacement()->variableOffsets.find(symbolInstance.getCrossTileID());
                if (prevOffset != getPrevPlacement()->variableOffsets.end()) {
                    variableOffsets[symbolInstance.getCrossTileID()] = prevOffset->second;
                }
            }
        }
    }

    if (symbolInstance.getPlacedIconIndex()) {
        if (!ctx.hasIconTextFit || !placeText || variableTextAnchors.empty()) {
            shift = {0.0f, 0.0f};
        }

        const auto& iconBuffer = symbolInstance.hasSdfIcon() ? bucket.sdfIcon : bucket.icon;
        const PlacedSymbol& placedSymbol = iconBuffer.placedSymbols.at(*symbolInstance.getPlacedIconIndex());
        const float fontSize = evaluateSizeForFeature(ctx.partiallyEvaluatedIconSize, placedSymbol);
        const auto& placeIconFeature = [&](const CollisionFeature& collisionFeature) {
            return collisionIndex.placeFeature(collisionFeature,
                                               shift,
                                               posMatrix,
                                               ctx.iconLabelPlaneMatrix,
                                               ctx.pixelRatio,
                                               placedSymbol,
                                               ctx.scale,
                                               fontSize,
                                               ctx.iconAllowOverlap,
                                               ctx.pitchTextWithMap,
                                               showCollisionBoxes,
                                               ctx.avoidEdges,
                                               collisionGroup.second,
                                               iconBoxes);
        };

        std::pair<bool, bool> placedIcon;
        if (placedVerticalText.first && symbolInstance.getVerticalIconCollisionFeature()) {
            placedIcon = placedVerticalIcon = placeIconFeature(*symbolInstance.getVerticalIconCollisionFeature());
        } else {
            placedIcon = placeIconFeature(symbolInstance.getIconCollisionFeature());
        }
        placeIcon = placedIcon.first;
        offscreen &= placedIcon.second;
    }

    const bool iconWithoutText = !symbolInstance.hasText() || ctx.getLayout().get<TextOptional>();
    const bool textWithoutIcon = !symbolInstance.hasIcon() || ctx.getLayout().get<IconOptional>();

    // combine placements for icon and text
    if (!iconWithoutText && !textWithoutIcon) {
        placeText = placeIcon = placeText && placeIcon;
    } else if (!textWithoutIcon) {
        placeText = placeText && placeIcon;
    } else if (!iconWithoutText) {
        placeIcon = placeText && placeIcon;
    }

    if (placeText) {
        if (placedVerticalText.first && symbolInstance.getVerticalTextCollisionFeature()) {
            collisionIndex.insertFeature(*symbolInstance.getVerticalTextCollisionFeature(),
                                         textBoxes,
                                         ctx.getLayout().get<TextIgnorePlacement>(),
                                         bucket.bucketInstanceId,
                                         collisionGroup.first);
        } else {
            collisionIndex.insertFeature(symbolInstance.getTextCollisionFeature(),
                                         textBoxes,
                                         ctx.getLayout().get<TextIgnorePlacement>(),
                                         bucket.bucketInstanceId,
                                         collisionGroup.first);
        }
    }

    if (placeIcon) {
        if (placedVerticalIcon.first && symbolInstance.getVerticalIconCollisionFeature()) {
            collisionIndex.insertFeature(*symbolInstance.getVerticalIconCollisionFeature(),
                                         iconBoxes,
                                         ctx.getLayout().get<IconIgnorePlacement>(),
                                         bucket.bucketInstanceId,
                                         collisionGroup.first);
        } else {
            collisionIndex.insertFeature(symbolInstance.getIconCollisionFeature(),
                                         iconBoxes,
                                         ctx.getLayout().get<IconIgnorePlacement>(),
                                         bucket.bucketInstanceId,
                                         collisionGroup.first);
        }
    }

    const bool hasIconCollisionCircleData = bucket.hasIconCollisionCircleData();
    const bool hasTextCollisionCircleData = bucket.hasTextCollisionCircleData();

    if (hasIconCollisionCircleData && symbolInstance.getIconCollisionFeature().alongLine && !iconBoxes.empty()) {
        collisionCircles[&symbolInstance.getIconCollisionFeature()] = iconBoxes;
    }
    if (hasTextCollisionCircleData && symbolInstance.getTextCollisionFeature().alongLine && !textBoxes.empty()) {
        collisionCircles[&symbolInstance.getTextCollisionFeature()] = textBoxes;
    }

    if (!symbolInstance.check(SYM_GUARD_LOC)) {
        return kUnplaced;
    }

    if (symbolInstance.getCrossTileID() != 0) {
        const auto hit = placements.find(symbolInstance.getCrossTileID());
        if (hit != placements.end()) {
            // If there's a previous placement with this ID, it comes from a tile that's fading out
            // Erase it so that the placement result from the non-fading tile supersedes it
            placements.erase(hit);
        }
    } else {
        assert(false);
        // We skipped some setup, don't use this one or we might run into inconsistencies
        symbolInstance.forceFail();
        return kUnplaced;
    }

    JointPlacement result(
        placeText || ctx.alwaysShowText, placeIcon || ctx.alwaysShowIcon, offscreen || bucket.justReloaded);
    placements.emplace(symbolInstance.getCrossTileID(), result);
    newSymbolPlaced(symbolInstance, ctx, result, ctx.placementType, textBoxes, iconBoxes);
    return result;
}

namespace {

SymbolInstanceReferences getBucketSymbols(const SymbolBucket& bucket,
                                          const std::optional<SortKeyRange>& sortKeyRange,
                                          double bearing) {
    if (bucket.layout->get<style::SymbolZOrder>() == style::SymbolZOrderType::ViewportY) {
        auto sortedSymbols = bucket.getSortedSymbols(static_cast<float>(bearing));
        // Place in the reverse order than draw i.e., starting from the foreground elements.
        std::reverse(std::begin(sortedSymbols), std::end(sortedSymbols));
        return sortedSymbols;
    }
    return bucket.getSymbols(sortKeyRange);
}

} // namespace

SymbolInstanceReferences Placement::getSortedSymbols(const BucketPlacementData& params, float) {
    const auto& bucket = static_cast<const SymbolBucket&>(params.bucket.get());
    SymbolInstanceReferences sortedSymbols = getBucketSymbols(
        bucket, params.sortKeyRange, collisionIndex.getTransformState().getBearing());
    auto* previousPlacement = getPrevPlacement();
    if (previousPlacement && isTiltedView()) {
        std::stable_sort(sortedSymbols.begin(),
                         sortedSymbols.end(),
                         [previousPlacement](const SymbolInstance& a, const SymbolInstance& b) noexcept {
                             auto* aPlacement = previousPlacement->getSymbolPlacement(a);
                             auto* bPlacement = previousPlacement->getSymbolPlacement(b);
                             if (!aPlacement) {
                                 // a < b, if 'a' is new and if 'b' was previously hidden.
                                 return bPlacement && !bPlacement->placed();
                             }
                             if (!bPlacement) {
                                 // a < b, if 'b' is new and 'a' was previously shown.
                                 return aPlacement->placed();
                             }
                             // a < b, if 'a' was shown and 'b' was hidden.
                             return aPlacement->placed() && !bPlacement->placed();
                         });
    }
    return sortedSymbols;
}

void Placement::commit() {
    bool placementChanged = false;
    if (!getPrevPlacement()) {
        assert(false);
        return;
    }
    prevZoomAdjustment = getPrevPlacement()->zoomAdjustment(placementZoom);
    const float increment = getPrevPlacement()->symbolFadeChange(commitTime);

    // add the opacities from the current placement, and copy their current
    // values from the previous placement
    for (auto& jointPlacement : placements) {
        auto prevOpacity = getPrevPlacement()->opacities.find(jointPlacement.first);
        if (prevOpacity != getPrevPlacement()->opacities.end()) {
            opacities.emplace(
                jointPlacement.first,
                JointOpacityState(
                    prevOpacity->second, increment, jointPlacement.second.text, jointPlacement.second.icon));
            placementChanged = placementChanged || jointPlacement.second.icon != prevOpacity->second.icon.placed ||
                               jointPlacement.second.text != prevOpacity->second.text.placed;
        } else {
            opacities.emplace(
                jointPlacement.first,
                JointOpacityState(
                    jointPlacement.second.text, jointPlacement.second.icon, jointPlacement.second.skipFade));
            placementChanged = placementChanged || jointPlacement.second.icon || jointPlacement.second.text;
        }
    }

    // copy and update values from the previous placement that aren't in the
    // current placement but haven't finished fading
    for (auto& prevOpacity : getPrevPlacement()->opacities) {
        if (!opacities.contains(prevOpacity.first)) {
            JointOpacityState jointOpacity(prevOpacity.second, increment, false, false);
            if (!jointOpacity.isHidden()) {
                opacities.emplace(prevOpacity.first, jointOpacity);
                placementChanged = placementChanged || prevOpacity.second.icon.placed || prevOpacity.second.text.placed;
            }
        }
    }

    for (auto& prevOffset : getPrevPlacement()->variableOffsets) {
        const uint32_t crossTileID = prevOffset.first;
        auto foundOffset = variableOffsets.find(crossTileID);
        auto foundOpacity = opacities.find(crossTileID);
        if (foundOffset == variableOffsets.end() && foundOpacity != opacities.end() &&
            !foundOpacity->second.isHidden()) {
            variableOffsets[prevOffset.first] = prevOffset.second;
        }
    }

    for (auto& prevOrientation : getPrevPlacement()->placedOrientations) {
        const uint32_t crossTileID = prevOrientation.first;
        auto foundOrientation = placedOrientations.find(crossTileID);
        auto foundOpacity = opacities.find(crossTileID);
        if (foundOrientation == placedOrientations.end() && foundOpacity != opacities.end() &&
            !foundOpacity->second.isHidden()) {
            placedOrientations[prevOrientation.first] = prevOrientation.second;
        }
    }

    fadeStartTime = placementChanged ? commitTime
                                     : (getPrevPlacement() ? getPrevPlacement()->fadeStartTime : TimePoint{});
}

void Placement::updateLayerBuckets(const RenderLayer& layer, const TransformState& state, bool updateOpacities) const {
    std::set<uint32_t> seenCrossTileIDs;
    for (const auto& item : layer.getPlacementData()) {
        if (!item.sortKeyRange || item.sortKeyRange->isFirstRange()) {
            item.bucket.get().updateVertices(*this, updateOpacities, state, item.tile, seenCrossTileIDs);
        }
    }
}

namespace {
Point<float> calculateVariableRenderShift(style::SymbolAnchorType anchor,
                                          float width,
                                          float height,
                                          std::array<float, 2> textOffset,
                                          float textBoxScale,
                                          float renderTextSize) {
    const AnchorAlignment alignment = AnchorAlignment::getAnchorAlignment(anchor);
    const float shiftX = -(alignment.horizontalAlign - 0.5f) * width;
    const float shiftY = -(alignment.verticalAlign - 0.5f) * height;
    return {(shiftX / textBoxScale + textOffset[0]) * renderTextSize,
            (shiftY / textBoxScale + textOffset[1]) * renderTextSize};
}
} // namespace

bool Placement::updateBucketDynamicVertices(SymbolBucket& bucket,
                                            const TransformState& state,
                                            const RenderTile& tile) const {
    using namespace style;
    const auto& layout = *bucket.layout;
    const bool alongLine = layout.get<SymbolPlacement>() != SymbolPlacementType::Point;
    const bool hasVariableAnchors = bucket.hasVariableTextAnchors() && bucket.hasTextData();
    const bool updateTextFitIcon = layout.get<IconTextFit>() != IconTextFitType::None &&
                                   (bucket.allowVerticalPlacement || hasVariableAnchors) &&
                                   (bucket.hasIconData() || bucket.hasSdfIconData());
    bool result = false;

    if (alongLine) {
        if (layout.get<IconRotationAlignment>() == AlignmentType::Map) {
            const bool pitchWithMap = layout.get<style::IconPitchAlignment>() == style::AlignmentType::Map;
            const bool keepUpright = layout.get<style::IconKeepUpright>();
            if (bucket.hasSdfIconData()) {
                reprojectLineLabels(bucket.sdfIcon.dynamicVertices(),
                                    bucket.sdfIcon.placedSymbols,
                                    tile.matrix,
                                    pitchWithMap,
                                    true /*rotateWithMap*/,
                                    keepUpright,
                                    tile,
                                    *bucket.iconSizeBinder,
                                    state);
                result = true;
            }
            if (bucket.hasIconData()) {
                reprojectLineLabels(bucket.icon.dynamicVertices(),
                                    bucket.icon.placedSymbols,
                                    tile.matrix,
                                    pitchWithMap,
                                    true /*rotateWithMap*/,
                                    keepUpright,
                                    tile,
                                    *bucket.iconSizeBinder,
                                    state);
                result = true;
            }
        }

        if (bucket.hasTextData() && layout.get<TextRotationAlignment>() == AlignmentType::Map) {
            const bool pitchWithMap = layout.get<style::TextPitchAlignment>() == style::AlignmentType::Map;
            const bool keepUpright = layout.get<style::TextKeepUpright>();
            reprojectLineLabels(bucket.text.dynamicVertices(),
                                bucket.text.placedSymbols,
                                tile.matrix,
                                pitchWithMap,
                                true /*rotateWithMap*/,
                                keepUpright,
                                tile,
                                *bucket.textSizeBinder,
                                state);
            result = true;
        }
    } else if (hasVariableAnchors) {
        bucket.text.sharedDynamicVertices->clear();
        bucket.hasVariablePlacement = false;

        const auto partiallyEvaluatedSize = bucket.textSizeBinder->evaluateForZoom(static_cast<float>(state.getZoom()));
        const auto tileScale = static_cast<float>(
            std::pow(2, state.getZoom() - tile.getOverscaledTileID().overscaledZ));
        const bool rotateWithMap = layout.get<TextRotationAlignment>() == AlignmentType::Map;
        const bool pitchWithMap = layout.get<TextPitchAlignment>() == AlignmentType::Map;
        const float pixelsToTileUnits = tile.id.pixelsToTileUnits(1.0f, static_cast<float>(state.getZoom()));
        const auto labelPlaneMatrix = getLabelPlaneMatrix(
            tile.matrix, pitchWithMap, rotateWithMap, state, pixelsToTileUnits);
        std::unordered_map<std::size_t, std::pair<std::size_t, Point<float>>> placedTextShifts;

        for (std::size_t i = 0; i < bucket.text.placedSymbols.size(); ++i) {
            const PlacedSymbol& symbol = bucket.text.placedSymbols[i];
            std::optional<VariableOffset> variableOffset;
            const bool skipOrientation = bucket.allowVerticalPlacement && !symbol.placedOrientation;
            if (!symbol.hidden && symbol.crossTileID != 0u && !skipOrientation) {
                auto it = variableOffsets.find(symbol.crossTileID);
                if (it != variableOffsets.end()) {
                    bucket.hasVariablePlacement = true;
                    variableOffset = it->second;
                }
            }

            if (!variableOffset) {
                // These symbols are from a justification that is not being
                // used, or a label that wasn't placed so we don't need to do
                // the extra math to figure out what incremental shift to apply.
                hideGlyphs(symbol.glyphOffsets.size(), bucket.text.dynamicVertices());
            } else {
                const Point<float> tileAnchor = symbol.anchorPoint;
                const auto projectedAnchor = project(tileAnchor, pitchWithMap ? tile.matrix : labelPlaneMatrix);
                const float perspectiveRatio = 0.5f +
                                               0.5f * (state.getCameraToCenterDistance() / projectedAnchor.second);
                float renderTextSize = evaluateSizeForFeature(partiallyEvaluatedSize, symbol) * perspectiveRatio /
                                       util::ONE_EM;
                if (pitchWithMap) {
                    // Go from size in pixels to equivalent size in tile units
                    renderTextSize *= bucket.tilePixelRatio / tileScale;
                }

                auto shift = calculateVariableRenderShift((*variableOffset).anchor,
                                                          (*variableOffset).width,
                                                          (*variableOffset).height,
                                                          (*variableOffset).offset,
                                                          (*variableOffset).textBoxScale,
                                                          renderTextSize);

                // Usual case is that we take the projected anchor and add the
                // pixel-based shift calculated above. In the (somewhat weird)
                // case of pitch-aligned text, we add an equivalent tile-unit
                // based shift to the anchor before projecting to the label
                // plane.
                Point<float> shiftedAnchor;
                if (pitchWithMap) {
                    shiftedAnchor =
                        project(Point<float>(tileAnchor.x + shift.x, tileAnchor.y + shift.y), labelPlaneMatrix).first;
                } else if (rotateWithMap) {
                    auto rotated = util::rotate(shift, -state.getPitch());
                    shiftedAnchor = Point<float>(projectedAnchor.first.x + rotated.x,
                                                 projectedAnchor.first.y + rotated.y);
                } else {
                    shiftedAnchor = Point<float>(projectedAnchor.first.x + shift.x, projectedAnchor.first.y + shift.y);
                }

                if (updateTextFitIcon && symbol.placedIconIndex) {
                    placedTextShifts.emplace(*symbol.placedIconIndex,
                                             std::pair<std::size_t, Point<float>>{i, shiftedAnchor});
                }

                for (std::size_t j = 0; j < symbol.glyphOffsets.size(); ++j) {
                    addDynamicAttributes(shiftedAnchor, symbol.angle, bucket.text.dynamicVertices());
                }
            }
        }

        if (updateTextFitIcon && bucket.hasVariablePlacement) {
            auto updateIcon = [&](SymbolBucket::Buffer& iconBuffer) {
                iconBuffer.sharedDynamicVertices->clear();
                for (std::size_t i = 0; i < iconBuffer.placedSymbols.size(); ++i) {
                    const PlacedSymbol& placedIcon = iconBuffer.placedSymbols[i];
                    if (placedIcon.hidden || (!placedIcon.placedOrientation && bucket.allowVerticalPlacement)) {
                        hideGlyphs(placedIcon.glyphOffsets.size(), iconBuffer.dynamicVertices());
                    } else {
                        const auto& pair = placedTextShifts.find(i);
                        if (pair == placedTextShifts.end()) {
                            hideGlyphs(placedIcon.glyphOffsets.size(), iconBuffer.dynamicVertices());
                        } else {
                            for (std::size_t j = 0; j < placedIcon.glyphOffsets.size(); ++j) {
                                addDynamicAttributes(
                                    pair->second.second, placedIcon.angle, iconBuffer.dynamicVertices());
                            }
                        }
                    }
                }
            };
            updateIcon(bucket.icon);
            updateIcon(bucket.sdfIcon);
        }

        result = true;
    } else if (bucket.allowVerticalPlacement && bucket.hasTextData()) {
        const auto updateDynamicVertices = [](SymbolBucket::Buffer& buffer) {
            buffer.sharedDynamicVertices->clear();
            for (const PlacedSymbol& symbol : buffer.placedSymbols) {
                if (symbol.hidden || !symbol.placedOrientation) {
                    hideGlyphs(symbol.glyphOffsets.size(), buffer.dynamicVertices());
                } else {
                    for (std::size_t j = 0; j < symbol.glyphOffsets.size(); ++j) {
                        addDynamicAttributes(symbol.anchorPoint, symbol.angle, buffer.dynamicVertices());
                    }
                }
            }
        };

        updateDynamicVertices(bucket.text);
        // When text box is rotated, icon-text-fit icon must be rotated as well.
        if (updateTextFitIcon) {
            updateDynamicVertices(bucket.icon);
            updateDynamicVertices(bucket.sdfIcon);
        }

        result = true;
    }

    return result;
}

void Placement::updateBucketOpacities(SymbolBucket& bucket,
                                      const TransformState& state,
                                      std::set<uint32_t>& seenCrossTileIDs) const {
    if (bucket.hasTextData()) bucket.text.sharedOpacityVertices->clear();
    if (bucket.hasIconData()) bucket.icon.sharedOpacityVertices->clear();
    if (bucket.hasSdfIconData()) bucket.sdfIcon.sharedOpacityVertices->clear();
    if (bucket.hasIconCollisionBoxData()) bucket.iconCollisionBox->dynamicVertices().clear();
    if (bucket.hasIconCollisionCircleData()) bucket.iconCollisionCircle->dynamicVertices().clear();
    if (bucket.hasTextCollisionBoxData()) bucket.textCollisionBox->dynamicVertices().clear();
    if (bucket.hasTextCollisionCircleData()) bucket.textCollisionCircle->dynamicVertices().clear();

    const JointOpacityState duplicateOpacityState(false, false, true);

    const bool textAllowOverlap = bucket.layout->get<style::TextAllowOverlap>();
    const bool iconAllowOverlap = bucket.layout->get<style::IconAllowOverlap>();
    const bool variablePlacement = bucket.hasVariableTextAnchors();
    const bool rotateWithMap = bucket.layout->get<style::TextRotationAlignment>() == style::AlignmentType::Map;
    const bool pitchWithMap = bucket.layout->get<style::TextPitchAlignment>() == style::AlignmentType::Map;
    const bool hasIconTextFit = bucket.layout->get<style::IconTextFit>() != style::IconTextFitType::None;
    const bool screenSpace = bucket.layout->get<style::SymbolScreenSpace>();

    // If allow-overlap is true, we can show symbols before placement runs on them
    // But we have to wait for placement if we potentially depend on a paired icon/text
    // with allow-overlap: false.
    // See https://github.com/mapbox/mapbox-gl-native/issues/12483
    // Prevent a flickering issue when showing a symbol allowing overlap.
    const JointOpacityState defaultOpacityState(
        (screenSpace || bucket.justReloaded) && textAllowOverlap &&
            (iconAllowOverlap || !(bucket.hasIconData() || bucket.hasSdfIconData()) ||
             bucket.layout->get<style::IconOptional>()),
        (screenSpace || bucket.justReloaded) && iconAllowOverlap &&
            (textAllowOverlap || !bucket.hasTextData() || bucket.layout->get<style::TextOptional>()),
        true);

    for (SymbolInstance& symbolInstance : bucket.symbolInstances) {
        if (!symbolInstance.check(SYM_GUARD_LOC)) continue;
        bool isDuplicate = seenCrossTileIDs.contains(symbolInstance.getCrossTileID());

        auto it = opacities.find(symbolInstance.getCrossTileID());
        auto opacityState = defaultOpacityState;
        if (isDuplicate) {
            opacityState = duplicateOpacityState;
        } else if (it != opacities.end()) {
            opacityState = it->second;
        }

        seenCrossTileIDs.insert(symbolInstance.getCrossTileID());

        if (symbolInstance.hasText() || symbolInstance.hasIcon()) {
            if (!symbolInstance.checkIndexes(bucket.text.placedSymbols.size(),
                                             bucket.icon.placedSymbols.size(),
                                             bucket.sdfIcon.placedSymbols.size(),
                                             SYM_GUARD_LOC))
                return;
        }
        if (symbolInstance.hasText()) {
            size_t textOpacityVerticesSize = 0u;
            const auto& opacityVertex = SymbolBucket::opacityVertex(opacityState.text.placed,
                                                                    opacityState.text.opacity);
            if (symbolInstance.getPlacedRightTextIndex()) {
                textOpacityVerticesSize += symbolInstance.getRightJustifiedGlyphQuadsSize() * 4;
                PlacedSymbol& placed = bucket.text.placedSymbols[*symbolInstance.getPlacedRightTextIndex()];
                placed.hidden = opacityState.isHidden();
            }
            if (symbolInstance.getPlacedCenterTextIndex() && !symbolInstance.getSingleLine()) {
                textOpacityVerticesSize += symbolInstance.getCenterJustifiedGlyphQuadsSize() * 4;
                PlacedSymbol& placed = bucket.text.placedSymbols[*symbolInstance.getPlacedCenterTextIndex()];
                placed.hidden = opacityState.isHidden();
            }
            if (symbolInstance.getPlacedLeftTextIndex() && !symbolInstance.getSingleLine()) {
                textOpacityVerticesSize += symbolInstance.getLeftJustifiedGlyphQuadsSize() * 4;
                PlacedSymbol& placed = bucket.text.placedSymbols[*symbolInstance.getPlacedLeftTextIndex()];
                placed.hidden = opacityState.isHidden();
            }
            if (symbolInstance.getPlacedVerticalTextIndex()) {
                textOpacityVerticesSize += symbolInstance.getVerticalGlyphQuadsSize() * 4;
                bucket.text.placedSymbols[*symbolInstance.getPlacedVerticalTextIndex()].hidden =
                    opacityState.isHidden();
            }

            bucket.text.opacityVertices().extend(textOpacityVerticesSize, opacityVertex);

            style::TextWritingModeType previousOrientation = style::TextWritingModeType::Horizontal;
            if (bucket.allowVerticalPlacement) {
                auto prevOrientation = placedOrientations.find(symbolInstance.getCrossTileID());
                if (prevOrientation != placedOrientations.end()) {
                    previousOrientation = prevOrientation->second;
                    markUsedOrientation(bucket, prevOrientation->second, symbolInstance);
                }
            }

            auto prevOffset = variableOffsets.find(symbolInstance.getCrossTileID());
            if (prevOffset != variableOffsets.end()) {
                markUsedJustification(bucket, prevOffset->second.anchor, symbolInstance, previousOrientation);
            }
        }
        if (symbolInstance.hasIcon()) {
            size_t iconOpacityVerticesSize = 0u;
            const auto& opacityVertex = SymbolBucket::opacityVertex(opacityState.icon.placed,
                                                                    opacityState.icon.opacity);
            auto& iconBuffer = symbolInstance.hasSdfIcon() ? bucket.sdfIcon : bucket.icon;

            if (symbolInstance.getPlacedIconIndex()) {
                iconOpacityVerticesSize += symbolInstance.getIconQuadsSize() * 4;
                iconBuffer.placedSymbols[*symbolInstance.getPlacedIconIndex()].hidden = opacityState.isHidden();
            }

            if (symbolInstance.getPlacedVerticalIconIndex()) {
                iconOpacityVerticesSize += symbolInstance.getIconQuadsSize() * 4;
                iconBuffer.placedSymbols[*symbolInstance.getPlacedVerticalIconIndex()].hidden = opacityState.isHidden();
            }

            iconBuffer.opacityVertices().extend(iconOpacityVerticesSize, opacityVertex);
        }

        auto updateIconCollisionBox = [&](const auto& feature, const bool placed, const Point<float>& shift) {
            if (feature.alongLine) {
                return;
            }
            const auto& dynamicVertex = SymbolBucket::collisionDynamicVertex(placed, false, shift);
            bucket.iconCollisionBox->dynamicVertices().extend(feature.boxes.size() * 4, dynamicVertex);
        };

        auto updateTextCollisionBox =
            [this, &bucket, &symbolInstance, &state, variablePlacement, rotateWithMap, pitchWithMap](
                const auto& feature, const bool placed) {
                Point<float> shift{0.0f, 0.0f};
                if (feature.alongLine) {
                    return shift;
                }
                bool used = true;
                if (variablePlacement) {
                    auto foundOffset = variableOffsets.find(symbolInstance.getCrossTileID());
                    if (foundOffset != variableOffsets.end()) {
                        const VariableOffset& variableOffset = foundOffset->second;
                        // This will show either the currently placed position or
                        // the last successfully placed position (so you can
                        // visualize what collision just made the symbol disappear,
                        // and the most likely place for the symbol to come back)
                        shift = calculateVariableLayoutOffset(variableOffset.anchor,
                                                              variableOffset.width,
                                                              variableOffset.height,
                                                              variableOffset.offset,
                                                              variableOffset.textBoxScale,
                                                              rotateWithMap,
                                                              pitchWithMap,
                                                              static_cast<float>(state.getBearing()));
                    } else {
                        // No offset -> this symbol hasn't been placed since coming
                        // on-screen No single box is particularly meaningful and
                        // all of them would be too noisy Use the center box just to
                        // show something's there, but mark it "not used"
                        used = false;
                    }
                }
                const auto& dynamicVertex = SymbolBucket::collisionDynamicVertex(placed, !used, shift);
                bucket.textCollisionBox->dynamicVertices().extend(feature.boxes.size() * 4, dynamicVertex);
                return shift;
            };

        auto updateCollisionCircles = [&](const auto& feature, const bool placed, bool isText) {
            if (!feature.alongLine) {
                return;
            }
            auto circles = collisionCircles.find(&feature);
            if (circles != collisionCircles.end()) {
                for (const auto& circle : circles->second) {
                    const auto& dynamicVertex = SymbolBucket::collisionDynamicVertex(placed, !circle.isCircle(), {});
                    isText ? bucket.textCollisionCircle->dynamicVertices().extend(4, dynamicVertex)
                           : bucket.iconCollisionCircle->dynamicVertices().extend(4, dynamicVertex);
                }
            } else {
                // This feature was not placed, because it was not loaded or
                // from a fading tile. Apply default values.
                static const auto dynamicVertex = SymbolBucket::collisionDynamicVertex(placed, false /*not used*/, {});
                isText ? bucket.textCollisionCircle->dynamicVertices().extend(4 * feature.boxes.size(), dynamicVertex)
                       : bucket.iconCollisionCircle->dynamicVertices().extend(4 * feature.boxes.size(), dynamicVertex);
            }
        };
        Point<float> textShift{0.0f, 0.0f};
        Point<float> verticalTextShift{0.0f, 0.0f};
        if (bucket.hasTextCollisionBoxData()) {
            textShift = updateTextCollisionBox(symbolInstance.getTextCollisionFeature(), opacityState.text.placed);
            if (bucket.allowVerticalPlacement && symbolInstance.getVerticalTextCollisionFeature()) {
                verticalTextShift = updateTextCollisionBox(*symbolInstance.getVerticalTextCollisionFeature(),
                                                           opacityState.text.placed);
            }
        }
        if (bucket.hasIconCollisionBoxData()) {
            updateIconCollisionBox(symbolInstance.getIconCollisionFeature(),
                                   opacityState.icon.placed,
                                   hasIconTextFit ? textShift : Point<float>{0.0f, 0.0f});
            if (bucket.allowVerticalPlacement && symbolInstance.getVerticalIconCollisionFeature()) {
                updateIconCollisionBox(*symbolInstance.getVerticalIconCollisionFeature(),
                                       opacityState.text.placed,
                                       hasIconTextFit ? verticalTextShift : Point<float>{0.0f, 0.0f});
            }
        }

        if (bucket.hasIconCollisionCircleData()) {
            updateCollisionCircles(symbolInstance.getIconCollisionFeature(), opacityState.icon.placed, false);
        }
        if (bucket.hasTextCollisionCircleData()) {
            updateCollisionCircles(symbolInstance.getTextCollisionFeature(), opacityState.text.placed, true);
        }
    }

    bucket.sortFeatures(static_cast<float>(state.getBearing()));
    static_cast<Bucket&>(bucket).check(SYM_GUARD_LOC);

    const auto retainedData = retainedQueryData.find(bucket.bucketInstanceId);
    if (retainedData != retainedQueryData.end()) {
        retainedData->second.featureSortOrder = bucket.featureSortOrder;
    }
}

namespace {
std::optional<size_t> justificationToIndex(style::TextJustifyType justify,
                                           const SymbolInstance& symbolInstance,
                                           style::TextWritingModeType orientation) {
    // Vertical symbol has just one justification, style::TextJustifyType::Left.
    if (orientation == style::TextWritingModeType::Vertical) {
        return symbolInstance.getPlacedVerticalTextIndex();
    }

    switch (justify) {
        case style::TextJustifyType::Right:
            return symbolInstance.getPlacedRightTextIndex();
        case style::TextJustifyType::Center:
            return symbolInstance.getPlacedCenterTextIndex();
        case style::TextJustifyType::Left:
            return symbolInstance.getPlacedLeftTextIndex();
        case style::TextJustifyType::Auto:
            break;
    }
    assert(false);
    return std::nullopt;
}

const style::TextJustifyType justifyTypes[] = {
    style::TextJustifyType::Right, style::TextJustifyType::Center, style::TextJustifyType::Left};

} // namespace

void Placement::markUsedJustification(SymbolBucket& bucket,
                                      style::TextVariableAnchorType placedAnchor,
                                      const SymbolInstance& symbolInstance,
                                      style::TextWritingModeType orientation) const {
    style::TextJustifyType anchorJustify = getAnchorJustification(placedAnchor);
    assert(anchorJustify != style::TextJustifyType::Auto);
    const std::optional<size_t>& autoIndex = justificationToIndex(anchorJustify, symbolInstance, orientation);

    for (auto& justify : justifyTypes) {
        const std::optional<size_t> index = justificationToIndex(justify, symbolInstance, orientation);
        if (index) {
            assert(bucket.text.placedSymbols.size() > *index);
            if (!symbolInstance.checkIndex(index, bucket.text.placedSymbols.size(), SYM_GUARD_LOC)) continue;
            if (autoIndex && *index != *autoIndex) {
                // There are multiple justifications and this one isn't it: shift offscreen
                bucket.text.placedSymbols.at(*index).crossTileID = 0u;
            } else {
                // Either this is the chosen justification or the justification is hardwired: use this one
                bucket.text.placedSymbols.at(*index).crossTileID = symbolInstance.getCrossTileID();
            }
        }
    }
}

void Placement::markUsedOrientation(SymbolBucket& bucket,
                                    style::TextWritingModeType orientation,
                                    const SymbolInstance& symbolInstance) const {
    const auto horizontal = orientation == style::TextWritingModeType::Horizontal
                                ? std::optional<style::TextWritingModeType>(orientation)
                                : std::nullopt;
    const auto vertical = orientation == style::TextWritingModeType::Vertical
                              ? std::optional<style::TextWritingModeType>(orientation)
                              : std::nullopt;

    if (!symbolInstance.checkIndexes(bucket.text.placedSymbols.size(),
                                     bucket.icon.placedSymbols.size(),
                                     bucket.sdfIcon.placedSymbols.size(),
                                     SYM_GUARD_LOC)) {
        return;
    }

    if (symbolInstance.getPlacedRightTextIndex()) {
        bucket.text.placedSymbols.at(*symbolInstance.getPlacedRightTextIndex()).placedOrientation = horizontal;
    }

    if (symbolInstance.getPlacedCenterTextIndex() && !symbolInstance.getSingleLine()) {
        bucket.text.placedSymbols.at(*symbolInstance.getPlacedCenterTextIndex()).placedOrientation = horizontal;
    }

    if (symbolInstance.getPlacedLeftTextIndex() && !symbolInstance.getSingleLine()) {
        bucket.text.placedSymbols.at(*symbolInstance.getPlacedLeftTextIndex()).placedOrientation = horizontal;
    }

    if (symbolInstance.getPlacedVerticalTextIndex()) {
        bucket.text.placedSymbols.at(*symbolInstance.getPlacedVerticalTextIndex()).placedOrientation = vertical;
    }

    auto& iconBuffer = symbolInstance.hasSdfIcon() ? bucket.sdfIcon : bucket.icon;
    if (symbolInstance.getPlacedIconIndex()) {
        iconBuffer.placedSymbols.at(*symbolInstance.getPlacedIconIndex()).placedOrientation = horizontal;
    }

    if (symbolInstance.getPlacedVerticalIconIndex()) {
        iconBuffer.placedSymbols.at(*symbolInstance.getPlacedVerticalIconIndex()).placedOrientation = vertical;
    }
}

bool Placement::isTiltedView() const {
    return updateParameters->transformState.getPitch() != 0.0f;
}

float Placement::symbolFadeChange(TimePoint now) const {
    if (transitionsEnabled() &&
        transitionOptions.duration.value_or(util::DEFAULT_TRANSITION_DURATION) > Milliseconds(0)) {
        return std::chrono::duration<float>(now - commitTime) /
                   transitionOptions.duration.value_or(util::DEFAULT_TRANSITION_DURATION) +
               prevZoomAdjustment;
    }
    return 1.0;
}

float Placement::zoomAdjustment(const float zoom) const {
    // When zooming out labels can overlap each other quickly. This
    // adjustment is used to reduce the fade duration for symbols while zooming
    // out quickly. It is also used to reduce the interval between placement
    // calculations. Reducing the interval between placements means collisions
    // are discovered and eliminated sooner.
    return std::max(0.0f, (placementZoom - zoom) / 1.5f);
}

const JointPlacement* Placement::getSymbolPlacement(const SymbolInstance& symbol) const {
    assert(symbol.getCrossTileID() != 0);
    const auto found = placements.find(symbol.getCrossTileID());
    return (found != placements.end()) ? &found->second : nullptr;
}

Duration Placement::getUpdatePeriod(const float zoom) const {
    // Even if transitionOptions.duration is set to a value < 300ms, we still wait
    // for this default transition duration before attempting another placement operation.
    const auto fadeDuration = std::max(util::DEFAULT_TRANSITION_DURATION,
                                       transitionOptions.duration.value_or(util::DEFAULT_TRANSITION_DURATION));
    return std::chrono::duration_cast<Duration>(fadeDuration * (1.0 - zoomAdjustment(zoom)));
}

bool Placement::transitionsEnabled() const {
    return transitionOptions.enablePlacementTransitions;
}

bool Placement::hasTransitions(TimePoint now) const {
    assert(transitionsEnabled());
    return std::chrono::duration<float>(now - fadeStartTime) <
           transitionOptions.duration.value_or(util::DEFAULT_TRANSITION_DURATION);
}

const std::vector<PlacedSymbolData>& Placement::getPlacedSymbolsData() const {
    const static std::vector<PlacedSymbolData> data;
    return data;
}

const CollisionIndex& Placement::getCollisionIndex() const {
    return collisionIndex;
}

const RetainedQueryData& Placement::getQueryData(uint32_t bucketInstanceId) const {
    auto it = retainedQueryData.find(bucketInstanceId);
    if (it == retainedQueryData.end()) {
        throw std::runtime_error("Placement::getQueryData with unrecognized bucketInstanceId");
    }
    return it->second;
}

/// Placement for Static map mode.
class StaticPlacement : public Placement {
public:
    explicit StaticPlacement(std::shared_ptr<const UpdateParameters> updateParameters_)
        : Placement(std::move(updateParameters_), std::nullopt) {}

protected:
    void commit() override;
    float symbolFadeChange(TimePoint) const override { return 1.0f; }
    bool hasTransitions(TimePoint) const override { return false; }
    bool transitionsEnabled() const override { return false; }
};

void StaticPlacement::commit() {
    fadeStartTime = commitTime;
    for (auto& jointPlacement : placements) {
        opacities.emplace(
            jointPlacement.first,
            JointOpacityState(jointPlacement.second.text, jointPlacement.second.icon, jointPlacement.second.skipFade));
    }
}

/// Placement for Tile map mode.

struct Intersection {
    Intersection(const SymbolInstance& symbol_, PlacementContext ctx_, IntersectStatus status_, std::size_t priority_)
        : symbol(symbol_),
          ctx(std::move(ctx_)),
          status(status_),
          priority(priority_) {}
    std::reference_wrapper<const SymbolInstance> symbol;
    PlacementContext ctx;
    IntersectStatus status;
    std::size_t priority; // less means more important
};

class TilePlacement : public StaticPlacement {
public:
    explicit TilePlacement(std::shared_ptr<const UpdateParameters> updateParameters_)
        : StaticPlacement(std::move(updateParameters_)) {}

private:
    void placeLayers(const RenderLayerReferences&) override;
    void placeSymbolBucket(const BucketPlacementData&, std::set<uint32_t>&) override;
    void collectPlacedSymbolData(bool enable) override { collectData = enable; }
    const std::vector<PlacedSymbolData>& getPlacedSymbolsData() const override { return placedSymbolsData; }

    std::optional<CollisionBoundaries> getAvoidEdges(const SymbolBucket&, const mat4&) override;
    bool canPlaceAtVariableAnchor(const CollisionBox& box,
                                  TextVariableAnchorType anchor,
                                  Point<float> shift,
                                  std::vector<style::TextVariableAnchorType>& anchors,
                                  const mat4& posMatrix,
                                  float textPixelRatio) override;
    void newSymbolPlaced(const SymbolInstance&,
                         const PlacementContext&,
                         const JointPlacement&,
                         style::SymbolPlacementType,
                         const std::vector<ProjectedCollisionBox>&,
                         const std::vector<ProjectedCollisionBox>&) override;

    bool shouldRetryPlacement(const JointPlacement&, const PlacementContext&);

    std::unordered_map<uint32_t, bool> locationCache;
    std::optional<CollisionBoundaries> tileBorders;
    std::set<uint32_t> seenCrossTileIDs;
    std::vector<PlacedSymbolData> placedSymbolsData;
    std::vector<Intersection> intersections;
    bool populateIntersections = false;
    std::size_t currentIntersectionPriority{};
    bool collectData = false;
};

void TilePlacement::placeLayers(const RenderLayerReferences& layers) {
    placedSymbolsData.clear();
    seenCrossTileIDs.clear();
    intersections.clear();
    currentIntersectionPriority = 0u;
    // Populate intersections.
    populateIntersections = true;
    for (auto it = layers.crbegin(); it != layers.crend(); ++it) {
        placeLayer(*it, seenCrossTileIDs);
    }

    std::sort(intersections.begin(), intersections.end(), [](const Intersection& a, const Intersection& b) {
        if (a.priority != b.priority) return a.priority < b.priority;
        uint8_t flagsA = a.status.flags;
        uint8_t flagsB = b.status.flags;
        // Items arranged as: VerticalBorders & HorizontalBorders (3) ->
        // VerticalBorders (2) -> HorizontalBorders (1)
        if (flagsA != flagsB) return flagsA > flagsB;
        // If both intersects the same border(s), look for a more noticeable cut-off.
        if (a.status.minSectionLength != b.status.minSectionLength) {
            return a.status.minSectionLength > b.status.minSectionLength;
        }
        // Look at the anchor coordinates
        if (a.symbol.get().getAnchor().point.y != b.symbol.get().getAnchor().point.y) {
            return a.symbol.get().getAnchor().point.y < b.symbol.get().getAnchor().point.y;
        }
        if (a.symbol.get().getAnchor().point.x != b.symbol.get().getAnchor().point.x) {
            return a.symbol.get().getAnchor().point.x < b.symbol.get().getAnchor().point.x;
        }
        // Finally, looking at the key hashes.
        return std::hash<std::u16string>()(a.symbol.get().getKey()) <
               std::hash<std::u16string>()(b.symbol.get().getKey());
    });
    // Place intersections.
    for (const auto& intersection : intersections) {
        const SymbolInstance& symbol = intersection.symbol;
        const PlacementContext& ctx = intersection.ctx;
        currentIntersectionPriority = intersection.priority;
        if (seenCrossTileIDs.contains(symbol.getCrossTileID())) continue;
        JointPlacement placement = placeSymbol(symbol, ctx);
        if (shouldRetryPlacement(placement, ctx)) continue;
        seenCrossTileIDs.insert(symbol.getCrossTileID());
    }
    // Place the rest labels.
    populateIntersections = false;
    for (auto it = layers.crbegin(); it != layers.crend(); ++it) {
        placeLayer(*it, seenCrossTileIDs);
    }
    commit();
}

std::optional<CollisionBoundaries> TilePlacement::getAvoidEdges(const SymbolBucket& bucket, const mat4& posMatrix) {
    tileBorders = collisionIndex.projectTileBoundaries(posMatrix);
    const auto& layout = *bucket.layout;
    if (layout.get<style::SymbolAvoidEdges>() ||
        layout.get<style::SymbolPlacement>() == style::SymbolPlacementType::Line) {
        return tileBorders;
    }
    return std::nullopt;
}

void TilePlacement::placeSymbolBucket(const BucketPlacementData& params, std::set<uint32_t>& seen) {
    assert(updateParameters);
    const auto& bucket = static_cast<const SymbolBucket&>(params.bucket.get());
    const auto& layout = *bucket.layout;
    if (!populateIntersections) {
        Placement::placeSymbolBucket(params, seen);
        return;
    }
    if (layout.get<SymbolPlacement>() != SymbolPlacementType::Point || layout.get<SymbolAvoidEdges>()) {
        // Collect intersection only for point placement.
        return;
    }
    const RenderTile& renderTile = params.tile;
    PlacementContext ctx{bucket,
                         params.tile,
                         collisionIndex.getTransformState(),
                         placementZoom,
                         collisionGroups.get(params.sourceId),
                         getAvoidEdges(bucket, renderTile.matrix)};

    // In this case we first try to place symbols, which intersects the tile
    // borders, so that those symbols will remain even if each tile is handled
    // independently.
    SymbolInstanceReferences symbolInstances = getBucketSymbols(
        bucket, params.sortKeyRange, collisionIndex.getTransformState().getBearing());

    // Keeps the data necessary to find a feature location according to a tile.
    struct NeighborTileData {
        NeighborTileData(const CollisionIndex& collisionIndex, UnwrappedTileID id_, Point<float> shift_)
            : id(id_),
              shift(shift_),
              matrix() {
            collisionIndex.getTransformState().matrixFor(matrix, id);
            matrix::multiply(matrix, collisionIndex.getTransformState().getProjectionMatrix(), matrix);
            borders = collisionIndex.projectTileBoundaries(matrix);
        }

        UnwrappedTileID id;
        Point<float> shift;
        mat4 matrix;
        CollisionBoundaries borders;
    };

    uint8_t z = renderTile.id.canonical.z;
    uint32_t x = renderTile.id.canonical.x;
    uint32_t y = renderTile.id.canonical.y;
    const std::array<NeighborTileData, 4> neighbours{{
        {collisionIndex, UnwrappedTileID(z, x, y - 1), {0.0f, util::EXTENT}},  // top
        {collisionIndex, UnwrappedTileID(z, x, y + 1), {0.0f, -util::EXTENT}}, // bottom
        {collisionIndex, UnwrappedTileID(z, x - 1, y), {util::EXTENT, 0.0f}},  // left
        {collisionIndex, UnwrappedTileID(z, x + 1, y), {-util::EXTENT, 0.0f}}  // right
    }};

    auto collisionBoxIntersectsTileEdges = [&](const CollisionBox& collisionBox,
                                               Point<float> shift) noexcept -> IntersectStatus {
        IntersectStatus intersects = collisionIndex.intersectsTileEdges(
            collisionBox, shift, renderTile.matrix, ctx.pixelRatio, *tileBorders);
        // Check if this symbol intersects the neighbor tile borders. If so, it
        // also shall be placed with priority.
        for (const auto& neighbor : neighbours) {
            if (intersects.flags != IntersectStatus::None) break;
            intersects = collisionIndex.intersectsTileEdges(
                collisionBox, shift + neighbor.shift, neighbor.matrix, ctx.pixelRatio, neighbor.borders);
        }
        return intersects;
    };

    auto symbolIntersectsTileEdges = [&collisionBoxIntersectsTileEdges,
                                      pitchTextWithMap = ctx.pitchTextWithMap,
                                      rotateTextWithMap = ctx.rotateTextWithMap,
                                      variableIconPlacement = ctx.hasIconTextFit && !ctx.iconAllowOverlap,
                                      bearing = static_cast<float>(ctx.getTransformState().getBearing())](
                                         const SymbolInstance& symbol) noexcept -> IntersectStatus {
        IntersectStatus result;
        std::optional<style::TextVariableAnchorType> variableAnchor;
        auto textVariableAnchorOffset = symbol.getTextVariableAnchorOffset();
        if (textVariableAnchorOffset && !textVariableAnchorOffset->empty()) {
            variableAnchor = textVariableAnchorOffset->begin()->anchorType;
        }

        if (!symbol.getTextCollisionFeature().boxes.empty()) {
            const auto& textCollisionBox = symbol.getTextCollisionFeature().boxes.front();

            Point<float> offset{};
            if (variableAnchor) {
                float width = textCollisionBox.x2 - textCollisionBox.x1;
                float height = textCollisionBox.y2 - textCollisionBox.y1;

                auto variableTextOffset = textVariableAnchorOffset->getOffsetByAnchor(*variableAnchor);

                offset = calculateVariableLayoutOffset(*variableAnchor,
                                                       width,
                                                       height,
                                                       variableTextOffset,
                                                       symbol.getTextBoxScale(),
                                                       rotateTextWithMap,
                                                       pitchTextWithMap,
                                                       bearing);
            }
            result = collisionBoxIntersectsTileEdges(textCollisionBox, offset);
        }

        if (!symbol.getIconCollisionFeature().boxes.empty()) {
            const auto& iconCollisionBox = symbol.getIconCollisionFeature().boxes.front();
            Point<float> offset{};
            if (variableAnchor && variableIconPlacement) {
                float width = iconCollisionBox.x2 - iconCollisionBox.x1;
                float height = iconCollisionBox.y2 - iconCollisionBox.y1;

                auto variableTextOffset = textVariableAnchorOffset->getOffsetByAnchor(*variableAnchor);

                offset = calculateVariableLayoutOffset(*variableAnchor,
                                                       width,
                                                       height,
                                                       variableTextOffset,
                                                       symbol.getTextBoxScale(),
                                                       rotateTextWithMap,
                                                       pitchTextWithMap,
                                                       bearing);
            }
            auto iconIntersects = collisionBoxIntersectsTileEdges(iconCollisionBox, offset);
            result.flags |= iconIntersects.flags;
            result.minSectionLength = std::max(result.minSectionLength, iconIntersects.minSectionLength);
        }

        return result;
    };

    for (const SymbolInstance& symbol : symbolInstances) {
        if (!symbol.check(SYM_GUARD_LOC)) {
            continue;
        }
        const auto intersectStatus = symbolIntersectsTileEdges(symbol);
        if (intersectStatus.flags == IntersectStatus::None) {
            continue;
        }
        intersections.emplace_back(symbol, ctx, intersectStatus, currentIntersectionPriority);
    }

    ++currentIntersectionPriority;
}

bool TilePlacement::canPlaceAtVariableAnchor(const CollisionBox& box,
                                             TextVariableAnchorType anchor,
                                             Point<float> shift,
                                             std::vector<style::TextVariableAnchorType>& anchors,
                                             const mat4& posMatrix,
                                             float textPixelRatio) {
    assert(tileBorders);
    if (populateIntersections) {
        // A variable label is only allowed to intersect tile border with the first anchor.
        if (anchor == anchors.front()) {
            // Check, that the label would intersect the tile borders even
            // without shift, otherwise intersection is not allowed (preventing
            // cut-offs in case the shift is lager than the buffer size).
            auto status = collisionIndex.intersectsTileEdges(box, {}, posMatrix, textPixelRatio, *tileBorders);
            if (status.flags != IntersectStatus::None) return true;
        }
        // The most important labels shall be placed first anyway, so we continue trying
        // the following variable anchors for them; less priority labels
        // will wait for the second round (when `populateIntersections` is `false`).
        if (currentIntersectionPriority > 0u) return false;
    }
    // Can be placed, if it does not intersect tile borders.
    auto status = collisionIndex.intersectsTileEdges(box, shift, posMatrix, textPixelRatio, *tileBorders);
    return (status.flags == IntersectStatus::None);
}

void TilePlacement::newSymbolPlaced(const SymbolInstance& symbol,
                                    const PlacementContext& ctx,
                                    const JointPlacement& placement,
                                    style::SymbolPlacementType placementType,
                                    const std::vector<ProjectedCollisionBox>& textCollisionBoxes,
                                    const std::vector<ProjectedCollisionBox>& iconCollisionBoxes) {
    if (!collectData || placementType != style::SymbolPlacementType::Point || shouldRetryPlacement(placement, ctx))
        return;

    std::optional<mapbox::geometry::box<float>> textCollisionBox;
    if (!textCollisionBoxes.empty()) {
        assert(textCollisionBoxes.size() == 1u);
        auto& box = textCollisionBoxes.front();
        assert(box.isBox());
        textCollisionBox = box.box();
    }
    std::optional<mapbox::geometry::box<float>> iconCollisionBox;
    if (!iconCollisionBoxes.empty()) {
        assert(iconCollisionBoxes.size() == 1u);
        auto& box = iconCollisionBoxes.front();
        assert(box.isBox());
        iconCollisionBox = box.box();
    }
    PlacedSymbolData symbolData{.key = symbol.getKey(),
                                .textCollisionBox = textCollisionBox,
                                .iconCollisionBox = iconCollisionBox,
                                .textPlaced = placement.text,
                                .iconPlaced = placement.icon,
                                .intersectsTileBorder = !placement.skipFade && populateIntersections,
                                .viewportPadding = collisionIndex.getViewportPadding(),
                                .layer = ctx.getBucket().bucketLeaderID};
    placedSymbolsData.emplace_back(std::move(symbolData));
}

bool TilePlacement::shouldRetryPlacement(const JointPlacement& placement, const PlacementContext& ctx) {
    // We re-try the placement to try out remaining variable anchors.
    return populateIntersections && !placement.placed() && ctx.getBucket().hasVariableTextAnchors();
}

// static
Mutable<Placement> Placement::create(std::shared_ptr<const UpdateParameters> updateParameters_,
                                     std::optional<Immutable<Placement>> prevPlacement) {
    MLN_TRACE_FUNC();
    assert(updateParameters_);
    switch (updateParameters_->mode) {
        case MapMode::Continuous:
            assert(prevPlacement);
            return makeMutable<Placement>(std::move(updateParameters_), std::move(prevPlacement));
        case MapMode::Static:
            return staticMutableCast<Placement>(makeMutable<StaticPlacement>(std::move(updateParameters_)));
        case MapMode::Tile:
            return staticMutableCast<Placement>(makeMutable<TilePlacement>(std::move(updateParameters_)));
    }
    assert(false);
    return makeMutable<Placement>();
}

} // namespace mbgl
