#include <mbgl/tile/tile_diff.hpp>

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
/// Output iterator that inserts elements in order into a target sorted container
template <typename TTarget, typename TComp>
struct OrderedInsert {
    TTarget& target;
    TComp compare;
    template <typename TItem>
    void operator()(const TItem& id) {
        target.insert(std::upper_bound(target.begin(), target.end(), id, compare), id);
    }
};
/// Type inference for `OrderedInsert`
template <typename TComp, typename TTarget>
auto make_ordered_insert(TTarget& target, TComp compare) {
    return OrderedInsert<TTarget, TComp>{target, std::move(compare)};
}

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
    // `RenderTiles` is sorted, but by their `UnwrappedTileID` rather than `OverscaledTileID` (the
    // key type in the map used in `TilePyramid::renderedTiles`).  Since the overscaled-z is compared
    // first, the order can be different, so we have to pull out the overscaled ID (from the contained
    // `Tile`) and sort by that.
    std::vector<OverscaledTileID> sortedA;
    sortedA.reserve(a.size());
    std::for_each(a.begin(), a.end(), make_ordered_insert(sortedA, std::less<OverscaledTileID>{}));
    assert(std::is_sorted(sortedA.begin(), sortedA.end(), std::less<OverscaledTileID>{}));

    std::vector<RenderTiles::element_type::value_type> sortedB;
    sortedB.reserve(std::distance(b->begin(), b->end()));
    std::for_each(b->begin(), b->end(), make_ordered_insert(sortedB, OverscaledLess{}));
    assert(std::is_sorted(sortedB.begin(), sortedB.end(), OverscaledLess{}));

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
