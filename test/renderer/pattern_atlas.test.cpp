#include <mln/test/util.hpp>
#include <mln/test/fixture_log_observer.hpp>
#include <mln/test/stub_style_observer.hpp>

#include <mln/renderer/pattern_atlas.hpp>
#include <mln/sprite/sprite_parser.hpp>
#include <mln/style/image_impl.hpp>
#include <mln/util/io.hpp>
#include <mln/util/image.hpp>
#include <mln/util/run_loop.hpp>
#include <mln/util/string.hpp>

#include <utility>

using namespace mln;

TEST(PatternAtlas, Basic) {
    FixtureLog log;
    PatternAtlas patternAtlas;

    auto images = parseSprite("default",
                              util::read_file("test/fixtures/annotations/emerald.png"),
                              util::read_file("test/fixtures/annotations/emerald.json"));
    for (auto& image : images) {
        if (image->id == "metro") {
            ASSERT_TRUE(patternAtlas.addPattern(*image));
        }
    }
    auto found = patternAtlas.getPattern("metro");
    ASSERT_TRUE(found);

    auto metro = *found;
    EXPECT_EQ(1, metro.tl()[0]);
    EXPECT_EQ(1, metro.tl()[1]);
    EXPECT_EQ(19, metro.br()[0]);
    EXPECT_EQ(19, metro.br()[1]);
    EXPECT_EQ(18, metro.displaySize()[0]);
    EXPECT_EQ(18, metro.displaySize()[1]);
    EXPECT_EQ(1.0f, metro.pixelRatio);
    EXPECT_EQ(patternAtlas.getPixelSize(), patternAtlas.getAtlasImageForTests().size);

    test::checkImage("test/fixtures/image_manager/basic", patternAtlas.getAtlasImageForTests());
}

TEST(PatternAtlas, Updates) {
    PatternAtlas patternAtlas;

    PremultipliedImage imageA({16, 12});
    imageA.fill(255);

    auto added = patternAtlas.addPattern(*makeMutable<style::Image::Impl>("one", std::move(imageA), 1.0f));
    ASSERT_TRUE(added);
    auto found = patternAtlas.getPattern("one");
    ASSERT_TRUE(found);
    EXPECT_EQ(added->paddedRect, found->paddedRect);

    auto a = *found;
    EXPECT_EQ(1, a.tl()[0]);
    EXPECT_EQ(1, a.tl()[1]);
    EXPECT_EQ(17, a.br()[0]);
    EXPECT_EQ(13, a.br()[1]);
    EXPECT_EQ(16, a.displaySize()[0]);
    EXPECT_EQ(12, a.displaySize()[1]);
    EXPECT_EQ(1.0f, a.pixelRatio);
    test::checkImage("test/fixtures/image_manager/updates_before", patternAtlas.getAtlasImageForTests());

    auto imageB = makeMutable<style::Image::Impl>("one", PremultipliedImage({5, 5}), 1.0f);
    EXPECT_FALSE(patternAtlas.addPattern(*imageB)); // Already added.

    patternAtlas.removePattern("one");
    ASSERT_FALSE(patternAtlas.getPattern("one"));
    EXPECT_TRUE(patternAtlas.addPattern(*imageB));

    auto b = *patternAtlas.getPattern("one");
    EXPECT_EQ(1, b.tl()[0]);
    EXPECT_EQ(1, b.tl()[1]);
    EXPECT_EQ(6, b.br()[0]);
    EXPECT_EQ(6, b.br()[1]);
    EXPECT_EQ(5, b.displaySize()[0]);
    EXPECT_EQ(5, b.displaySize()[1]);
    EXPECT_EQ(1.0f, b.pixelRatio);
    test::checkImage("test/fixtures/image_manager/updates_after", patternAtlas.getAtlasImageForTests());
}
