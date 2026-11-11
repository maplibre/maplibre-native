#include <mln/test/util.hpp>

#include <mln/geometry/anchor.hpp>
#include <mln/layout/symbol_layout.hpp>
#include <mln/layout/symbol_projection.hpp>
#include <mln/map/transform_state.hpp>
#include <mln/math/angles.hpp>
#include <mln/renderer/buckets/symbol_bucket.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/style/variable_anchor_offset_collection.hpp>
#include <mln/text/placement.hpp>
#include <mln/tile/tile.hpp>
#include <mln/util/mat4.hpp>

#include <cmath>
#include <memory>
#include <numbers>
#include <optional>

using namespace mln;

namespace {

constexpr float margin = 20.0f;

// Whether a label whose unflipped reading direction points `degrees` counterclockwise from the
// screen's x axis (gl coordinates, y up: 90 = straight up) has to be drawn flipped.
bool needsFlip(float degrees,
               std::optional<bool> wasFlipped,
               float aspectRatio = 1.0f,
               float keepUprightMargin = margin) {
    const float radians = util::deg2radf(degrees);
    const Point<float> first{0.0f, 0.0f};
    const Point<float> last{100.0f * std::cos(radians) / aspectRatio, 100.0f * std::sin(radians)};
    return lineLabelNeedsFlip(first, last, aspectRatio, wasFlipped, keepUprightMargin);
}

} // namespace

TEST(LineLabelFlip, FirstDrawingFlipsWhatRunsRightToLeft) {
    // As without the margin, also right next to the vertical.
    EXPECT_FALSE(needsFlip(0.0f, std::nullopt));
    EXPECT_FALSE(needsFlip(45.0f, std::nullopt));
    EXPECT_FALSE(needsFlip(89.0f, std::nullopt));
    EXPECT_TRUE(needsFlip(91.0f, std::nullopt));
    EXPECT_TRUE(needsFlip(180.0f, std::nullopt));
    EXPECT_TRUE(needsFlip(225.0f, std::nullopt));
    EXPECT_TRUE(needsFlip(269.0f, std::nullopt));
    EXPECT_FALSE(needsFlip(271.0f, std::nullopt));
}

TEST(LineLabelFlip, KeepsItsDirectionWhileTheLineStaysCloseToTheVertical) {
    // Drawn unflipped, the map rotating a few degrees either way around the vertical of the line.
    for (float degrees : {85.0f, 92.0f, 95.0f, 105.0f, 265.0f, 275.0f, 255.0f}) {
        EXPECT_FALSE(needsFlip(degrees, false)) << degrees;
    }
    // Drawn flipped: it stays flipped through the same rotations.
    for (float degrees : {95.0f, 88.0f, 85.0f, 75.0f, 265.0f, 275.0f, 285.0f}) {
        EXPECT_TRUE(needsFlip(degrees, true)) << degrees;
    }
}

TEST(LineLabelFlip, TurnsOverOnceClearlyUpsideDown) {
    EXPECT_TRUE(needsFlip(90.0f + margin + 1.0f, false));
    EXPECT_TRUE(needsFlip(270.0f - margin - 1.0f, false));
    EXPECT_FALSE(needsFlip(90.0f - margin - 1.0f, true));
    EXPECT_FALSE(needsFlip(270.0f + margin + 1.0f, true));
}

TEST(LineLabelFlip, MeasuresTheMarginOnTheScreenNotInGlUnits) {
    // On a tall screen one gl unit across is much shorter than one up: 15 degrees from the vertical
    // on the screen is inside the margin, even though in raw gl units it would look like 30.
    EXPECT_FALSE(needsFlip(105.0f, false, 0.46f));
    EXPECT_TRUE(needsFlip(105.0f, true, 0.46f));
}

TEST(LineLabelFlip, WithoutAMarginTheLastDirectionIsIgnored) {
    // The default: decided anew on every drawing, whichever way the label was drawn before.
    for (float degrees : {0.0f, 85.0f, 89.0f, 91.0f, 95.0f, 180.0f, 265.0f, 269.0f, 271.0f, 275.0f}) {
        const bool rule = needsFlip(degrees, std::nullopt, 1.0f, 0.0f);
        EXPECT_EQ(rule, degrees > 90.0f && degrees < 270.0f) << degrees;
        EXPECT_EQ(needsFlip(degrees, false, 1.0f, 0.0f), rule) << degrees;
        EXPECT_EQ(needsFlip(degrees, true, 1.0f, 0.0f), rule) << degrees;
    }
    // Straight up: not flipped, as today, even if it was drawn flipped before.
    EXPECT_FALSE(lineLabelNeedsFlip({0.0f, 0.0f}, {0.0f, 100.0f}, 1.0f, true, 0.0f));
}

namespace {

class FakeTile final : public Tile {
public:
    explicit FakeTile(const OverscaledTileID& id)
        : Tile(Kind::Geometry, id, "source") {}
    std::unique_ptr<TileRenderData> createRenderData() override { return nullptr; }
    void cancel() override {}
    bool layerPropertiesUpdated(const Immutable<style::LayerProperties>&) override { return true; }
};

// Placement with a keep-upright margin, with access to what it does on every frame
// (updateBucketDynamicAttributeData) and once per placement (syncLineLabelDirections).
class TestPlacement : public Placement {
public:
    explicit TestPlacement(float keepUprightMargin_) { keepUprightMargin = keepUprightMargin_; }
    using Placement::syncLineLabelDirections;
    using Placement::updateBucketDynamicAttributeData;
};

SymbolInstance makeLineLabel(uint32_t crossTileID) {
    const ShapedTextOrientations shaping{};
    const style::SymbolLayoutProperties::Evaluated layout;
    const std::array<float, 2> offset{{0.0f, 0.0f}};
    const ImageMap imageMap;
    auto sharedData = std::make_shared<SymbolInstanceSharedData>(GeometryCoordinates{},
                                                                 shaping,
                                                                 std::nullopt,
                                                                 std::nullopt,
                                                                 layout,
                                                                 style::SymbolPlacementType::Line,
                                                                 offset,
                                                                 imageMap,
                                                                 0.0f,
                                                                 SymbolContent::IconRGBA,
                                                                 false,
                                                                 false);
    Anchor anchor(4096.0f, 4000.0f, 0.0f, 0u);
    const IndexedSubfeature subfeature(0, {}, {}, 0);
    SymbolInstance symbol(anchor,
                          std::move(sharedData),
                          shaping,
                          std::nullopt,
                          std::nullopt,
                          0,
                          0,
                          style::SymbolPlacementType::Line,
                          offset,
                          0,
                          0,
                          offset,
                          subfeature,
                          0,
                          0,
                          u"Main Street",
                          0.0f,
                          0.0f,
                          0.0f,
                          VariableAnchorOffsetCollection(),
                          false);
    symbol.setCrossTileID(crossTileID);
    return symbol;
}

// A text (three glyphs) or an icon (one) along a line that runs straight up its tile.
PlacedSymbol makePlacedSymbol(std::vector<float> glyphOffsets) {
    const GeometryCoordinates line{{4096, 7000}, {4096, 1000}};
    const Anchor anchor(4096.0f, 4000.0f, 0.0f, 0u);
    PlacedSymbol placed(anchor.point,
                        0,
                        16.0f,
                        16.0f,
                        {{0.0f, 0.0f}},
                        WritingModeType::Horizontal,
                        line,
                        SymbolLayout::calculateTileDistances(line, anchor),
                        std::nullopt);
    placed.glyphOffsets = std::move(glyphOffsets);
    return placed;
}

// A bucket of a layer of line labels kept upright and rotated with the map: one label per cross
// tile id, each with its text and an icon (an arrow along the line, kept upright as well), and one
// SDF icon along the same line.
std::unique_ptr<SymbolBucket> makeBucket(const std::vector<uint32_t>& crossTileIDs) {
    auto layout = makeMutable<style::SymbolLayoutProperties::PossiblyEvaluated>();
    layout->get<style::SymbolPlacement>() = style::SymbolPlacementType::Line;
    layout->get<style::TextRotationAlignment>() = style::AlignmentType::Map;
    layout->get<style::TextPitchAlignment>() = style::AlignmentType::Viewport;
    layout->get<style::TextKeepUpright>() = true;
    layout->get<style::IconRotationAlignment>() = style::AlignmentType::Map;
    layout->get<style::IconPitchAlignment>() = style::AlignmentType::Viewport;
    layout->get<style::IconKeepUpright>() = true;
    std::vector<SymbolInstance> symbols;
    for (const uint32_t crossTileID : crossTileIDs) {
        symbols.push_back(makeLineLabel(crossTileID));
    }
    auto bucket = std::make_unique<SymbolBucket>(std::move(layout),
                                                 std::map<std::string, Immutable<style::LayerProperties>>{},
                                                 16.0f,
                                                 1.0f,
                                                 0.0f,
                                                 false,
                                                 false,
                                                 "layer",
                                                 std::move(symbols),
                                                 std::vector<SortKeyRange>{},
                                                 1.0f,
                                                 false,
                                                 std::vector<style::TextWritingModeType>{},
                                                 false);
    bucket->text.segments.emplace_back(0, 0);
    bucket->icon.segments.emplace_back(0, 0);
    bucket->sdfIcon.segments.emplace_back(0, 0);
    bucket->sdfIcon.placedSymbols.push_back(makePlacedSymbol({0.0f}));
    for (std::size_t i = 0; i < bucket->symbolInstances.size(); ++i) {
        bucket->text.placedSymbols.push_back(makePlacedSymbol({-30.0f, 0.0f, 30.0f}));
        bucket->symbolInstances[i].setPlacedRightTextIndex(i);
        bucket->icon.placedSymbols.push_back(makePlacedSymbol({0.0f}));
        bucket->symbolInstances[i].refPlacedIconIndex() = i;
    }
    return bucket;
}

std::optional<bool>& textDirection(SymbolBucket& bucket, std::size_t label = 0) {
    return bucket.text.placedSymbols.at(label).keepUprightFlipped;
}

std::optional<bool>& iconDirection(SymbolBucket& bucket) {
    return bucket.icon.placedSymbols.at(0).keepUprightFlipped;
}

std::optional<bool>& sdfIconDirection(SymbolBucket& bucket) {
    return bucket.sdfIcon.placedSymbols.at(0).keepUprightFlipped;
}

// The angles at which a frame drew the first glyph of the first text, the first icon and the SDF
// icon of a bucket.
struct Angles {
    float text;
    float icon;
    float sdfIcon;
};

// A map of 256 x 512 at zoom 0, showing the whole tile 0/0/0.
class Frame {
public:
    Frame()
        : tile(OverscaledTileID(0, 0, 0)),
          renderTile(UnwrappedTileID(0, 0, 0), tile) {
        state.setSize({256, 512});
        state.setLatLngZoom(LatLng(0, 0), 0);
        setBearing(0.0);
    }

    // The tile's matrix at this bearing, in degrees.
    void setBearing(double degrees) {
        state.setBearing(util::deg2rad(-degrees));
        state.matrixFor(renderTile.matrix, renderTile.id);
        matrix::multiply(renderTile.matrix, TransformParameters(state).projMatrix, renderTile.matrix);
    }

    // Draws the bucket as on every frame.
    Angles draw(const TestPlacement& placement, SymbolBucket& bucket) {
        placement.updateBucketDynamicAttributeData(bucket, state, renderTile);
        return {.text = bucket.text.dynamicAttributeData().at(0).a1[2],
                .icon = bucket.icon.dynamicAttributeData().at(0).a1[2],
                .sdfIcon = bucket.sdfIcon.dynamicAttributeData().at(0).a1[2]};
    }

    // What Placement does with a layer once per placement or change of tiles.
    void sync(const TestPlacement& placement, const std::vector<SymbolBucket*>& buckets) {
        LayerPlacementData data;
        for (SymbolBucket* bucket : buckets) {
            data.push_back({.bucket = *bucket,
                            .tile = renderTile,
                            .featureIndex = nullptr,
                            .sourceId = "source",
                            .sortKeyRange = std::nullopt});
        }
        placement.syncLineLabelDirections(data);
    }

    FakeTile tile;
    RenderTile renderTile;
    TransformState state;
};

// Whether a glyph drawn at `angle` and later at `laterAngle` was turned over in between.
bool turnedOver(float angle, float laterAngle) {
    constexpr float pi = std::numbers::pi_v<float>;
    const float difference = std::fmod(std::abs(laterAngle - angle), 2.0f * pi);
    return std::min(difference, 2.0f * pi - difference) > pi / 2.0f;
}

} // namespace

TEST(LineLabelFlip, WithAMarginTheTextKeepsItsDirectionButIconsDoNot) {
    const TestPlacement placement(margin);
    auto bucket = makeBucket({1});
    Frame frame;
    frame.setBearing(-6.0);
    const auto first = frame.draw(placement, *bucket);
    ASSERT_TRUE(textDirection(*bucket).has_value());
    const bool firstDirection = *textDirection(*bucket);

    // The map turns from 6 degrees one side of the line to 6 degrees the other side.
    frame.setBearing(6.0);
    const auto second = frame.draw(placement, *bucket);
    EXPECT_FALSE(turnedOver(first.text, second.text));
    EXPECT_EQ(textDirection(*bucket), firstDirection);
    // Icons, kept upright too, turn over as without the margin: nothing is remembered for them.
    EXPECT_TRUE(turnedOver(first.icon, second.icon));
    EXPECT_TRUE(turnedOver(first.sdfIcon, second.sdfIcon));
    EXPECT_FALSE(iconDirection(*bucket).has_value());
    EXPECT_FALSE(sdfIconDirection(*bucket).has_value());

    // Past the margin the text turns over as well.
    frame.setBearing(30.0);
    const auto third = frame.draw(placement, *bucket);
    EXPECT_TRUE(turnedOver(first.text, third.text));
    EXPECT_EQ(textDirection(*bucket), !firstDirection);
}

TEST(LineLabelFlip, WithoutAMarginNothingIsRemembered) {
    const TestPlacement placement(0.0f);
    auto bucket = makeBucket({1});
    Frame frame;
    frame.setBearing(-6.0);
    const auto first = frame.draw(placement, *bucket);
    frame.setBearing(6.0);
    const auto second = frame.draw(placement, *bucket);
    // Today's rule: text and icons all turn over at the vertical.
    EXPECT_TRUE(turnedOver(first.text, second.text));
    EXPECT_TRUE(turnedOver(first.icon, second.icon));
    EXPECT_TRUE(turnedOver(first.sdfIcon, second.sdfIcon));
    EXPECT_FALSE(textDirection(*bucket).has_value());
    EXPECT_FALSE(iconDirection(*bucket).has_value());
    EXPECT_FALSE(sdfIconDirection(*bucket).has_value());

    // Nor is a direction set on the label read.
    for (const bool wasFlipped : {false, true}) {
        textDirection(*bucket) = wasFlipped;
        EXPECT_EQ(frame.draw(placement, *bucket).text, second.text) << wasFlipped;
    }
}

TEST(LineLabelFlip, WithoutAMarginNothingIsSynced) {
    const TestPlacement placement(0.0f);
    auto drawn = makeBucket({1});
    textDirection(*drawn) = true;
    auto fromCache = makeBucket({1});
    fromCache->lineLabelDirectionsStale = true;
    Frame frame;
    frame.sync(placement, {drawn.get(), fromCache.get()});
    EXPECT_FALSE(textDirection(*fromCache).has_value());
    EXPECT_TRUE(fromCache->lineLabelDirectionsStale);
}

TEST(LineLabelFlip, TwoCopiesOnTwoZoomLevelsReadTheSameWay) {
    const TestPlacement placement(margin);
    Frame frame;
    frame.setBearing(6.0);
    // How a label drawn for the first time reads here, 6 degrees past the vertical of its line.
    auto fresh = makeBucket({3});
    frame.draw(placement, *fresh);
    ASSERT_TRUE(textDirection(*fresh).has_value());
    const bool firstDrawing = *textDirection(*fresh);

    // The zoom 16 copies were drawn before the map turned: label 1 the other way. The zoom 17 tile
    // has just come in, listed first.
    auto z16 = makeBucket({1, 2});
    textDirection(*z16, 0) = !firstDrawing;
    textDirection(*z16, 1) = firstDrawing;
    auto z17 = makeBucket({1, 2});
    z17->lineLabelDirectionsStale = true;
    frame.sync(placement, {z17.get(), z16.get()});
    EXPECT_EQ(textDirection(*z17, 0), !firstDrawing);
    EXPECT_EQ(textDirection(*z17, 1), firstDrawing);
    EXPECT_EQ(textDirection(*z16, 0), !firstDrawing);
    EXPECT_EQ(textDirection(*z16, 1), firstDrawing);
    EXPECT_FALSE(z17->lineLabelDirectionsStale);

    // The new copy is drawn the way the label reads, not as if for the first time.
    frame.draw(placement, *z17);
    EXPECT_EQ(textDirection(*z17, 0), !firstDrawing);
    EXPECT_EQ(textDirection(*z17, 1), firstDrawing);
}

TEST(LineLabelFlip, ATileBackFromTheCacheDoesNotBringBackAnOldDirection) {
    const TestPlacement placement(margin);
    Frame frame;
    // Drawn unflipped when its tile went to the cache; since then the label has turned over on
    // another copy, which is drawn.
    auto fromCache = makeBucket({1});
    textDirection(*fromCache) = false;
    fromCache->lineLabelDirectionsStale = true;
    auto drawn = makeBucket({1});
    textDirection(*drawn) = true;
    frame.sync(placement, {fromCache.get(), drawn.get()});
    EXPECT_EQ(textDirection(*fromCache), true);
    EXPECT_EQ(textDirection(*drawn), true);

    // With no other copy, it starts afresh rather than reading the old way.
    const TestPlacement later(margin);
    auto alone = makeBucket({1});
    textDirection(*alone) = false;
    alone->lineLabelDirectionsStale = true;
    frame.sync(later, {alone.get()});
    EXPECT_FALSE(textDirection(*alone).has_value());
    EXPECT_FALSE(alone->lineLabelDirectionsStale);
}

TEST(LineLabelFlip, AHiddenCopyDoesNotPassOnItsDirection) {
    const TestPlacement placement(margin);
    Frame frame;
    auto duplicate = makeBucket({1});
    textDirection(*duplicate) = false;
    duplicate->text.placedSymbols.at(0).hidden = true;
    auto drawn = makeBucket({1});
    textDirection(*drawn) = true;
    frame.sync(placement, {drawn.get(), duplicate.get()});
    EXPECT_EQ(textDirection(*duplicate), true);
    EXPECT_EQ(textDirection(*drawn), true);
}

TEST(LineLabelFlip, IconsAreNotSynced) {
    const TestPlacement placement(margin);
    Frame frame;
    auto drawn = makeBucket({1});
    textDirection(*drawn) = true;
    iconDirection(*drawn) = false;
    auto other = makeBucket({1});
    other->lineLabelDirectionsStale = true;
    frame.sync(placement, {drawn.get(), other.get()});
    EXPECT_EQ(textDirection(*other), true);
    EXPECT_FALSE(iconDirection(*other).has_value());
    EXPECT_EQ(iconDirection(*drawn), false);
}

TEST(LineLabelFlip, ThePreviousPlacementFillsInForALabelNotDrawnNow) {
    auto drawn = makeBucket({1});
    textDirection(*drawn) = true;
    LineLabelDirections previous;
    previous.remember(*drawn);

    // Now its only copy is hidden: the direction comes from the previous placement, if any.
    auto hidden = makeBucket({1});
    hidden->text.placedSymbols.at(0).hidden = true;
    LineLabelDirections current;
    current.remember(*hidden);
    current.restore(*hidden, &previous);
    EXPECT_EQ(textDirection(*hidden), true);
    current.restore(*hidden, nullptr);
    EXPECT_FALSE(textDirection(*hidden).has_value());
}
