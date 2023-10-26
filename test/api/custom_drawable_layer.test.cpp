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
        constexpr float extent = mbgl::util::EXTENT;

        // add polylines
        {
            using namespace mbgl;

            constexpr auto numLines = 6;
            Color colors[numLines]{Color::red(),
                                   Color::blue(),
                                   Color(1.f, 0.5f, 0, 0.5f),
                                   Color(1.f, 1.f, 0, 0.3f),
                                   Color::black(),
                                   Color(1.f, 0, 1.f, 0.2f)};
            float blurs[numLines]{0.0f, 4.0f, 16.0f, 2.0f, 0.5f, 24.0f};
            float opacities[numLines]{1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.5f};
            float gapWidths[numLines]{0.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f};
            float offsets[numLines]{0.0f, -1.0f, 2.0f, -2.0f, 0.5f, -5.0f};
            float widths[numLines]{8.0f, 4.0f, 16.0f, 2.0f, 0.5f, 24.0f};

            constexpr auto numPoints = 100;
            GeometryCoordinates polyline;
            for (auto ipoint{0}; ipoint < numPoints; ++ipoint) {
                polyline.emplace_back(ipoint * extent / numPoints,
                                      std::sin(ipoint * 2 * M_PI / numPoints) * extent / numLines / 2.f);
            }

            gfx::PolylineGeneratorOptions options;
            options.beginCap = style::LineCapType::Round;
            options.endCap = style::LineCapType::Round;
            options.joinType = style::LineJoinType::Round;

            for (auto index{0}; index < numLines; ++index) {
                for (auto& p : polyline) {
                    p.y += extent / numLines;
                }

                // set property values
                interface.setColor(colors[index]);
                interface.setBlur(blurs[index]);
                interface.setOpacity(opacities[index]);
                interface.setGapWidth(gapWidths[index]);
                interface.setOffset(offsets[index]);
                interface.setWidth(widths[index]);

                // add polyline
                interface.addPolyline(polyline, options);
            }
        }

        // add fill polygon
        {
            using namespace mbgl;

            GeometryCollection geometry{
                {
                    // ring 1
                    {static_cast<int16_t>(extent* 0.1f), static_cast<int16_t>(extent* 0.2f)},
                    {static_cast<int16_t>(extent* 0.5f), static_cast<int16_t>(extent* 0.5f)},
                    {static_cast<int16_t>(extent* 0.7f), static_cast<int16_t>(extent* 0.5f)},
                    {static_cast<int16_t>(extent* 0.5f), static_cast<int16_t>(extent* 1.0f)},
                    {static_cast<int16_t>(extent* 0.0f), static_cast<int16_t>(extent* 0.5f)},
                    {static_cast<int16_t>(extent* 0.1f), static_cast<int16_t>(extent* 0.2f)},
                },
                {
                    // ring 2
                    {static_cast<int16_t>(extent* 0.1f), static_cast<int16_t>(extent* 0.25f)},
                    {static_cast<int16_t>(extent* 0.15f), static_cast<int16_t>(extent* 0.5f)},
                    {static_cast<int16_t>(extent* 0.25f), static_cast<int16_t>(extent* 0.45f)},
                    {static_cast<int16_t>(extent* 0.1f), static_cast<int16_t>(extent* 0.25f)},
                },
            };

            // set properties
            interface.setColor(Color::green());
            interface.setOpacity(0.5f);

            // add fill
            interface.addFill(geometry);
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
