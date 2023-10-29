#include <mbgl/test/util.hpp>

#include <mbgl/util/logging.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/io.hpp>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

#include <mapbox/pixelmatch.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace mbgl {
namespace test {

void checkImage(const std::string& base,
                const PremultipliedImage& actual,
                double imageThreshold,
                double pixelThreshold) {
#if !TEST_READ_ONLY
    if (getenv("UPDATE")) {
        util::write_file(base + "/expected.png", encodePNG(actual));
        return;
    }
#endif

    std::string expected_image;
    try {
        expected_image = util::read_file(base + "/expected.png");
    } catch (std::exception& ex) {
        Log::Error(Event::Setup, "Failed to load expected image " + base + "/expected.png" + ": " + ex.what());
        throw;
    }

    PremultipliedImage expected = decodeImage(expected_image);
    PremultipliedImage diff{expected.size};

#if !TEST_READ_ONLY
    util::write_file(base + "/actual.png", encodePNG(actual));
#endif

    ASSERT_EQ(expected.size, actual.size);

    uint64_t pixels = mapbox::pixelmatch(actual.data.get(),
                                         expected.data.get(),
                                         expected.size.width,
                                         expected.size.height,
                                         diff.data.get(),
                                         pixelThreshold);

    EXPECT_LE(static_cast<double>(pixels) / (expected.size.width * expected.size.height), imageThreshold);

#if !TEST_READ_ONLY
    util::write_file(base + "/diff.png", encodePNG(diff));
#endif
}

} // namespace test
} // namespace mbgl
