#include <mbgl/tile/tile_diff.hpp>
#include <mbgl/util/std.hpp>

namespace mbgl {
namespace util {
/// Run `set_difference(A,B)`, `set_difference(B,A)`, and `set_intersection(A,B)` at the same time
template <class LeftRange, class RightRange, class OutputLeft, class OutputRight, class OutputIntersect, class Compare>
void set_segregate(LeftRange const& leftRange,
                   RightRange const& rightRange,
                   OutputLeft leftOnly,
                   OutputRight rightOnly,
                   OutputIntersect both,
                   Compare comp) {
    auto left = leftRange.begin();
    auto right = rightRange.begin();
    while (left != leftRange.end()) {
        if (right == rightRange.end()) {
            std::copy(left, leftRange.end(), leftOnly);
            return;
        }
        if (comp(*left, *right)) {
            *leftOnly++ = *left++;
        } else {
            if (!comp(*right, *left)) {
                *both++ = std::make_pair(*left++, *right++);
            } else {
                *rightOnly++ = *right++;
            }
        }
    }
    std::copy(right, rightRange.end(), rightOnly);
}
} // namespace util

namespace {
/// Compare `RenderTile`s by their overscaled tile ID
struct OverscaledLess {
    bool operator()(const RenderTile& lhs, const RenderTile& rhs) const {
        return lhs.getOverscaledTileID() < rhs.getOverscaledTileID();
    }
    bool operator()(const OverscaledTileID& lhs, const RenderTile& rhs) const {
        return lhs < rhs.getOverscaledTileID();
    }
    bool operator()(const RenderTile& lhs, const OverscaledTileID& rhs) const {
        return lhs.getOverscaledTileID() < rhs;
    }
};

// Add just the right-hand (new) side
struct RightInserter {
    std::vector<RenderTiles::element_type::value_type>& target;
    RightInserter& operator++() { return *this; }
    RightInserter& operator++(int) { return *this; }
    RightInserter& operator*() { return *this; }
    void operator=(std::pair<const OverscaledTileID&, const RenderTile&> value) { target.push_back(value.second); }
};
} // namespace

TileDifference diffTiles(const std::vector<OverscaledTileID>& a, const RenderTiles& b) {
    // `RenderTiles` is sorted, but by their `UnwrappedTileID` rather than `OverscaledTileID` (the key type in
    // the map used by `TilePyramid::renderedTiles`).  Since the overscaled-z is compared first, the order can
    // be different, so we have to pull out the overscaled ID from the contained `Tile` and sort by that.
    std::vector<OverscaledTileID> sortedA;
    sortedA.reserve(a.size());
    std::ranges::copy(a, util::make_ordered_inserter(sortedA));
    assert(std::ranges::is_sorted(sortedA, std::less<OverscaledTileID>{}));

    std::vector<RenderTiles::element_type::value_type> sortedB;
    sortedB.reserve(b->size());
    std::ranges::copy(*b, util::make_ordered_inserter(sortedB, OverscaledLess{}));
    assert(std::ranges::is_sorted(sortedB, OverscaledLess{}));

    TileDifference result;
    result.remainder.reserve(std::max(sortedA.size(), sortedB.size()));
    util::set_segregate(sortedA,
                        sortedB,
                        std::back_inserter(result.removed),
                        std::back_inserter(result.added),
                        RightInserter{result.remainder},
                        OverscaledLess{});
    return result;
}

} // namespace mbgl
