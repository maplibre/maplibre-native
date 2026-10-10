#include <mln/test/util.hpp>

#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/dynamic_texture_atlas.hpp>
#include <mln/gfx/headless_backend.hpp>
#include <mln/style/image_impl.hpp>
#include <mln/util/image.hpp>

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

    atlas.removeTextures(original.textureHandles, original.dynamicTexture);
    atlas.removeTextures(shared.textureHandles, shared.dynamicTexture);
    atlas.removeTextures(updated.textureHandles, updated.dynamicTexture);
}
