#include <mln/test/util.hpp>

#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/dynamic_texture_atlas.hpp>
#include <mln/gfx/headless_backend.hpp>
#include <mln/style/image_impl.hpp>
#include <mln/util/image.hpp>

#include <set>
#include <vector>

using namespace mln;

namespace {

ImageMap makeIcon(uint8_t value) {
    PremultipliedImage image({4, 4});
    image.fill(value);
    ImageMap icons;
    icons.emplace("marker", makeMutable<style::Image::Impl>("marker", std::move(image), 1.0f));
    return icons;
}

} // namespace

TEST(DynamicTextureAtlas, UpdatedImageGetsSeparateRegion) {
    auto backend = gfx::HeadlessBackend::Create();
    auto& rendererBackend = *backend->getRendererBackend();
    gfx::BackendScope scope{rendererBackend};
    gfx::DynamicTextureAtlas atlas(rendererBackend.getContext());

    // Keep the first atlas alive, like a tile layout that is still in use.
    auto original = atlas.uploadIconsAndPatterns(makeIcon(1), {}, {});
    ASSERT_EQ(1u, original.textureHandles.size());

    // Same image ID and version share the existing region.
    auto shared = atlas.uploadIconsAndPatterns(makeIcon(1), {}, {});
    ASSERT_EQ(1u, shared.textureHandles.size());
    EXPECT_EQ(original.textureHandles[0], shared.textureHandles[0]);

    // An updated image with the same ID and size must not reuse the old pixels.
    auto updated = atlas.uploadIconsAndPatterns(makeIcon(2), {}, {{"marker", 1}});
    ASSERT_EQ(1u, updated.textureHandles.size());
    EXPECT_FALSE(original.textureHandles[0] == updated.textureHandles[0]);
    EXPECT_EQ(1u, updated.iconPositions.at("marker").version);

    // A second update while version 1 is still live must not alias it either.
    auto updatedAgain = atlas.uploadIconsAndPatterns(makeIcon(3), {}, {{"marker", 2}});
    ASSERT_EQ(1u, updatedAgain.textureHandles.size());
    EXPECT_FALSE(updated.textureHandles[0] == updatedAgain.textureHandles[0]);
    EXPECT_FALSE(original.textureHandles[0] == updatedAgain.textureHandles[0]);

    atlas.removeTextures(original.textureHandles, original.dynamicTexture);
    atlas.removeTextures(shared.textureHandles, shared.dynamicTexture);
    atlas.removeTextures(updated.textureHandles, updated.dynamicTexture);
    atlas.removeTextures(updatedAgain.textureHandles, updatedAgain.dynamicTexture);
}

TEST(DynamicTextureAtlas, ManyLiveVersionsGetSeparateRegions) {
    auto backend = gfx::HeadlessBackend::Create();
    auto& rendererBackend = *backend->getRendererBackend();
    gfx::BackendScope scope{rendererBackend};
    gfx::DynamicTextureAtlas atlas(rendererBackend.getContext());

    // Every version still referenced by a layout needs its own region.
    constexpr uint32_t versions = 64;
    std::vector<gfx::ImageAtlas> atlases;
    for (uint32_t version = 0; version < versions; ++version) {
        ImageVersionMap versionMap;
        if (version) versionMap.emplace("marker", version);
        atlases.push_back(atlas.uploadIconsAndPatterns(makeIcon(static_cast<uint8_t>(version)), {}, versionMap));
        ASSERT_EQ(1u, atlases.back().textureHandles.size());
    }

    std::set<int32_t> ids;
    for (const auto& imageAtlas : atlases) {
        ids.insert(imageAtlas.textureHandles[0].getId());
    }
    EXPECT_EQ(versions, ids.size());

    for (auto& imageAtlas : atlases) {
        atlas.removeTextures(imageAtlas.textureHandles, imageAtlas.dynamicTexture);
    }
}
