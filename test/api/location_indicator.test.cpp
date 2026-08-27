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
