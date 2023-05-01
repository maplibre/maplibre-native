#include <mbgl/test/util.hpp>

#include <mbgl/style/image.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/exception.hpp>

using namespace mbgl;

TEST(StyleImage, ZeroWidth) {
    try {
        style::Image("test", PremultipliedImage({0, 16}), 2.0f);
        FAIL() << "Expected exception";
    } catch (util::StyleImageException& ex) {
        EXPECT_STREQ("dimensions may not be zero", ex.what());
    }
}

TEST(StyleImage, ZeroHeight) {
    try {
        style::Image("test", PremultipliedImage({16, 0}), 2.0f);
        FAIL() << "Expected exception";
    } catch (util::StyleImageException& ex) {
        EXPECT_STREQ("dimensions may not be zero", ex.what());
    }
}

TEST(StyleImage, ZeroRatio) {
    try {
        style::Image("test", PremultipliedImage({16, 16}), 0.0f);
        FAIL() << "Expected exception";
    } catch (util::StyleImageException& ex) {
        EXPECT_STREQ("pixelRatio may not be <= 0", ex.what());
    }
}

TEST(StyleImage, Retina) {
    style::Image image("test", PremultipliedImage({32, 24}), 2.0f);
    EXPECT_EQ(32u, image.getImage().size.width);
    EXPECT_EQ(24u, image.getImage().size.height);
    EXPECT_EQ(2, image.getPixelRatio());
}

TEST(StyleImage, FractionalRatio) {
    style::Image image("test", PremultipliedImage({20, 12}), 1.5f);
    EXPECT_EQ(20u, image.getImage().size.width);
    EXPECT_EQ(12u, image.getImage().size.height);
    EXPECT_EQ(1.5, image.getPixelRatio());
}

TEST(StyleImage, InvalidStretchX) {
    // out of left bound
    try {
        style::Image("test", PremultipliedImage({16, 16}), 1, {{-1.0f, 3.0f}});
        FAIL() << "Expected exception";
    } catch (util::StyleImageException& ex) {
        EXPECT_STREQ("stretchX is out of bounds or overlapping", ex.what());
    }

    // overlapping
    try {
        style::Image("test", PremultipliedImage({16, 16}), 1, {{0.0f, 3.0f}, {2.0f, 4.0f}});
        FAIL() << "Expected exception";
    } catch (util::StyleImageException& ex) {
        EXPECT_STREQ("stretchX is out of bounds or overlapping", ex.what());
    }
}

TEST(StyleImage, InvalidStretchY) {
    // out of bottom bound
    try {
        style::Image("test", PremultipliedImage({16, 16}), 1, {}, {{14.0f, 20.0f}});
        FAIL() << "Expected exception";
    } catch (util::StyleImageException& ex) {
        EXPECT_STREQ("stretchX is out of bounds or overlapping", ex.what());
    }

    // must be sorted
    try {
        style::Image("test", PremultipliedImage({16, 16}), 1, {}, {{4.0f, 8.0f}, {2.0f, 3.0f}});
        FAIL() << "Expected exception";
    } catch (util::StyleImageException& ex) {
        EXPECT_STREQ("stretchX is out of bounds or overlapping", ex.what());
    }
}

TEST(StyleImage, InvalidContent) {
    // bottom right out of bounds
    try {
        style::Image("test", PremultipliedImage({16, 16}), 1, {}, {}, style::ImageContent{0, 0, 24, 28});
        FAIL() << "Expected exception";
    } catch (util::StyleImageException& ex) {
        EXPECT_STREQ("content area is invalid", ex.what());
    }

    // bottom right < top left
    try {
        style::Image("test", PremultipliedImage({16, 16}), 1, {}, {}, style::ImageContent{14, 14, 12, 10});
        FAIL() << "Expected exception";
    } catch (util::StyleImageException& ex) {
        EXPECT_STREQ("content area is invalid", ex.what());
    }

    // top left out of bounds
    try {
        style::Image("test", PremultipliedImage({16, 16}), 1, {}, {}, style::ImageContent{-2, -8, 12, 10});
        FAIL() << "Expected exception";
    } catch (util::StyleImageException& ex) {
        EXPECT_STREQ("content area is invalid", ex.what());
    }
}

TEST(StyleImage, StretchContent) {
    style::Image image("test",
                       PremultipliedImage({16, 16}),
                       1,
                       {{2.0f, 14.0f}},
                       {{0.0f, 4.0f}, {12.0f, 16.0f}},
                       style::ImageContent{2, 2, 14, 14});
    EXPECT_EQ(16u, image.getImage().size.width);
    EXPECT_EQ(16u, image.getImage().size.height);
    EXPECT_EQ(1.0, image.getPixelRatio());
    EXPECT_EQ((style::ImageStretches{{2.0f, 14.0f}}), image.getStretchX());
    EXPECT_EQ((style::ImageStretches{{0.0f, 4.0f}, {12.0f, 16.0f}}), image.getStretchY());
    EXPECT_EQ((style::ImageContent{2, 2, 14, 14}), image.getContent());
}
