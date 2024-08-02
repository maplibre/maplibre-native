#pragma once

#include <mbgl/style/image_impl.hpp>
#include <mbgl/style/source_impl.hpp>
#include <mbgl/style/layer_impl.hpp>
#include <mbgl/util/immutable.hpp>
#include <mbgl/util/variant.hpp>

#include <boost/iterator/iterator_adaptor.hpp>

namespace mbgl {

class RenderTile;

struct TileDifference {
    const std::vector<OverscaledTileID> added;
    const std::vector<OverscaledTileID> removed;
    const std::vector<OverscaledTileID> remainder;
};

namespace tile_diff_detail {

template <typename W, typename T>
inline constexpr bool is_ref_wrap = std::is_same_v<std::remove_cv_t<typename W::value_type>, std::reference_wrapper<T>>;
template <typename T, typename TI>
using adaptor_base = boost::iterator_adaptor<TI, T, boost::use_default, boost::use_default, const OverscaledTileID&>;

// Iterate a collection of `RenderTile` (or `reference_wrapper` thereto) as if they were tile IDs
template <typename T, bool wrap = is_ref_wrap<T, RenderTile> || is_ref_wrap<T, const RenderTile>>
struct TileIDIterator : public adaptor_base<T, TileIDIterator<T>> {
    const OverscaledTileID& dereference() const { return this->base()->get().getOverscaledTileID(); }
    TileIDIterator(const TileIDIterator<T>::iterator_adaptor_::base_type& p)
        : TileIDIterator<T>::iterator_adaptor_(p) {}
};
template <typename T>
struct TileIDIterator<T, false> : public adaptor_base<T, TileIDIterator<T>> {
    const OverscaledTileID& dereference() const { return this->base()->getOverscaledTileID(); }
    TileIDIterator(const TileIDIterator<T>::iterator_adaptor_::base_type& p)
        : TileIDIterator<T>::iterator_adaptor_(p) {}
};
} // namespace tile_diff_detail

/// @brief Compute the differences in tile IDs between two containers of `RenderTile` ordered by tile ID
template <typename TIterA, typename TIterB>
TileDifference diffTiles(TIterA aBeg_, TIterA aEnd_, TIterB bBeg_, TIterB bEnd_) {
    using namespace tile_diff_detail;
    const TileIDIterator<TIterA> aBeg = aBeg_;
    const TileIDIterator<TIterA> aEnd = aEnd_;
    const TileIDIterator<TIterB> bBeg = bBeg_;
    const TileIDIterator<TIterB> bEnd = bEnd_;

    assert(std::is_sorted(aBeg, aEnd));
    assert(std::is_sorted(bBeg, bEnd));

    std::vector<OverscaledTileID> added;
    std::set_difference(bBeg, bEnd, aBeg, aEnd, std::back_inserter(added));

    std::vector<OverscaledTileID> removed;
    std::set_difference(aBeg, aEnd, bBeg, bEnd, std::back_inserter(removed));

    std::vector<OverscaledTileID> remainder;
    std::set_intersection(aBeg, aEnd, bBeg, bEnd, std::back_inserter(remainder));

    return {
        std::move(added),
        std::move(removed),
        std::move(remainder),
    };
}

} // namespace mbgl
