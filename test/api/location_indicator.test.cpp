#include <mln/test/util.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/map.hpp>
#include <mln/map/map_options.hpp>
#include <mln/storage/resource_options.hpp>
#include <mln/renderer/layers/render_location_indicator_layer.hpp>
#include <mln/style/layers/location_indicator_layer.hpp>
#include <mln/style/projection.hpp>
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
#include <vector>

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
        R"({"version":8,"projection":{"type":)" + (projection.starts_with('[') ? projection : '"' + projection + '"') +
        R"(},"sources":{},"layers":[{"id":"background","type":"background","paint":{"background-color":"white"}}]})");
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

// The black pixels of a rendered image, as an NDC box.
std::optional<gfx::RenderingStats::NDCBound> blackBound(const PremultipliedImage& image) {
    std::optional<gfx::RenderingStats::NDCBound> bound;
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
                if (!bound) {
                    bound = {.minX = left, .maxX = right, .minY = bottom, .maxY = top};
                }
                bound->minX = std::min(bound->minX, left);
                bound->maxX = std::max(bound->maxX, right);
                bound->minY = std::min(bound->minY, bottom);
                bound->maxY = std::max(bound->maxY, top);
            }
        }
    }
    return bound;
}

// Black on white: the WebGPU shader multiplies the image by the black the tweaker hands every textured quad.
void addBlackPuck(Map& map, const LatLng& location) {
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
}

struct Puck {
    /// The black pixels of the rendered image, as an NDC box.
    std::optional<gfx::RenderingStats::NDCBound> rendered;
    /// What the rendered feature capture reports.
    std::optional<gfx::RenderingStats::NDCBound> captured;
};

Puck renderPuck(const std::string& projection,
                const LatLng& location,
                const std::vector<LatLng>& cameraPath = {LatLng{0.0, 0.0}}) {
    util::RunLoop loop;

    HeadlessFrontend frontend{1};
    Map map(frontend,
            MapObserver::nullObserver(),
            MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()).withRenderedFeatureInfo(true),
            ResourceOptions().withCachePath(":memory:"));

    map.getStyle().loadJSON(
        R"({"version":8,"projection":{"type":)" + (projection.starts_with('[') ? projection : '"' + projection + '"') +
        R"(},"sources":{},"layers":[{"id":"background","type":"background","paint":{"background-color":"white"}}]})");
    for (const auto& center : cameraPath) {
        map.jumpTo(CameraOptions().withCenter(center).withZoom(1.0));
    }

    addBlackPuck(map, location);

    Puck result;
    result.rendered = blackBound(frontend.render(map).image);
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

// The puck lies flat on the sphere: the surface turning away narrows it, and nothing makes it larger than the
// 16 pixels its image has at the map's center.
TEST(LocationIndicator, GlobePuckKeepsItsSize) {
#ifndef MLN_DRAWABLE_LOCATION_INDICATOR
    GTEST_SKIP() << "OpenGL draws the location indicator with its own renderer, which has no globe path yet";
#endif
    const mln::JSValue emptyObject(rapidjson::kObjectType);
    style::conversion::Error error;
    if (!LayerManager::get()->createLayer("location-indicator", "probe", &emptyObject, error)) {
        GTEST_SKIP() << "no location-indicator layer on this platform";
    }

    constexpr double pixel = 2.0 / 256.0;
    for (const auto& location : {LatLng{0.0, 0.0},
                                 LatLng{0.0, 30.0},
                                 LatLng{0.0, 60.0},
                                 LatLng{40.0, 0.0},
                                 LatLng{60.0, 0.0},
                                 LatLng{-20.0, -45.0}}) {
        SCOPED_TRACE(testing::Message() << location.latitude() << ", " << location.longitude());
        const auto puck = renderPuck("globe", location);
        ASSERT_TRUE(puck.rendered);
        const double width = (puck.rendered->maxX - puck.rendered->minX) / pixel;
        const double height = (puck.rendered->maxY - puck.rendered->minY) / pixel;
        EXPECT_LE(width, 18.0);
        EXPECT_LE(height, 18.0);
        EXPECT_GE(std::max(width, height), 13.0);
    }

    // Mercator keeps the image's size at every latitude.
    const auto mercator = renderPuck("mercator", LatLng{35.0, 20.0});
    ASSERT_TRUE(mercator.rendered);
    EXPECT_NEAR((mercator.rendered->maxX - mercator.rendered->minX) / pixel, 16.0, 2.0);
    EXPECT_NEAR((mercator.rendered->maxY - mercator.rendered->minY) / pixel, 16.0, 2.0);
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
        const auto puck = renderPuck("globe", location);
        ASSERT_TRUE(puck.rendered);
        ASSERT_TRUE(puck.captured);
        EXPECT_NEAR(puck.captured->minX, puck.rendered->minX, twoPixels);
        EXPECT_NEAR(puck.captured->maxX, puck.rendered->maxX, twoPixels);
        EXPECT_NEAR(puck.captured->minY, puck.rendered->minY, twoPixels);
        EXPECT_NEAR(puck.captured->maxY, puck.rendered->maxY, twoPixels);
    }

    const auto hidden = renderPuck("globe", LatLng{0.0, 120.0});
    EXPECT_FALSE(hidden.rendered);
    EXPECT_FALSE(hidden.captured);
}

// Half way between Mercator and the globe the capture follows the blend the puck is drawn with.
TEST(LocationIndicator, PuckCapturedWhereItIsDrawnMidTransition) {
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
        const auto puck = renderPuck(R"(["mercator", "vertical-perspective", 0.5])", location);
        ASSERT_TRUE(puck.rendered);
        ASSERT_TRUE(puck.captured);
        EXPECT_NEAR(puck.captured->minX, puck.rendered->minX, twoPixels);
        EXPECT_NEAR(puck.captured->maxX, puck.rendered->maxX, twoPixels);
        EXPECT_NEAR(puck.captured->minY, puck.rendered->minY, twoPixels);
        EXPECT_NEAR(puck.captured->maxY, puck.rendered->maxY, twoPixels);
    }
}

// Past the antimeridian the globe keeps the center on the next world copy, here at 181 degrees; the puck's
// Mercator side has to be placed in that copy too, or the blend draws it half a world away.
TEST(LocationIndicator, PuckDrawnWhereItIsWithTheCenterPastTheAntimeridian) {
#ifndef MLN_DRAWABLE_LOCATION_INDICATOR
    GTEST_SKIP() << "OpenGL draws the location indicator with its own renderer, which has no globe path yet";
#endif
    const mln::JSValue emptyObject(rapidjson::kObjectType);
    style::conversion::Error error;
    if (!LayerManager::get()->createLayer("location-indicator", "probe", &emptyObject, error)) {
        GTEST_SKIP() << "no location-indicator layer on this platform";
    }
    constexpr double twoPixels = 2.0 * 2.0 / 256.0;
    const auto puck = renderPuck(R"(["mercator", "vertical-perspective", 0.5])",
                                 LatLng{0.0, 179.0},
                                 {LatLng{0.0, 0.0}, LatLng{0.0, 120.0}, LatLng{0.0, -179.0}});
    ASSERT_TRUE(puck.rendered);
    ASSERT_TRUE(puck.captured);
    EXPECT_NEAR(puck.captured->minX, puck.rendered->minX, twoPixels);
    EXPECT_NEAR(puck.captured->maxX, puck.rendered->maxX, twoPixels);
    EXPECT_NEAR(puck.captured->minY, puck.rendered->minY, twoPixels);
    EXPECT_NEAR(puck.captured->maxY, puck.rendered->maxY, twoPixels);
    // Two degrees west of the center, near the middle of the view.
    EXPECT_LT(std::abs((puck.rendered->minX + puck.rendered->maxX) / 2.0), 0.1);
    EXPECT_LT(std::abs((puck.rendered->minY + puck.rendered->maxY) / 2.0), 0.1);
}

// Changing the projection rebuilds the puck's drawables, which take its corners and image again without the camera
// or the puck changing.
TEST(LocationIndicator, PuckDrawnAfterTheProjectionChanges) {
#ifndef MLN_DRAWABLE_LOCATION_INDICATOR
    GTEST_SKIP() << "OpenGL draws the location indicator with its own renderer, which has no globe path yet";
#endif
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
    map.jumpTo(CameraOptions().withCenter(LatLng{0.0, 0.0}).withZoom(1.0));
    addBlackPuck(map, LatLng{20.0, 20.0});
    ASSERT_TRUE(blackBound(frontend.render(map).image));

    for (const auto* type : {"globe", "mercator", "globe"}) {
        SCOPED_TRACE(type);
        auto projection = std::make_unique<style::Projection>();
        projection->setType(ProjectionDefinition(type));
        map.getStyle().setProjection(std::move(projection));
        EXPECT_TRUE(blackBound(frontend.render(map).image));
    }
}

// At zoom 0 the bottom of the window is past the globe's edge, where every point stands for a point on the horizon:
// the hat and the shadow took the direction they lean in along the horizon there. Pitched, they lean straight up and
// down the screen from the puck.
TEST(LocationIndicator, GlobeHatAndShadowLeanUpAndDownTheScreenWithTheWindowBottomInSpace) {
#ifndef MLN_DRAWABLE_LOCATION_INDICATOR
    GTEST_SKIP() << "OpenGL draws the location indicator with its own renderer, which has no globe path yet";
#endif
    const mln::JSValue emptyObject(rapidjson::kObjectType);
    style::conversion::Error error;
    if (!LayerManager::get()->createLayer("location-indicator", "probe", &emptyObject, error)) {
        GTEST_SKIP() << "no location-indicator layer on this platform";
    }

    enum class Part {
        Bearing,
        Top,
        Shadow
    };
    // The center of the part's image in the rendered image, in NDC.
    const auto center = [](Part part) -> std::optional<Point<double>> {
        util::RunLoop loop;

        HeadlessFrontend frontend{1};
        Map map(frontend,
                MapObserver::nullObserver(),
                MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()),
                ResourceOptions().withCachePath(":memory:"));
        map.getStyle().loadJSON(
            R"({"version":8,"projection":{"type":"globe"},"sources":{},"layers":[{"id":"background","type":"background","paint":{"background-color":"white"}}]})");
        map.jumpTo(CameraOptions().withCenter(LatLng{0.0, 0.0}).withZoom(0.0).withPitch(20.0));

        PremultipliedImage black({16, 16});
        for (std::size_t i = 0; i < black.bytes(); i += 4) {
            black.data[i] = 0;
            black.data[i + 1] = 0;
            black.data[i + 2] = 0;
            black.data[i + 3] = 255;
        }
        map.getStyle().addImage(std::make_unique<style::Image>("image", std::move(black), 1.0f));

        auto puck = std::make_unique<LocationIndicatorLayer>("puck");
        puck->setLocation(std::array<double, 3>{{0.0, 40.0, 0.0}});
        puck->setImageTiltDisplacement(30.0f);
        switch (part) {
            case Part::Bearing:
                puck->setBearingImage(expression::Image("image"));
                puck->setBearingImageSize(1.0f);
                break;
            case Part::Top:
                puck->setTopImage(expression::Image("image"));
                puck->setTopImageSize(1.0f);
                break;
            case Part::Shadow:
                puck->setShadowImage(expression::Image("image"));
                puck->setShadowImageSize(1.0f);
                break;
        }
        map.getStyle().addLayer(std::move(puck));

        const auto drawn = blackBound(frontend.render(map).image);
        if (!drawn) {
            return std::nullopt;
        }
        return Point<double>{(drawn->minX + drawn->maxX) / 2.0, (drawn->minY + drawn->maxY) / 2.0};
    };

    constexpr double pixel = 2.0 / 256.0;
    const auto bearing = center(Part::Bearing);
    const auto top = center(Part::Top);
    const auto shadow = center(Part::Shadow);
    ASSERT_TRUE(bearing);
    ASSERT_TRUE(top);
    ASSERT_TRUE(shadow);
    EXPECT_NEAR(top->x, bearing->x, pixel);
    EXPECT_NEAR(shadow->x, bearing->x, pixel);
    EXPECT_GT(top->y, bearing->y + 2.0 * pixel);
    EXPECT_LT(shadow->y, bearing->y - 2.0 * pixel);
}
