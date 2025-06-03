#include <mbgl/test/util.hpp>

#include <mbgl/util/logging.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/io.hpp>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

#include <gmock/gmock-matchers.h>
#include <gmock/gmock-more-matchers.h>
#include <mapbox/pixelmatch.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

using namespace ::testing;

namespace mbgl {
namespace test {

void checkImage(const std::string& base,
                const PremultipliedImage& actual,
                double imageThreshold,
                double pixelThreshold) {
    EXPECT_LE(getImageDiff(base, actual, pixelThreshold), imageThreshold);
}

void checkImages(const std::vector<std::string>& possibleExpected,
                 const PremultipliedImage& actual,
                 double imageThreshold,
                 double pixelThreshold) {
    std::vector<double> diffs(0.0, possibleExpected.size());

    for (const auto& expected : possibleExpected) {
        diffs.push_back(getImageDiff(expected, actual, pixelThreshold));
    }

    EXPECT_THAT(diffs, Contains(Le(imageThreshold)));
}

double getImageDiff(const std::string& base, const PremultipliedImage& actual, double pixelThreshold) {
#if !TEST_READ_ONLY
    if (getenv("UPDATE")) {
        util::write_file(base + "/expected.png", encodePNG(actual));
        return 0.0;
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

    if (expected.size != actual.size) {
        return 1.0;
    }

    uint64_t pixels = mapbox::pixelmatch(actual.data.get(),
                                         expected.data.get(),
                                         expected.size.width,
                                         expected.size.height,
                                         diff.data.get(),
                                         pixelThreshold);

#if !TEST_READ_ONLY
    util::write_file(base + "/diff.png", encodePNG(diff));
#endif

    return static_cast<double>(pixels) / (expected.size.width * expected.size.height);
}

} // namespace test
} // namespace mbgl
