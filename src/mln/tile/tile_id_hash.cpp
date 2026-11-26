#include <mln/tile/tile_id.hpp>
#include <mln/util/hash.hpp>

namespace std {

size_t hash<mln::CanonicalTileID>::operator()(const mln::CanonicalTileID& id) const noexcept {
    std::size_t seed = 0;
    mln::util::hash_combine(seed, id.x);
    mln::util::hash_combine(seed, id.y);
    mln::util::hash_combine(seed, id.z);
    return seed;
}

size_t hash<mln::UnwrappedTileID>::operator()(const mln::UnwrappedTileID& id) const noexcept {
    std::size_t seed = 0;
    mln::util::hash_combine(seed, std::hash<mln::CanonicalTileID>{}(id.canonical));
    mln::util::hash_combine(seed, id.wrap);
    return seed;
}

size_t hash<mln::OverscaledTileID>::operator()(const mln::OverscaledTileID& id) const noexcept {
    std::size_t seed = 0;
    mln::util::hash_combine(seed, std::hash<mln::CanonicalTileID>{}(id.canonical));
    mln::util::hash_combine(seed, id.overscaledZ);
    return seed;
}

} // namespace std
