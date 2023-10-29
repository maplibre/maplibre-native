#include <mbgl/test/util.hpp>

#include <mbgl/gfx/headless_frontend.hpp>
#include <mbgl/map/map.hpp>
#include <mbgl/map/map_options.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/style/layers/fill_layer.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/util/io.hpp>
#include <mbgl/util/mat4.hpp>
#include <mbgl/util/run_loop.hpp>

#if MLN_DRAWABLE_RENDERER

#include <mbgl/style/layers/custom_drawable_layer.hpp>
#include <mbgl/util/constants.hpp>

#include <memory>
#include <cmath>

class TestDrawableLayer : public mbgl::style::CustomDrawableLayerHost {
public:
    void initialize() override {}

    void update(Interface& interface) override {
        // if we have built our drawable(s) already, either update or skip
        if (interface.getDrawableCount()) return;

        // set tile
        interface.setTileID({11, 327, 791});

        // add polylines
        using namespace mbgl;

        constexpr auto numLines = 6;
        Interface::LineOptions options[numLines]{
            {/*color=*/Color::red(),
             /*blur=*/0.0f,
             /*opacity=*/1.0f,
             /*gapWidth=*/0.0f,
             /*offset=*/0.0f,
             /*width=*/8.0f,
             {}},
            {/*color=*/Color::blue(),
             /*blur=*/4.0f,
             /*opacity=*/1.0f,
             /*gapWidth=*/2.0f,
             /*offset=*/-1.0f,
             /*width=*/4.0f,
             {}},
            {/*color=*/Color(1.f, 0.5f, 0, 0.5f),
             /*blur=*/16.0f,
             /*opacity=*/1.0f,
             /*gapWidth=*/1.0f,
             /*offset=*/2.0f,
             /*width=*/16.0f,
             {}},
            {/*color=*/Color(1.f, 1.f, 0, 0.3f),
             /*blur=*/2.0f,
             /*opacity=*/1.0f,
             /*gapWidth=*/1.0f,
             /*offset=*/-2.0f,
             /*width=*/2.0f,
             {}},
            {/*color=*/Color::black(),
             /*blur=*/0.5f,
             /*opacity=*/0.5f,
             /*gapWidth=*/1.0f,
             /*offset=*/0.5f,
             /*width=*/0.5f,
             {}},
            {/*color=*/Color(1.f, 0, 1.f, 0.2f),
             /*blur=*/24.0f,
             /*opacity=*/0.5f,
             /*gapWidth=*/1.0f,
             /*offset=*/-5.0f,
             /*width=*/24.0f,
             {}},
        };
        for (auto& opt : options) {
            opt.geometry.beginCap = style::LineCapType::Round;
            opt.geometry.endCap = style::LineCapType::Round;
            opt.geometry.joinType = style::LineJoinType::Round;
        }

        constexpr auto numPoints = 100;
        GeometryCoordinates polyline;
        for (auto ipoint{0}; ipoint < numPoints; ++ipoint) {
            polyline.emplace_back(ipoint * util::EXTENT / numPoints,
                                  std::sin(ipoint * 2 * M_PI / numPoints) * util::EXTENT / numLines / 2.f);
        }

        for (auto index{0}; index < numLines; ++index) {
            for (auto& p : polyline) {
                p.y += util::EXTENT / numLines;
            }

            // set property values
            interface.setLineOptions(options[index]);

            // add polyline
            interface.addPolyline(polyline);
        }

        // finish
        interface.finish();
    }

    void deinitialize() override {}
};

TEST(CustomDrawableLayer, Basic) {
    using namespace mbgl;
    using namespace mbgl::style;

    util::RunLoop loop;

    HeadlessFrontend frontend{1};
    Map map(frontend,
            MapObserver::nullObserver(),
            MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()),
            ResourceOptions().withCachePath(":memory:").withAssetPath("test/fixtures/api/assets"));

    // load style
    map.getStyle().loadJSON(util::read_file("test/fixtures/api/water.json"));
    map.jumpTo(CameraOptions().withCenter(LatLng{37.8, -122.4426032}).withZoom(10.0));

    // add fill layer
    auto layer = std::make_unique<FillLayer>("landcover", "mapbox");
    layer->setSourceLayer("landcover");
    layer->setFillColor(Color{1.0, 1.0, 0.0, 1.0});
    map.getStyle().addLayer(std::move(layer));

    // add custom drawable layer
    map.getStyle().addLayer(
        std::make_unique<CustomDrawableLayer>("custom-drawable", std::make_unique<TestDrawableLayer>()));

    // render and test
    test::checkImage("test/fixtures/custom_drawable_layer/basic", frontend.render(map).image, 0.0006, 0.1);
}

#endif // MLN_DRAWABLE_RENDERER
