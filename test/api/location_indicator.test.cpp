#include <mln/test/util.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/map.hpp>
#include <mln/map/map_options.hpp>
#include <mln/storage/resource_options.hpp>
#include <mln/renderer/layers/render_location_indicator_layer.hpp>
#include <mln/style/layers/location_indicator_layer.hpp>
#include <mln/style/style.hpp>
#include <mln/util/geo.hpp>
#include <mln/util/image.hpp>
#include <mln/layermanager/layer_manager.hpp>
#include <mln/style/conversion/json.hpp>
#include <mln/style/rapidjson_conversion.hpp>
#include <mln/util/rapidjson.hpp>
#include <mln/util/run_loop.hpp>

#include <cmath>
#include <optional>
#include <string>

using namespace mln;
using namespace mln::style;

namespace {

// The centre of the accuracy circle in the rendered image: the mean of its red pixels.
std::optional<ScreenCoordinate> redCentre(const PremultipliedImage& image) {
    double x = 0;
    double y = 0;
    std::size_t count = 0;
    for (uint32_t row = 0; row < image.size.height; ++row) {
        for (uint32_t col = 0; col < image.size.width; ++col) {
            const auto* pixel = image.data.get() + (row * image.size.width + col) * 4;
            if (pixel[0] > 200 && pixel[1] < 50 && pixel[2] < 50) {
                x += col;
                y += row;
                ++count;
            }
        }
    }
    if (count == 0) {
        return std::nullopt;
    }
    return ScreenCoordinate{x / static_cast<double>(count), y / static_cast<double>(count)};
}

void expectPuckAtItsLocation(const std::string& projection) {
    // The Darwin layer manager does not register the location indicator; the SDKs draw their own puck.
    const mln::JSValue emptyObject(rapidjson::kObjectType);
    style::conversion::Error error;
    if (!LayerManager::get()->createLayer("location-indicator", "probe", &emptyObject, error)) {
        GTEST_SKIP() << "no location-indicator layer on this platform";
    }

    util::RunLoop loop;

    HeadlessFrontend frontend{1};
    Map map(frontend,
            MapObserver::nullObserver(),
            MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()),
            ResourceOptions().withCachePath(":memory:"));

    map.getStyle().loadJSON(
        R"({"version":8,"projection":{"type":")" + projection +
        R"("},"sources":{},"layers":[{"id":"background","type":"background","paint":{"background-color":"white"}}]})");
    map.jumpTo(CameraOptions().withCenter(LatLng{0.0, 0.0}).withZoom(1.0));

    const LatLng location{20.0, 20.0};
    auto puck = std::make_unique<LocationIndicatorLayer>("puck");
    puck->setLocation(std::array<double, 3>{{location.latitude(), location.longitude(), 0.0}});
    puck->setAccuracyRadius(1000000.0f);
    puck->setAccuracyRadiusColor(Color::red());
    puck->setAccuracyRadiusBorderColor(Color::red());
    map.getStyle().addLayer(std::move(puck));

    const auto image = frontend.render(map).image;
    const auto centre = redCentre(image);
    ASSERT_TRUE(centre.has_value()) << projection << ": no accuracy circle rendered";

    const auto expected = map.pixelForLatLng(location);
    EXPECT_NEAR(centre->x, expected.x, 3.0) << projection;
    EXPECT_NEAR(centre->y, expected.y, 3.0) << projection;
}

struct Puck {
    /// The black pixels of the rendered image, as an NDC box.
    std::optional<gfx::RenderingStats::NDCBound> rendered;
    /// What the rendered feature capture reports.
    std::optional<gfx::RenderingStats::NDCBound> captured;
};

Puck renderPuckOnGlobe(const LatLng& location) {
    util::RunLoop loop;

    HeadlessFrontend frontend{1};
    Map map(frontend,
            MapObserver::nullObserver(),
            MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()).withRenderedFeatureInfo(true),
            ResourceOptions().withCachePath(":memory:"));

    map.getStyle().loadJSON(
        R"({"version":8,"projection":{"type":"globe"},"sources":{},)"
        R"("layers":[{"id":"background","type":"background","paint":{"background-color":"white"}}]})");
    map.jumpTo(CameraOptions().withCenter(LatLng{0.0, 0.0}).withZoom(1.0));

    // Black on white: the WebGPU shader multiplies the image by the black the tweaker hands every textured quad.
    PremultipliedImage black({16, 16});
    for (std::size_t i = 0; i < black.bytes(); i += 4) {
        black.data[i] = 0;
        black.data[i + 1] = 0;
        black.data[i + 2] = 0;
        black.data[i + 3] = 255;
    }
    map.getStyle().addImage(std::make_unique<style::Image>("puck", std::move(black), 1.0f));

    auto puck = std::make_unique<LocationIndicatorLayer>("puck");
    puck->setLocation(std::array<double, 3>{{location.latitude(), location.longitude(), 0.0}});
    puck->setBearingImage(expression::Image("puck"));
    puck->setBearingImageSize(1.0f);
    map.getStyle().addLayer(std::move(puck));

    // The puck's image is uploaded with the first frame and drawn from the next.
    frontend.render(map);
    const auto image = frontend.render(map).image;

    Puck result;
    const double width = image.size.width;
    const double height = image.size.height;
    for (uint32_t row = 0; row < image.size.height; ++row) {
        for (uint32_t col = 0; col < image.size.width; ++col) {
            const auto* pixel = image.data.get() + (row * image.size.width + col) * 4;
            // Opaque black; the space around the globe is transparent.
            if (pixel[0] < 50 && pixel[1] < 50 && pixel[2] < 50 && pixel[3] > 200) {
                const double left = col / width * 2.0 - 1.0;
                const double right = (col + 1) / width * 2.0 - 1.0;
                const double top = 1.0 - row / height * 2.0;
                const double bottom = 1.0 - (row + 1) / height * 2.0;
                if (!result.rendered) {
                    result.rendered = {.minX = left, .maxX = right, .minY = bottom, .maxY = top};
                }
                result.rendered->minX = std::min(result.rendered->minX, left);
                result.rendered->maxX = std::max(result.rendered->maxX, right);
                result.rendered->minY = std::min(result.rendered->minY, bottom);
                result.rendered->maxY = std::max(result.rendered->maxY, top);
            }
        }
    }
    map.getRenderedFeatures(
        "maplibre:LocationIndicator", std::nullopt, std::nullopt, [&](const auto&, const auto& info) -> bool {
            result.captured = info.ndcBound;
            return true;
        });
    return result;
}

} // namespace

TEST(LocationIndicator, MercatorPuckAtItsLocation) {
    expectPuckAtItsLocation("mercator");
}

TEST(LocationIndicator, GlobePuckAtItsLocation) {
#ifndef MLN_DRAWABLE_LOCATION_INDICATOR
    GTEST_SKIP() << "OpenGL draws the location indicator with its own renderer, which has no globe path yet";
#endif
    expectPuckAtItsLocation("globe");
}

// The capture reports the puck where the globe draws it, and nothing for a puck behind the horizon.
TEST(LocationIndicator, GlobePuckCapturedWhereItIsDrawn) {
#ifndef MLN_DRAWABLE_LOCATION_INDICATOR
    GTEST_SKIP() << "OpenGL draws the location indicator with its own renderer, which has no globe path yet";
#endif
    const mln::JSValue emptyObject(rapidjson::kObjectType);
    style::conversion::Error error;
    if (!LayerManager::get()->createLayer("location-indicator", "probe", &emptyObject, error)) {
        GTEST_SKIP() << "no location-indicator layer on this platform";
    }

    constexpr double twoPixels = 2.0 * 2.0 / 256.0;
    for (const auto& location : {LatLng{0.0, 0.0}, LatLng{40.0, 0.0}, LatLng{0.0, 30.0}, LatLng{-20.0, -45.0}}) {
        SCOPED_TRACE(testing::Message() << location.latitude() << ", " << location.longitude());
        const auto puck = renderPuckOnGlobe(location);
        ASSERT_TRUE(puck.rendered);
        ASSERT_TRUE(puck.captured);
        EXPECT_NEAR(puck.captured->minX, puck.rendered->minX, twoPixels);
        EXPECT_NEAR(puck.captured->maxX, puck.rendered->maxX, twoPixels);
        EXPECT_NEAR(puck.captured->minY, puck.rendered->minY, twoPixels);
        EXPECT_NEAR(puck.captured->maxY, puck.rendered->maxY, twoPixels);
    }

    const auto hidden = renderPuckOnGlobe(LatLng{0.0, 120.0});
    EXPECT_FALSE(hidden.rendered);
    EXPECT_FALSE(hidden.captured);
}
