#include <mln/test/util.hpp>

#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/context.hpp>
#include <mln/gfx/dynamic_texture_atlas.hpp>
#include <mln/gfx/headless_backend.hpp>
#include <mln/style/image_impl.hpp>

using namespace mln;

namespace {

Immutable<style::Image::Impl> makeImage(const std::string& id, bool sdf, Size size = {16, 16}) {
    return makeMutable<style::Image::Impl>(id, PremultipliedImage(size), 1.0f, sdf);
}

class DynamicTextureAtlasTest : public ::testing::Test {
protected:
    std::unique_ptr<gfx::HeadlessBackend> backend = gfx::HeadlessBackend::Create({64, 64});
    gfx::BackendScope scope{*backend->getRendererBackend()};
    gfx::DynamicTextureAtlas atlas{backend->getRendererBackend()->getContext()};
    std::vector<gfx::ImageAtlas> atlases;

    const gfx::ImageAtlas& uploadPattern(const Immutable<style::Image::Impl>& image) {
        atlases.push_back(atlas.uploadIconsAndPatterns({}, {{image->id, image}}, {}));
        return atlases.back();
    }

    void TearDown() override {
        for (const auto& imageAtlas : atlases) {
            atlas.removeTextures(imageAtlas.textureHandles, imageAtlas.dynamicTexture);
        }
    }
};

} // namespace

TEST_F(DynamicTextureAtlasTest, SharesAllocationWithinImageRevision) {
    const auto image = makeImage("pattern", false);

    const auto first = uploadPattern(image).textureHandles.at(0);
    const auto second = uploadPattern(image).textureHandles.at(0);

    EXPECT_EQ(first.getId(), second.getId());
    EXPECT_EQ(first.getRectangle(), second.getRectangle());
}

TEST_F(DynamicTextureAtlasTest, ReplacedSameSizeImageGetsFreshAllocation) {
    // Each revision keeps its layout alive, as a tile still rendering an older layout would.
    const auto original = uploadPattern(makeImage("pattern", false)).textureHandles.at(0);
    const auto sdf = uploadPattern(makeImage("pattern", true)).textureHandles.at(0);
    const auto toggledBack = uploadPattern(makeImage("pattern", false)).textureHandles.at(0);

    EXPECT_NE(original.getRectangle(), sdf.getRectangle());
    EXPECT_NE(original.getRectangle(), toggledBack.getRectangle());
    EXPECT_NE(sdf.getRectangle(), toggledBack.getRectangle());
}

TEST_F(DynamicTextureAtlasTest, IconAndPatternUsageDoNotShareAllocation) {
    const auto image = makeImage("image", false);

    atlases.push_back(atlas.uploadIconsAndPatterns({{image->id, image}}, {{image->id, image}}, {}));
    const auto& imageAtlas = atlases.back();

    ASSERT_EQ(2u, imageAtlas.textureHandles.size());
    EXPECT_NE(imageAtlas.textureHandles[0].getRectangle(), imageAtlas.textureHandles[1].getRectangle());
}

TEST_F(DynamicTextureAtlasTest, ReleasedRevisionGetsFreshAllocation) {
    const auto image = makeImage("pattern", false);
    const auto released = atlas.uploadIconsAndPatterns({}, {{image->id, image}}, {});
    const auto releasedId = released.textureHandles.at(0).getId();
    atlas.removeTextures(released.textureHandles, released.dynamicTexture);

    // The revision was forgotten on its last release, so it is packed and uploaded again under a new id.
    EXPECT_NE(releasedId, uploadPattern(image).textureHandles.at(0).getId());
}
