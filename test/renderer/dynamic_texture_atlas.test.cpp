#include <mln/test/util.hpp>

#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/context.hpp>
#include <mln/gfx/dynamic_texture_atlas.hpp>
#include <mln/gfx/headless_backend.hpp>
#include <mln/style/image_impl.hpp>

using namespace mln;

namespace {

Immutable<Glyph> makeGlyph(char16_t code) {
    Glyph glyph;
    glyph.id = GlyphID(code);
    glyph.bitmap = AlphaImage({8, 8});
    return makeMutable<Glyph>(std::move(glyph));
}

Immutable<style::Image::Impl> makeImage(const std::string& id, bool sdf, Size size = {16, 16}) {
    return makeMutable<style::Image::Impl>(id, PremultipliedImage(size), 1.0f, sdf);
}

class DynamicTextureAtlasTest : public ::testing::Test {
protected:
    std::unique_ptr<gfx::HeadlessBackend> backend = gfx::HeadlessBackend::Create({64, 64});
    gfx::BackendScope scope{*backend->getRendererBackend()};
    gfx::DynamicTextureAtlas dynTexAtlas{backend->getRendererBackend()->getContext()};
    std::vector<gfx::ImageAtlas> atlases;

    const gfx::ImageAtlas& uploadPattern(const Immutable<style::Image::Impl>& image, int32_t version = 0) {
        atlases.push_back(dynTexAtlas.uploadIconsAndPatterns({}, {{image->id, image}}, {{image->id, version}}));
        return atlases.back();
    }

    void TearDown() override {
        for (const auto& imageAtlas : atlases) {
            dynTexAtlas.removeTextures(imageAtlas.textureHandles, imageAtlas.dynamicTexture);
        }
    }
};

} // namespace

TEST_F(DynamicTextureAtlasTest, SharesAllocationWithinImageRevision) {
    const auto image = makeImage("pattern", false);

    const auto firstAtlas = uploadPattern(image);
    EXPECT_EQ(firstAtlas.dynamicTexture->getVersion(firstAtlas.textureHandles.at(0).getId()), 0);

    const auto secondAtlas = uploadPattern(image, 1);

    EXPECT_EQ(firstAtlas.textureHandles.at(0).getId(), secondAtlas.textureHandles.at(0).getId());
    EXPECT_EQ(firstAtlas.textureHandles.at(0).getRectangle(), secondAtlas.textureHandles.at(0).getRectangle());

    EXPECT_EQ(secondAtlas.dynamicTexture->getVersion(secondAtlas.textureHandles.at(0).getId()), 1);
}

TEST_F(DynamicTextureAtlasTest, ReplacedSameImageGetsSameAllocation) {
    const auto original = uploadPattern(makeImage("pattern", false), 0).textureHandles.at(0);
    const auto sdf = uploadPattern(makeImage("pattern", true), 1).textureHandles.at(0);
    const auto toggledBack = uploadPattern(makeImage("pattern", false), 2).textureHandles.at(0);

    EXPECT_EQ(original.getRectangle(), sdf.getRectangle());
    EXPECT_EQ(original.getRectangle(), toggledBack.getRectangle());
    EXPECT_EQ(sdf.getRectangle(), toggledBack.getRectangle());
}

TEST_F(DynamicTextureAtlasTest, IconAndPatternUsageDoNotShareAllocation) {
    const auto icon = makeImage("icon", false);
    const auto pattern = makeImage("pattern", false);

    atlases.push_back(dynTexAtlas.uploadIconsAndPatterns({{icon->id, icon}}, {{pattern->id, pattern}}, {}));
    const auto& imageAtlas = atlases.back();

    ASSERT_EQ(2u, imageAtlas.textureHandles.size());
    EXPECT_NE(imageAtlas.textureHandles[0].getRectangle(), imageAtlas.textureHandles[1].getRectangle());
}

TEST_F(DynamicTextureAtlasTest, ReleasedRevisionGetsSameAllocation) {
    const auto image = makeImage("pattern", false);
    const auto imageAtlas = dynTexAtlas.uploadIconsAndPatterns({}, {{image->id, image}}, {});
    const auto releasedId = imageAtlas.textureHandles.at(0).getId();
    dynTexAtlas.removeTextures(imageAtlas.textureHandles, imageAtlas.dynamicTexture);

    EXPECT_EQ(releasedId, uploadPattern(image).textureHandles.at(0).getId());
}

TEST_F(DynamicTextureAtlasTest, GlyphsOfDifferentFontStacksDoNotShareAllocation) {
    // These font stack hashes and code points once gave both glyphs the same bin id, as real font stack hashes often
    // did in 32-bit builds, so one glyph drew as the other.
    const auto a = makeGlyph(u'A');
    const auto b = makeGlyph(u'B');
    const auto glyphAtlas = dynTexAtlas.uploadGlyphs({{0, {{b->id, b}}}, {4, {{a->id, a}}}});

    EXPECT_NE(glyphAtlas.glyphPositions.at(0).at(b->id).rect, glyphAtlas.glyphPositions.at(4).at(a->id).rect);
    dynTexAtlas.removeTextures(glyphAtlas.textureHandles, glyphAtlas.dynamicTexture);
}
