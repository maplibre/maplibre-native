#pragma once

#include <mbgl/style/image_impl.hpp>
#include <mbgl/style/source_impl.hpp>
#include <mbgl/style/layer_impl.hpp>
#include <mbgl/util/immutable.hpp>
#include <mbgl/util/variant.hpp>

#include <boost/iterator/iterator_adaptor.hpp>

namespace mbgl {

class RenderTile;

class TileDifference {
public:
    std::vector<UnwrappedTileID> added;
    std::vector<UnwrappedTileID> removed;
    std::vector<UnwrappedTileID> remainder;
};

namespace detail {
// Iterate a collection of `RenderTile` as if they were tile IDs
template <typename T>
struct TileIDIterator : public boost::iterator_adaptor<TileIDIterator<T>,
                                                       T,
                                                       UnwrappedTileID,
                                                       boost::random_access_traversal_tag,
                                                       const UnwrappedTileID&> {
    const UnwrappedTileID& dereference() const { return this->base()->id; }
    TileIDIterator(const TileIDIterator<T>::iterator_adaptor_::base_type& p)
        : TileIDIterator<T>::iterator_adaptor_(p) {}
};
} // namespace detail

/// @brief Compute the differences in tile IDs between two containers of `RenderTile` ordered by tile ID
template <typename TIterA, typename TIterB>
TileDifference diffTiles(TIterA aBeg_, TIterA aEnd_, TIterB bBeg_, TIterB bEnd_) {
    using namespace detail;
    const TileIDIterator<TIterA> aBeg = aBeg_;
    const TileIDIterator<TIterA> aEnd = aEnd_;
    const TileIDIterator<TIterB> bBeg = bBeg_;
    const TileIDIterator<TIterB> bEnd = bEnd_;

    assert(std::is_sorted(aBeg, aEnd));
    assert(std::is_sorted(bBeg, bEnd));

    TileDifference result;
    std::set_difference(aBeg, aEnd, bBeg, bEnd, std::back_inserter(result.removed));
    std::set_difference(bBeg, bEnd, aBeg, aEnd, std::back_inserter(result.added));
    std::set_intersection(aBeg, aEnd, bBeg, bEnd, std::back_inserter(result.remainder));

    return result;
}

} // namespace mbgl
