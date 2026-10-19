#include <gmock/gmock.h>

#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/stub_map_observer.hpp>
#include <mln/test/fake_file_source.hpp>
#include <mln/test/fixture_log_observer.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/headless_frontend.hpp>
#include <mln/gfx/shader_registry.hpp>
#include <mln/map/map_options.hpp>
#include <mln/math/log2.hpp>
#include <mln/renderer/renderer.hpp>
#include <mln/renderer/update_parameters.hpp>
#include <mln/storage/file_source_manager.hpp>
#include <mln/storage/main_resource_loader.hpp>
#include <mln/storage/network_status.hpp>
#include <mln/storage/online_file_source.hpp>
#include <mln/storage/resource_options.hpp>
#include <mln/style/expression/dsl.hpp>
#include <mln/style/image_impl.hpp>
#include <mln/style/image.hpp>
#include <mln/style/layers/background_layer.hpp>
#include <mln/style/layers/background_layer.hpp>
#include <mln/style/layers/fill_layer.hpp>
#include <mln/style/layers/raster_layer.hpp>
#include <mln/style/layers/symbol_layer.hpp>
#include <mln/style/sources/custom_geometry_source.hpp>
#include <mln/style/sources/geojson_source.hpp>
#include <mln/style/sources/image_source.hpp>
#include <mln/style/sources/vector_source.hpp>
#include <mln/style/style_impl.hpp>
#include <mln/style/style.hpp>
#include <mln/util/async_task.hpp>
#include <mln/util/client_options.hpp>
#include <mln/util/color.hpp>
#include <mln/util/image.hpp>
#include <mln/util/io.hpp>
#include <mln/util/logging.hpp>
#include <mln/util/run_loop.hpp>

#include <mln/layermanager/layer_manager.hpp>
#include <mln/plugin/plugin_layer_factory.hpp>
#include <mln/plugin/plugin_layer.hpp>
#include <mln/plugin/plugin_layer_impl.hpp>

#include <atomic>

using namespace mln;
using namespace mln::style;
using namespace std::literals::string_literals;

template <class FileSource = StubFileSource, class Frontend = HeadlessFrontend>
class MapTest {
public:
    util::RunLoop runLoop;
    std::shared_ptr<FileSource> fileSource;
    StubMapObserver observer;
    Frontend frontend;
    MapAdapter map;

    MapTest(float pixelRatio = 1, MapMode mode = MapMode::Static)
        : fileSource(std::make_shared<FileSource>(ResourceOptions::Default(), ClientOptions())),
          frontend(pixelRatio),
          map(frontend,
              observer,
              fileSource,
              MapOptions().withMapMode(mode).withSize(frontend.getSize()).withPixelRatio(pixelRatio)) {}

    explicit MapTest(MapOptions options)
        : fileSource(std::make_shared<FileSource>()),
          frontend(options.pixelRatio()),
          map(frontend, observer, fileSource, options.withSize(frontend.getSize())) {}

    template <typename T = FileSource>
    MapTest(const std::string& cachePath,
            const std::string& assetPath,
            float pixelRatio = 1,
            MapMode mode = MapMode::Static,
            typename std::enable_if_t<std::is_same_v<T, MainResourceLoader>>* = nullptr)
        : fileSource(std::make_shared<T>(ResourceOptions().withCachePath(cachePath).withAssetPath(assetPath),
                                         ClientOptions())),
          frontend(pixelRatio),
          map(frontend,
              observer,
              fileSource,
              MapOptions().withMapMode(mode).withSize(frontend.getSize()).withPixelRatio(pixelRatio)) {}

    template <typename T = FileSource>
    MapTest(const ResourceOptions& resourceOptions,
            const ClientOptions& clientOptions = ClientOptions(),
            float pixelRatio = 1,
            MapMode mode = MapMode::Static)
        : fileSource(std::make_shared<T>(resourceOptions, clientOptions)),
          frontend(pixelRatio),
          map(frontend,
              observer,
              fileSource,
              MapOptions().withMapMode(mode).withSize(frontend.getSize()).withPixelRatio(pixelRatio)) {}
};

TEST(Plugin, PluginLayerProperty) {
    bool _singleFloatValid = false;
    bool _colorValid = false;

    std::cout << "Test: Plugin.PluginLayerProperty\n";

    {
        PluginLayerProperty p;
        p._propertyType = PluginLayerProperty::PropertyType::SingleFloat;
        p._singleFloatValue = 1.0;
        auto json = p.asJSON();
        std::string testValue = R"("":1.000000)";
        std::cout << "   _singleFloatValid: " << json << ";|" << testValue << "|\n";
        _singleFloatValid = json == testValue;
        EXPECT_STREQ(testValue.c_str(), json.c_str());
    }

    {
        PluginLayerProperty p;
        p._propertyType = PluginLayerProperty::PropertyType::Color;
        p._dataDrivenColorValue = Color(1, 1, 1, 1);
        auto json = p.asJSON();
        std::string testValue = R"JSON("":"rgba(255,255,255,1)")JSON";
        std::cout << "   _colorValid: |" << json << "|;|" << testValue << "|\n";
        _colorValid = json == testValue;
        EXPECT_STREQ(testValue.c_str(), json.c_str());
    }

    ASSERT_TRUE(_colorValid);
    ASSERT_TRUE(_singleFloatValid);
}

TEST(Plugin, PluginLayerPropertyManager) {
    PluginLayerPropertyManager pm;
    PluginLayerProperty* p1 = new PluginLayerProperty();
    p1->_propertyType = PluginLayerProperty::PropertyType::SingleFloat;
    p1->_propertyName = "float";
    p1->_singleFloatValue = 1.0;
    pm.addProperty(p1);

    PluginLayerProperty* p2 = new PluginLayerProperty();
    p2->_propertyType = PluginLayerProperty::PropertyType::Color;
    p2->_propertyName = "color";
    p2->_dataDrivenColorValue = Color(1, 1, 1, 1);
    pm.addProperty(p2);

    auto json = pm.propertiesAsJSON();

    std::cout << "Test: Plugin.PluginLayerPropertyManager\n";
    std::string testValue = R"JSON({"color":"rgba(255,255,255,1)", "float":1.000000})JSON";
    std::cout << "   JSON: |" << json << "|;|" << testValue << "|\n";
    EXPECT_STREQ(testValue.c_str(), json.c_str());

    bool _jsonValid = (json == testValue);
    ASSERT_TRUE(_jsonValid);
}

TEST(Plugin, PluginLayer) {
    bool _layerCreated = false;
    bool _initialPropertiesFound = false;
    bool _layerRendered = false;
    bool _paintPropertiesFound = false;

    // Create the plug-in layer type
    std::string layerType = "plugin-layer-test";
    auto pluginLayerFactory = std::make_unique<PluginLayerFactory>(
        layerType,
        mln::style::LayerTypeInfo::Source::NotRequired,
        mln::style::LayerTypeInfo::Pass3D::NotRequired,
        mln::style::LayerTypeInfo::Layout::NotRequired,
        mln::style::LayerTypeInfo::FadingTiles::NotRequired,
        mln::style::LayerTypeInfo::CrossTileIndex::NotRequired,
        mln::style::LayerTypeInfo::TileKind::NotRequired);
    pluginLayerFactory->setOnLayerCreatedEvent(
        [&_layerRendered, &_layerCreated, &_initialPropertiesFound, &_paintPropertiesFound](
            mln::style::PluginLayer* pluginLayer) {
            //            std::cout << "On Layer Created\n";
            _layerCreated = true;

            auto pluginLayerImpl = (mln::style::PluginLayer::Impl*)pluginLayer->baseImpl.get();
            auto& pm = pluginLayerImpl->_propertyManager;
            PluginLayerProperty* p = new PluginLayerProperty();
            p->_propertyName = "scale";
            p->_propertyType = PluginLayerProperty::PropertyType::SingleFloat;
            p->_defaultSingleFloatValue = 1.0;
            pm.addProperty(p);
            pluginLayerImpl->setRenderFunction([&_layerRendered]([[maybe_unused]] PaintParameters& paintParameters) {
                //                std::cout << "On Layer Rendered\n";
                //                std::cout << "  Paint Properties.currentLayer: " << paintParameters.currentLayer <<
                //                "\n";
                _layerRendered = true;
            });

            pluginLayerImpl->setUpdatePropertiesFunction(
                [&_initialPropertiesFound, &_paintPropertiesFound](const std::string& properties) {
                    //                    std::cout << "On Layer Update Properties\n";
                    //                    std::cout << "  -> " << properties << "\n";

                    auto propertiesCheck = R"({"custom-property1":"custom-property-value1"})";
                    if (properties == propertiesCheck) {
                        _initialPropertiesFound = true;
                    }
                    if (properties.find("scale") > 0) {
                        _paintPropertiesFound = true;
                    }
                });
        });

    auto lm = LayerManager::get();
    lm->addLayerTypeCoreOnly(std::move(pluginLayerFactory));

    MapTest<> test{1, MapMode::Continuous};
    test.map.getStyle().loadJSON(R"STYLE({
      "version": 8,
      "layers": [{
        "id": "plugin-layer-1",
        "type": "plugin-layer-test",
        "properties": {
            "custom-property1":"custom-property-value1"
        },
        "paint": {"scale": 1.0 }
      }]
    })STYLE");

    for (int i = 0; i < 10; i++) {
        test.runLoop.runOnce();
    }

    ASSERT_TRUE(_layerCreated);
    ASSERT_TRUE(_initialPropertiesFound);
    ASSERT_TRUE(_layerRendered);
    ASSERT_TRUE(_paintPropertiesFound);
}
