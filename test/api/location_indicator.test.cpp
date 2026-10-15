#include <mln/test/util.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/layermanager/layer_manager.hpp>
#include <mln/map/bound_options.hpp>
#include <mln/map/map.hpp>
#include <mln/map/map_options.hpp>
#include <mln/storage/resource_options.hpp>
#include <mln/style/conversion/json.hpp>
#include <mln/style/layers/location_indicator_layer.hpp>
#include <mln/style/rapidjson_conversion.hpp>
#include <mln/style/style.hpp>
#include <mln/util/constants.hpp>
#include <mln/util/image.hpp>
#include <mln/util/rapidjson.hpp>
#include <mln/util/run_loop.hpp>

#include <cmath>
#include <optional>

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

} // namespace

// Bounds keep the center unwrapped, here past the antimeridian at 181 degrees; the puck is drawn where it is.
TEST(LocationIndicator, PuckAtItsLocationWithTheCenterPastTheAntimeridian) {
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
        R"({"version":8,"sources":{},"layers":[{"id":"background","type":"background","paint":{"background-color":"white"}}]})");
    map.setBounds(BoundOptions().withLatLngBounds(LatLngBounds::hull({-10.0, 170.0}, {10.0, 190.0})));
    map.jumpTo(CameraOptions().withCenter(LatLng{0.0, 181.0}).withZoom(4.0));
    ASSERT_DOUBLE_EQ(181.0, map.getCameraOptions().center->longitude());

    const LatLng location{0.0, 179.0};
    auto puck = std::make_unique<LocationIndicatorLayer>("puck");
    puck->setLocation(std::array<double, 3>{{location.latitude(), location.longitude(), 0.0}});
    puck->setAccuracyRadius(50000.0f);
    puck->setAccuracyRadiusColor(Color::red());
    puck->setAccuracyRadiusBorderColor(Color::red());
    map.getStyle().addLayer(std::move(puck));

    const auto centre = redCentre(frontend.render(map).image);
    ASSERT_TRUE(centre.has_value()) << "no accuracy circle rendered";

    // 179 degrees is 2 degrees west of the center: 2 / 360 of the 512 * 2^4 pixel wide world at zoom 4.
    const double westOfCenter = 2.0 / 360.0 * util::tileSize_D * std::pow(2.0, 4.0);
    EXPECT_NEAR(centre->x, frontend.getSize().width / 2.0 - westOfCenter, 3.0);
    EXPECT_NEAR(centre->y, frontend.getSize().height / 2.0, 3.0);
}
