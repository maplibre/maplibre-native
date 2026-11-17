#if MLN_RENDER_BACKEND_OPENGL
#include <mln/test/util.hpp>

#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/texture2d.hpp>
#include <mln/gl/context.hpp>
#include <mln/gl/headless_backend.hpp>
#include <mln/renderer/texture_pool.hpp>

#include <set>

using namespace mln;

namespace {

constexpr uint32_t fullSize = 64;
constexpr uint32_t halfSize = 32;

// A GL context to make the targets in. Their textures are allocated when a target first bakes, so
// none is here, but the size of each is known from the start.
struct Fixture {
    gl::HeadlessBackend backend{{32, 32}};
    gfx::BackendScope scope{backend};
    gl::Context context{backend};
    TexturePool pool{fullSize, halfSize};
    const Color background = Color::black();

    TexturePool::Resizes create(const std::set<UnwrappedTileID>& cover,
                                const std::set<UnwrappedTileID>& full,
                                size_t maxResizes = 0) {
        return pool.createRenderTargets(context, cover, full, background, maxResizes);
    }

    uint32_t width(const UnwrappedTileID& id) const {
        const auto target = pool.getRenderTarget(id);
        return target ? target->getTexture()->getSize().width : 0;
    }
};

UnwrappedTileID tile(uint32_t x) {
    return UnwrappedTileID(15, 17000 + x, 10000);
}

std::set<UnwrappedTileID> tiles(uint32_t count) {
    std::set<UnwrappedTileID> out;
    for (uint32_t x = 0; x < count; ++x) out.insert(tile(x));
    return out;
}

} // namespace

TEST(DrapeTargetPool, EachTileGetsTheSizeItAsksFor) {
    Fixture f;
    f.create({tile(0), tile(1)}, {tile(0)});
    EXPECT_EQ(fullSize, f.width(tile(0)));
    EXPECT_EQ(halfSize, f.width(tile(1)));
    EXPECT_EQ(std::set<UnwrappedTileID>{tile(0)}, f.pool.tilesAtFullSize());
}

TEST(DrapeTargetPool, ATargetOfTheRightSizeIsKept) {
    Fixture f;
    f.create({tile(0), tile(1)}, {tile(0)});
    const auto full = f.pool.getRenderTarget(tile(0));
    const auto half = f.pool.getRenderTarget(tile(1));
    EXPECT_TRUE(f.create({tile(0), tile(1)}, {tile(0)}).replaced.empty());
    EXPECT_EQ(full, f.pool.getRenderTarget(tile(0)));
    EXPECT_EQ(half, f.pool.getRenderTarget(tile(1)));
}

TEST(DrapeTargetPool, ATargetOfTheWrongSizeIsReplaced) {
    // The tile that moves between the full-size and half-size sets gets a new target, and so a new
    // texture: the terrain drawable has to follow it.
    Fixture f;
    f.create({tile(0), tile(1)}, {tile(0)});
    const auto before = f.pool.getRenderTarget(tile(0));

    f.create({tile(0), tile(1)}, {tile(1)});
    EXPECT_EQ(halfSize, f.width(tile(0)));
    EXPECT_EQ(fullSize, f.width(tile(1)));
    EXPECT_NE(before, f.pool.getRenderTarget(tile(0)));
}

TEST(DrapeTargetPool, AtMostTheLimitChangeSizeInACall) {
    // A replaced target bakes at once, outside the drape budget, so the pool paces them itself.
    Fixture f;
    f.create(tiles(6), {});
    ASSERT_TRUE(f.pool.tilesAtFullSize().empty());

    auto resizes = f.create(tiles(6), tiles(6), 2);
    EXPECT_EQ(2u, resizes.replaced.size());
    EXPECT_TRUE(resizes.deferred);
    EXPECT_EQ(2u, f.pool.tilesAtFullSize().size());
    resizes = f.create(tiles(6), tiles(6), 2);
    EXPECT_EQ(2u, resizes.replaced.size());
    EXPECT_TRUE(resizes.deferred);
    EXPECT_EQ(4u, f.pool.tilesAtFullSize().size());
    resizes = f.create(tiles(6), tiles(6), 2);
    EXPECT_EQ(2u, resizes.replaced.size());
    EXPECT_FALSE(resizes.deferred);
    EXPECT_EQ(6u, f.pool.tilesAtFullSize().size());
}

TEST(DrapeTargetPool, TargetsShrinkBeforeOthersGrow) {
    // Four tiles are full size and the view now wants the other four, which sort first by tile ID.
    // With room for four changes, the four giving up their memory go first: the full-size count
    // falls to nothing before it rises, instead of reaching eight for a frame.
    Fixture f;
    f.create(tiles(8), {tile(4), tile(5), tile(6), tile(7)});
    const std::set<UnwrappedTileID> wanted{tile(0), tile(1), tile(2), tile(3)};

    auto resizes = f.create(tiles(8), wanted, 4);
    EXPECT_EQ((std::set<UnwrappedTileID>{tile(4), tile(5), tile(6), tile(7)}), resizes.replaced);
    EXPECT_TRUE(resizes.deferred);
    EXPECT_TRUE(f.pool.tilesAtFullSize().empty());

    // With room for six, the four shrink and two grow.
    Fixture g;
    g.create(tiles(8), {tile(4), tile(5), tile(6), tile(7)});
    resizes = g.create(tiles(8), wanted, 6);
    EXPECT_EQ(6u, resizes.replaced.size());
    EXPECT_EQ((std::set<UnwrappedTileID>{tile(0), tile(1)}), g.pool.tilesAtFullSize());
    EXPECT_TRUE(resizes.deferred);

    resizes = g.create(tiles(8), wanted, 6);
    EXPECT_EQ(wanted, g.pool.tilesAtFullSize());
    EXPECT_FALSE(resizes.deferred);
}

TEST(DrapeTargetPool, WithoutALimitEverySwapIsMadeAtOnce) {
    // A still gets no later frame to finish deferred swaps, so it asks for none: all of them, the
    // shrinking and the growing, in the one call.
    Fixture f;
    f.create(tiles(8), {tile(4), tile(5), tile(6), tile(7)});
    const std::set<UnwrappedTileID> wanted{tile(0), tile(1), tile(2), tile(3)};

    const auto resizes = f.create(tiles(8), wanted, 0);
    EXPECT_EQ(8u, resizes.replaced.size());
    EXPECT_FALSE(resizes.deferred);
    EXPECT_EQ(wanted, f.pool.tilesAtFullSize());
}

TEST(DrapeTargetPool, NewTargetsAreNotLimited) {
    // Without a target a tile would be blank: all of them are made, whatever the limit.
    Fixture f;
    const auto resizes = f.create(tiles(6), tiles(6), 2);
    EXPECT_TRUE(resizes.replaced.empty());
    EXPECT_FALSE(resizes.deferred);
    EXPECT_EQ(6u, f.pool.tilesAtFullSize().size());
}

TEST(DrapeTargetPool, AFailedTargetIsDroppedAndRebuiltSmaller) {
    Fixture f;
    f.create({tile(0), tile(1)}, {tile(0), tile(1)});
    f.pool.renderTargetFailed(tile(0));
    EXPECT_EQ(0u, f.width(tile(0)));

    // The tile gets a target again at once - the old one recorded a bake it never made - but half
    // size, as does a tile asking for full size that had no target; one that has it keeps it.
    f.create({tile(0), tile(1), tile(2)}, {tile(0), tile(1), tile(2)});
    EXPECT_EQ(halfSize, f.width(tile(0)));
    EXPECT_EQ(fullSize, f.width(tile(1)));
    EXPECT_EQ(halfSize, f.width(tile(2)));
}

TEST(DrapeTargetPool, FullSizeComesBackAfterExactlyTheWindow) {
    Fixture f;
    f.create({tile(0)}, {tile(0)});
    EXPECT_TRUE(f.pool.renderTargetFailed(tile(0)));

    // The tile asks for full size on every frame of the window and does not get it; on the frame
    // after the window it does.
    for (uint32_t frame = 1; frame <= TexturePool::failureWindowFrames; ++frame) {
        f.create({tile(0)}, {tile(0)});
        EXPECT_EQ(halfSize, f.width(tile(0))) << "frame " << frame;
    }
    f.create({tile(0)}, {tile(0)});
    EXPECT_EQ(fullSize, f.width(tile(0)));
}

TEST(DrapeTargetPool, AFailureThatGoesOnIsReportedOncePerWindow) {
    // If even a half-size target cannot be had, the same tile fails every frame it is drawn: the
    // first failure logs and asks for another frame, and the ones after it, for the rest of the
    // window, only retry quietly.
    Fixture f;
    f.create({tile(0)}, {});
    EXPECT_TRUE(f.pool.renderTargetFailed(tile(0)));
    for (uint32_t frame = 1; frame < TexturePool::failureWindowFrames; ++frame) {
        f.create({tile(0)}, {});
        EXPECT_FALSE(f.pool.renderTargetFailed(tile(0))) << "frame " << frame;
    }

    // A failure inside the window does not extend it: the window ends on its own frame, and the
    // failure that goes on is reported again there, and quietly after that.
    f.create({tile(0)}, {});
    EXPECT_TRUE(f.pool.renderTargetFailed(tile(0)));
    f.create({tile(0)}, {});
    EXPECT_FALSE(f.pool.renderTargetFailed(tile(0)));
}

#endif
