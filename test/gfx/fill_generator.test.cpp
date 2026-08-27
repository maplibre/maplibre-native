#include <mln/test/util.hpp>

#include <mln/gfx/fill_generator.hpp>
#include <mln/util/constants.hpp>

using namespace mln;

namespace {

GeometryCollection square(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    return {{{x0, y0}, {x1, y0}, {x1, y1}, {x0, y1}, {x0, y0}}};
}

// The line vertex packs the tile X doubled, with the round-join bit in the low bit.
bool anyOutlineVertexOutsideTileX(const gfx::VertexVector<LineLayoutVertex>& vertices) {
    for (std::size_t i = 0; i < vertices.elements(); i++) {
        const int16_t x = static_cast<int16_t>(vertices.at(i).a1[0] / 2);
        if (x < 0 || x > util::EXTENT) {
            return true;
        }
    }
    return false;
}

} // namespace

TEST(FillGenerator, ZoomZeroOutlineStaysInsideTheTile) {
    const auto wide = square(-2000, 1000, util::EXTENT + 2000, 2000);
    for (const uint8_t zoom : {0, 1}) {
        gfx::VertexVector<FillLayoutVertex> fillVertices;
        gfx::IndexVector<gfx::Triangles> fillIndexes;
        SegmentVector fillSegments;
        gfx::VertexVector<LineLayoutVertex> lineVertices;
        gfx::IndexVector<gfx::Triangles> lineIndexes;
        SegmentVector lineSegments;
        gfx::IndexVector<gfx::Lines> basicLineIndexes;
        SegmentVector basicLineSegments;
        gfx::generateFillAndOutineBuffers(wide,
                                          fillVertices,
                                          fillIndexes,
                                          fillSegments,
                                          lineVertices,
                                          lineIndexes,
                                          lineSegments,
                                          basicLineIndexes,
                                          basicLineSegments,
                                          CanonicalTileID(zoom, 0, 0),
                                          8);
        EXPECT_GT(lineVertices.elements(), 0u);
        // Only the zoom 0 tile wraps onto itself; every other zoom keeps the outline of its buffer.
        EXPECT_EQ(zoom != 0, anyOutlineVertexOutsideTileX(lineVertices)) << "zoom " << int(zoom);
    }
}
