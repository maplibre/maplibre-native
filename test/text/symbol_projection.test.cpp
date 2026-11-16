#include <mln/test/util.hpp>

#include <mln/layout/symbol_projection.hpp>
#include <mln/math/angles.hpp>
#include <mln/renderer/buckets/symbol_bucket.hpp>
#include <mln/util/mat4.hpp>

#include <initializer_list>
#include <limits>
#include <numbers>
#include <vector>

using namespace mln;

namespace {

// A line that turns by 20 degrees at (1000, 1000): it comes in 10 degrees to one side of the
// y axis and leaves 10 degrees to the other. Layout allows a label across that bend at any
// text-max-angle of 20 degrees or more.
PlacedSymbol labelAcrossTheBend() {
    PlacedSymbol symbol(Point<float>{998.3f, 990.2f}, // 10 units before the bend
                        0,
                        24.0f,
                        24.0f,
                        {{0.0f, 0.0f}},
                        WritingModeType::Horizontal,
                        GeometryCoordinates{{653, -970}, {1000, 1000}, {653, 2970}},
                        {},
                        std::nullopt);
    symbol.glyphOffsets = {-77.0f, -55.0f, -33.0f, -11.0f, 11.0f, 33.0f, 55.0f, 77.0f};
    return symbol;
}

// The label plane of a label that faces the viewport is the screen. Seen along the y axis in a
// pitched view, distances along y shrink and distances along x do not.
mat4 screenSeenAlongY(const double shrink) {
    mat4 m;
    matrix::identity(m);
    matrix::scale(m, m, 1.0, shrink, 1.0);
    return m;
}

PlacementResult place(const mat4& labelPlaneMatrix, const float turnLimit, std::size_t* vertices = nullptr) {
    const PlacedSymbol symbol = labelAcrossTheBend();
    mat4 identity;
    matrix::identity(identity);
    SymbolBucket::DynamicAttributeVector dynamicVertexArray;
    const PlacementResult result = placeGlyphsAlongLine(symbol,
                                                        24.0f,
                                                        false,
                                                        false,
                                                        identity,
                                                        labelPlaneMatrix,
                                                        identity,
                                                        dynamicVertexArray,
                                                        project(symbol.anchorPoint, labelPlaneMatrix).first,
                                                        1.0f,
                                                        nullptr,
                                                        turnLimit);
    if (vertices) *vertices = dynamicVertexArray.elements();
    return result;
}

} // namespace

TEST(SymbolProjection, TheTurnOfALabelIsItsLargestOverThreeGlyphsInARow) {
    const auto turn = [](std::initializer_list<float> degrees) {
        std::vector<PlacedGlyph> placed;
        float x = 0.0f;
        for (const float angle : degrees) {
            placed.emplace_back(Point<float>{x, 0.0f}, util::deg2radf(angle), std::nullopt);
            x += 10.0f;
        }
        return labelPlaneTurn(placed) * 180.0f / std::numbers::pi_v<float>;
    };

    EXPECT_NEAR(10.0f, turn({5.0f, 10.0f, 15.0f, 20.0f}), 0.01f);
    // One kink.
    EXPECT_NEAR(70.0f, turn({0.0f, 0.0f, 70.0f, 70.0f}), 0.01f);
    // A turn spread over three glyphs.
    EXPECT_NEAR(100.0f, turn({0.0f, 50.0f, 100.0f, 100.0f}), 0.01f);
    // Left then right counts both turns.
    EXPECT_NEAR(120.0f, turn({0.0f, 60.0f, 0.0f}), 0.01f);
    // The same turns with a straight glyph between them do not add up.
    EXPECT_NEAR(50.0f, turn({0.0f, 50.0f, 50.0f, 100.0f}), 0.01f);
    // The turn is the shorter way round: 350 degrees to 10 degrees is 20 degrees.
    EXPECT_NEAR(20.0f, turn({350.0f, 10.0f}), 0.01f);

    // A glyph with no place on the line carries no angle and is skipped.
    const float nowhere = -std::numeric_limits<float>::infinity();
    const std::vector<PlacedGlyph> withAGap = {{Point<float>{0, 0}, 0.0f, std::nullopt},
                                               {Point<float>{nowhere, nowhere}, 3.0f, std::nullopt},
                                               {Point<float>{20, 0}, 0.1f, std::nullopt}};
    EXPECT_NEAR(0.1f, labelPlaneTurn(withAGap), 0.0001f);
}

TEST(SymbolProjection, ALabelThatFoldsOnThePitchedScreenIsLeftOut) {
    const float maxAngle = util::deg2radf(45.0f);
    const float limit = labelPlaneTurnLimit(maxAngle, false);
    EXPECT_FLOAT_EQ(2.0f * maxAngle, limit);

    // From straight above the 20 degree bend is a 20 degree bend: the label is placed.
    std::size_t vertices = 0;
    EXPECT_EQ(PlacementResult::OK, place(screenSeenAlongY(1.0), limit, &vertices));
    EXPECT_GT(vertices, 0u);

    // At a pitch of 60 degrees, looking along the y axis, y shrinks to half of x and the bend
    // is 39 degrees on the screen: a kink, and the label is still placed.
    EXPECT_EQ(PlacementResult::OK, place(screenSeenAlongY(0.5), limit, &vertices));
    EXPECT_GT(vertices, 0u);

    // At a pitch of 84 degrees y shrinks to a tenth of x and the same bend is 121 degrees on
    // the screen: the label would fold, and is left out.
    EXPECT_EQ(PlacementResult::Folded, place(screenSeenAlongY(0.1), limit, &vertices));
    EXPECT_EQ(0u, vertices);

    // Without a limit (a label that pitches with the map) it is placed as before.
    EXPECT_EQ(PlacementResult::OK, place(screenSeenAlongY(0.1), std::numeric_limits<float>::infinity(), &vertices));
    EXPECT_GT(vertices, 0u);
}

TEST(SymbolProjection, ALabelLeftOutComesBackOnlyWithinTheMaxAngle) {
    const float maxAngle = util::deg2radf(45.0f);
    const float leftOutLimit = labelPlaneTurnLimit(maxAngle, true);
    EXPECT_FLOAT_EQ(maxAngle, leftOutLimit);

    // At a pitch of about 73 degrees y shrinks to 0.3 of x and the bend is 61 degrees on the
    // screen: a label that is drawn stays, one that was left out stays out.
    EXPECT_EQ(PlacementResult::OK, place(screenSeenAlongY(0.3), labelPlaneTurnLimit(maxAngle, false)));
    EXPECT_EQ(PlacementResult::Folded, place(screenSeenAlongY(0.3), leftOutLimit));

    // Back at a pitch of 60 degrees the bend is 39 degrees and the label returns.
    EXPECT_EQ(PlacementResult::OK, place(screenSeenAlongY(0.5), leftOutLimit));
}
