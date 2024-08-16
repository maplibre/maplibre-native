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

// Add just the Tile ID portion of a result (for deleted items)
struct IDOnlyInserter {
    std::vector<OverscaledTileID>& target;
    IDOnlyInserter& operator++() { return *this; }
    IDOnlyInserter& operator++(int) { return *this; }
    IDOnlyInserter& operator*() { return *this; }
    void operator=(const RenderTile& tile) { target.push_back(tile.getOverscaledTileID()); }
};
// Add just the right-hand (new) side
struct RightInserter {
    std::vector<RenderTiles::element_type::value_type>& target;
    RightInserter& operator++() { return *this; }
    RightInserter& operator++(int) { return *this; }
    RightInserter& operator*() { return *this; }
    void operator=(std::pair<const RenderTile&, const RenderTile&> value) { target.push_back(value.second); }
};
} // namespace

/// @brief Compute the differences in tile IDs between two containers of `RenderTile`
TileDifference diffTiles(const RenderTiles& a, const RenderTiles& b) {
    // `RenderTiles` is sorted, but by their `UnwrappedTileID` rather than `OverscaledTileID` (the
    // key type in the map used in `TilePyramid::renderedTiles`).  Since the overscaled-z is compared
    // first, the order can be different, so we have to pull out the overscaled ID (from the contained
    // `Tile`) and sort by that.
    std::vector<RenderTiles::element_type::value_type> sortedA;
    sortedA.reserve(std::distance(a->begin(), a->end()));
    std::for_each(a->begin(), a->end(), make_ordered_insert(sortedA, OverscaledLess{}));
    assert(std::is_sorted(sortedA.begin(), sortedA.end(), OverscaledLess{}));

    std::vector<RenderTiles::element_type::value_type> sortedB;
    sortedB.reserve(std::distance(b->begin(), b->end()));
    std::for_each(b->begin(), b->end(), make_ordered_insert(sortedB, OverscaledLess{}));
    assert(std::is_sorted(sortedB.begin(), sortedB.end(), OverscaledLess{}));

    TileDifference result;
    result.remainder.reserve(std::max(sortedA.size(), sortedB.size()));
    util::set_segregate(sortedA,
                        sortedB,
                        IDOnlyInserter{result.removed},
                        std::back_inserter(result.added),
                        RightInserter{result.remainder},
                        OverscaledLess{});
    return result;
}

} // namespace mbgl
