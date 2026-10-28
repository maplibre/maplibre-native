#include <mln/test/util.hpp>

#include <mln/map/transform_state.hpp>
#include <mln/renderer/tile_pyramid.hpp>

using namespace mln;

namespace {

TransformState transformState(const Size size) {
    TransformState state;
    state.setSize(size);
    state.setMinZoom(0);
    state.setMaxZoom(22);
    return state;
}

} // namespace

TEST(TilePyramid, CacheSizeCoversTheZoomRangeByDefault) {
    EXPECT_EQ(11u, TilePyramid::cacheSize(transformState({512, 512}), 512, std::nullopt));
    EXPECT_EQ(44u, TilePyramid::cacheSize(transformState({1024, 1024}), 512, std::nullopt));
}

TEST(TilePyramid, CacheSizeIsTheConfiguredOne) {
    EXPECT_EQ(2u, TilePyramid::cacheSize(transformState({512, 512}), 512, 2));
    EXPECT_EQ(0u, TilePyramid::cacheSize(transformState({1024, 1024}), 512, 0));
}
