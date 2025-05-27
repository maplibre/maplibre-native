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

#include <mbgl/style/layers/custom_drawable_layer.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/io.hpp>

#include <memory>
#include <cmath>
#include <numbers>

using namespace std::numbers;

class LineTestDrawableLayer : public mbgl::style::CustomDrawableLayerHost {
public:
    void initialize() override {}

    void update(Interface& interface) override {
        // if we have built our drawable(s) already, either update or skip
        if (interface.getDrawableCount()) return;

        // set tile
        interface.setTileID({11, 327, 791});

        // add polylines
        {
            using namespace mbgl;

            constexpr auto numLines = 6;
            Interface::LineOptions options[numLines]{
                {
                    /*geometry=*/{},
                    /*blur=*/0.0f,
                    /*opacity=*/1.0f,
                    /*gapWidth=*/0.0f,
                    /*offset=*/0.0f,
                    /*width=*/8.0f,
                    /*color=*/Color::red(),
                },
                {
                    /*geometry=*/{},
                    /*blur=*/4.0f,
                    /*opacity=*/1.0f,
                    /*gapWidth=*/2.0f,
                    /*offset=*/-1.0f,
                    /*width=*/4.0f,
                    /*color=*/Color::blue(),
                },
                {
                    /*geometry=*/{},
                    /*blur=*/16.0f,
                    /*opacity=*/1.0f,
                    /*gapWidth=*/1.0f,
                    /*offset=*/2.0f,
                    /*width=*/16.0f,
                    /*color=*/Color(1.f, 0.5f, 0, 0.5f),
                },
                {
                    /*geometry=*/{},
                    /*blur=*/2.0f,
                    /*opacity=*/1.0f,
                    /*gapWidth=*/1.0f,
                    /*offset=*/-2.0f,
                    /*width=*/2.0f,
                    /*color=*/Color(1.f, 1.f, 0, 0.3f),
                },
                {
                    /*geometry=*/{},
                    /*blur=*/0.5f,
                    /*opacity=*/0.5f,
                    /*gapWidth=*/1.0f,
                    /*offset=*/0.5f,
                    /*width=*/0.5f,
                    /*color=*/Color::black(),
                },
                {
                    /*geometry=*/{},
                    /*blur=*/24.0f,
                    /*opacity=*/0.5f,
                    /*gapWidth=*/1.0f,
                    /*offset=*/-5.0f,
                    /*width=*/24.0f,
                    /*color=*/Color(1.f, 0, 1.f, 0.2f),
                },
            };
            for (auto& opt : options) {
                opt.geometry.beginCap = style::LineCapType::Round;
                opt.geometry.endCap = style::LineCapType::Round;
                opt.geometry.joinType = style::LineJoinType::Round;
            }

            constexpr auto numPoints = 100;
            GeometryCoordinates polyline;
            for (auto ipoint{0}; ipoint < numPoints; ++ipoint) {
                polyline.emplace_back(
                    ipoint * util::EXTENT / numPoints,
                    static_cast<int16_t>(std::sin(ipoint * 2 * pi / numPoints) * util::EXTENT / numLines / 2.f));
            }

            for (auto index{0}; index < numLines; ++index) {
                for (auto& p : polyline) {
                    p.y += util::EXTENT / numLines;
                }

                // set property values
                interface.setLineOptions(options[index]);

                // add polyline
                interface.addPolyline(polyline, Interface::LineShaderType::Classic);
            }
        }

        // finish
        interface.finish();
    }

    void deinitialize() override {}
};

class FillTestDrawableLayer : public mbgl::style::CustomDrawableLayerHost {
public:
    void initialize() override {}

    void update(Interface& interface) override {
        // if we have built our drawable(s) already, either update or skip
        if (interface.getDrawableCount()) return;

        // set tile
        interface.setTileID({11, 327, 791});

        // add fill polygon
        {
            using namespace mbgl;

            GeometryCollection geometry{
                {
                    // ring 1
                    {static_cast<int16_t>(util::EXTENT * 0.1f), static_cast<int16_t>(util::EXTENT * 0.2f)},
                    {static_cast<int16_t>(util::EXTENT * 0.5f), static_cast<int16_t>(util::EXTENT * 0.5f)},
                    {static_cast<int16_t>(util::EXTENT * 0.7f), static_cast<int16_t>(util::EXTENT * 0.5f)},
                    {static_cast<int16_t>(util::EXTENT * 0.5f), static_cast<int16_t>(util::EXTENT * 1.0f)},
                    {static_cast<int16_t>(util::EXTENT * 0.0f), static_cast<int16_t>(util::EXTENT * 0.5f)},
                    {static_cast<int16_t>(util::EXTENT * 0.1f), static_cast<int16_t>(util::EXTENT * 0.2f)},
                },
                {
                    // ring 2
                    {static_cast<int16_t>(util::EXTENT * 0.1f), static_cast<int16_t>(util::EXTENT * 0.25f)},
                    {static_cast<int16_t>(util::EXTENT * 0.15f), static_cast<int16_t>(util::EXTENT * 0.5f)},
                    {static_cast<int16_t>(util::EXTENT * 0.25f), static_cast<int16_t>(util::EXTENT * 0.45f)},
                    {static_cast<int16_t>(util::EXTENT * 0.1f), static_cast<int16_t>(util::EXTENT * 0.25f)},
                },
            };

            // set properties
            interface.setFillOptions({/*color=*/Color::green(), /*opacity=*/0.5f});

            // add fill
            interface.addFill(geometry);
        }

        // finish
        interface.finish();
    }

    void deinitialize() override {}
};

class SymbolIconTestDrawableLayer : public mbgl::style::CustomDrawableLayerHost {
public:
    void initialize() override {}

    void update(Interface& interface) override {
        // if we have built our drawable(s) already, either update or skip
        if (interface.getDrawableCount()) return;

        // set tile
        interface.setTileID({11, 327, 791});

        // add symbol icon
        {
            using namespace mbgl;
            GeometryCoordinate position{static_cast<int16_t>(util::EXTENT * 0.5f),
                                        static_cast<int16_t>(util::EXTENT * 0.5f)};

            // load image
            std::string imageData; // doh!
            constexpr auto imagePath = "test/fixtures/custom_drawable_layer/symbol_icon/pin1.png";
            try {
                imageData = util::read_file(imagePath);
            } catch (std::exception& ex) {
                using namespace std::string_literals;
                Log::Error(Event::Setup, "Failed to load expected image "s + imagePath + ": " + ex.what());
                throw;
            }
            std::shared_ptr<PremultipliedImage> image = std::make_shared<PremultipliedImage>(decodeImage(imageData));

            // set symbol options
            Interface::SymbolOptions options;
            options.texture = interface.context.createTexture2D();
            options.texture->setImage(image);
            options.texture->setSamplerConfiguration(
                {gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
            constexpr std::array<std::array<float, 2>, 2> textureCoordinates = {{{0.0f, 0.08f}, {1.0f, 0.9f}}};
            constexpr float xspan = textureCoordinates[1][0] - textureCoordinates[0][0];
            constexpr float yspan = textureCoordinates[1][1] - textureCoordinates[0][1];
            static_assert(xspan > 0.0f && yspan > 0.0f);
            options.size = {static_cast<uint32_t>(image->size.width * 0.2f * xspan),
                            static_cast<uint32_t>(image->size.height * 0.2f * yspan)};
            options.anchor = {0.5f, 0.95f};
            options.angleDegrees = 45.0f;
            options.scaleWithMap = false;
            options.pitchWithMap = false;
            interface.setSymbolOptions(options);

            // add symbol
            interface.addSymbol(position, textureCoordinates);
        }

        // finish
        interface.finish();
    }

    void deinitialize() override {}
};

TEST(CustomDrawableLayer, Line) {
    using namespace mbgl;
    using namespace mbgl::style;

    util::RunLoop loop;

    HeadlessFrontend frontend{1};
    Map map(frontend,
            MapObserver::nullObserver(),
            MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()),
            ResourceOptions().withCachePath(":memory:").withAssetPath("test/fixtures/api/assets"));

    // load style
    map.getStyle().loadJSON(util::read_file("test/fixtures/api/simple.json"));
    map.jumpTo(CameraOptions().withCenter(LatLng{37.8, -122.4426032}).withZoom(10.0));

    // add custom drawable layer
    map.getStyle().addLayer(
        std::make_unique<CustomDrawableLayer>("custom-drawable", std::make_unique<LineTestDrawableLayer>()));

    // render and test
    test::checkImage("test/fixtures/custom_drawable_layer/line", frontend.render(map).image, 0.000657, 0.1);
}

TEST(CustomDrawableLayer, Fill) {
    using namespace mbgl;
    using namespace mbgl::style;

    util::RunLoop loop;

    HeadlessFrontend frontend{1};
    Map map(frontend,
            MapObserver::nullObserver(),
            MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()),
            ResourceOptions().withCachePath(":memory:").withAssetPath("test/fixtures/api/assets"));

    // load style
    map.getStyle().loadJSON(util::read_file("test/fixtures/api/simple.json"));
    map.jumpTo(CameraOptions().withCenter(LatLng{37.8, -122.4426032}).withZoom(10.0));

    // add custom drawable layer
    map.getStyle().addLayer(
        std::make_unique<CustomDrawableLayer>("custom-drawable", std::make_unique<FillTestDrawableLayer>()));

    // render and test
    test::checkImage("test/fixtures/custom_drawable_layer/fill", frontend.render(map).image, 0.000657, 0.1);
}

TEST(CustomDrawableLayer, SymbolIcon) {
    using namespace mbgl;
    using namespace mbgl::style;

    util::RunLoop loop;

    HeadlessFrontend frontend{1};
    Map map(frontend,
            MapObserver::nullObserver(),
            MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize()),
            ResourceOptions().withCachePath(":memory:").withAssetPath("test/fixtures/api/assets"));

    // load style
    map.getStyle().loadJSON(util::read_file("test/fixtures/api/simple.json"));
    map.jumpTo(CameraOptions().withCenter(LatLng{37.8, -122.4426032}).withZoom(10.0));

    // add custom drawable layer
    map.getStyle().addLayer(
        std::make_unique<CustomDrawableLayer>("custom-drawable", std::make_unique<SymbolIconTestDrawableLayer>()));

    // render and test
    test::checkImage("test/fixtures/custom_drawable_layer/symbol_icon", frontend.render(map).image, 0.000657, 0.1);
}
