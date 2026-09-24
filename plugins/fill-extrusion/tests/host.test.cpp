#include <fill_extrusion.hpp>
#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/map_options.hpp>
#include <mln/renderer/renderer.hpp>
#include <mln/layermanager/layer_manager.hpp>
#include <mln/plugin/plugin_registry.hpp>
#include <mln/style/style.hpp>
#include <mln/style/layers/fill_extrusion_layer.hpp>
#include <mln/style/layers/plugin_style_layer.hpp>
#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/layer.hpp>
#include <mln/style/rapidjson_conversion.hpp>
#include <mln/style/light.hpp>
#include <mln/test/map_adapter.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/util/run_loop.hpp>
#include <gtest/gtest.h>

using namespace mln;
namespace {
const mln_plugin_descriptor_v1* registeredDescriptor;
mln_plugin_status captureDescriptor(const mln_plugin_descriptor_v1* d, char*, size_t) {
    registeredDescriptor = d;
    return MLN_PLUGIN_STATUS_OK;
}
const char* styleJSON = R"({"version":8,"center":[0,0],"zoom":15.5,"pitch":55,"bearing":25,
"sources":{"s":{"type":"geojson","data":{"type":"FeatureCollection","features":[
{"type":"Feature","id":1,"properties":{"h":100},"geometry":{"type":"Polygon","coordinates":[[[-0.002,-0.002],[0.002,-0.002],[0.002,0.002],[-0.002,0.002],[-0.002,-0.002]],[[-0.001,-0.001],[-0.001,0.001],[0.001,0.001],[0.001,-0.001],[-0.001,-0.001]]]}},
{"type":"Feature","id":2,"properties":{"h":60},"geometry":{"type":"Polygon","coordinates":[[[0,-0.002],[0.004,-0.002],[0.004,0.001],[0,0.001],[0,-0.002]]]}}
]}}},"layers":[{"id":"background","type":"background","paint":{"background-color":"#bcd"}}]})";
struct Scene {
    std::shared_ptr<StubFileSource> source = std::make_shared<StubFileSource>();
    MapObserver observer;
    HeadlessFrontend frontend{Size{256, 256}, 1};
    MapAdapter map{frontend, observer, source, MapOptions().withMapMode(MapMode::Static).withSize({256, 256})};
    style::Layer* layer = nullptr;
    Scene(bool plugin) {
        map.getStyle().loadJSON(styleJSON);
        std::unique_ptr<style::Layer> extrusion;
        if (plugin) {
            style::conversion::Error error;
            auto parsed = style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
                R"({"id":"extrusion","source":"s","type":"fill-extrusion"})", error);
            EXPECT_TRUE(parsed) << error.message;
            extrusion = std::move(*parsed);
            EXPECT_EQ(extrusion->getTypeInfo(), &plugin::PluginRegistry::get().findLayerType("fill-extrusion")->info);
        } else
            extrusion = std::make_unique<style::FillExtrusionLayer>("extrusion", "s");
        layer = extrusion.get();
        map.getStyle().addLayer(std::move(extrusion));
    }
    void set(const std::string& name, const char* json) {
        JSDocument doc;
        doc.Parse(json);
        const JSValue* value = &doc;
        EXPECT_FALSE(layer->setProperty(name, style::conversion::Convertible(value)));
    }
};
void compare(Scene& builtin, Scene& plugin) {
    const auto a = builtin.frontend.render(builtin.map);
    const auto b = plugin.frontend.render(plugin.map);
    ASSERT_EQ(a.image.size, b.image.size);
    // Independent tessellation may rasterize shared edges differently. Require
    // all but 0.1% of pixels to agree within one 8-bit color unit.
    size_t different = 0;
    for (size_t i = 0; i < a.image.bytes(); i += 4) {
        bool differs = false;
        for (size_t c = 0; c < 4; ++c) differs |= std::abs(int(a.image.data[i + c]) - int(b.image.data[i + c])) > 1;
        different += differs;
    }
    EXPECT_LE(different, size_t(65));
}
TEST(FillExtrusionPlugin, ReplacementAndDynamicParity) {
    util::RunLoop loop;
    // Construct and render a built-in object before registration; it must also
    // survive subsequent property updates after the name has been replaced.
    Scene builtin(false);
    builtin.set("fill-extrusion-height", "100");
    builtin.frontend.render(builtin.map);
    char error[512]{};
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_fill_extrusion_register(captureDescriptor, nullptr, 0));
    auto copy = *registeredDescriptor;
    auto type = copy.layer_types[0];
    copy.layer_types = &type;
    type.replace_builtin = 0;
    EXPECT_EQ(MLN_PLUGIN_STATUS_CONFLICT, mln_plugin_register_v1(&copy, error, sizeof(error)));
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_fill_extrusion_register(mln_plugin_register_v1, error, sizeof(error))) << error;
    type.replace_builtin = 1;
    copy.plugin_id = {"another-extrusion", 17};
    EXPECT_EQ(MLN_PLUGIN_STATUS_CONFLICT, mln_plugin_register_v1(&copy, error, sizeof(error)));
    EXPECT_EQ(MLN_PLUGIN_STATUS_ALREADY_REGISTERED,
              mln_fill_extrusion_register(mln_plugin_register_v1, error, sizeof(error)));
    Scene plugin(true);
    for (auto* scene : {&builtin, &plugin}) {
        scene->set("fill-extrusion-height",
                   R"(["interpolate",["linear"],["zoom"],15,["get","h"],16,["*",2,["get","h"]]])");
        scene->set("fill-extrusion-color", "\"#da526c\"");
    }
    for (const auto* opacity : {"0", "0.5", "1", "0.25", "0", "1"}) {
        SCOPED_TRACE(opacity);
        builtin.set("fill-extrusion-opacity", opacity);
        plugin.set("fill-extrusion-opacity", opacity);
        compare(builtin, plugin);
    }
    for (const auto* gradient : {"false", "true"}) {
        builtin.set("fill-extrusion-vertical-gradient", gradient);
        plugin.set("fill-extrusion-vertical-gradient", gradient);
        for (const auto* anchor : {"\"map\"", "\"viewport\""}) {
            for (auto* scene : {&builtin, &plugin}) {
                scene->set("fill-extrusion-translate", "[21,-17]");
                scene->set("fill-extrusion-translate-anchor", anchor);
                scene->set("fill-extrusion-opacity", "0.4");
                scene->map.getStyle().getLight()->setIntensity(0.75f);
                scene->map.getStyle().getLight()->setColor(Color::parse("#a0c0ff").value());
            }
            compare(builtin, plugin);
        }
    }
    for (auto* scene : {&builtin, &plugin}) {
        scene->set("fill-extrusion-height", R"(["coalesce",["feature-state","height"],["get","h"]])");
        scene->set("fill-extrusion-color",
                   R"(["case",["boolean",["feature-state","selected"],false],"#12ef47","#ab3265"])");
        scene->frontend.getRenderer()->setFeatureState("s", {}, "1", {{"height", 180.0}, {"selected", true}});
    }
    compare(builtin, plugin);
    for (auto* scene : {&builtin, &plugin})
        scene->frontend.getRenderer()->setFeatureState("s", {}, "1", {{"height", 30.0}, {"selected", false}});
    compare(builtin, plugin);

    JSDocument unsupported;
    unsupported.SetString("unsupported");
    const JSValue* value = &unsupported;
    EXPECT_TRUE(plugin.layer->setProperty("fill-extrusion-pattern", style::conversion::Convertible(value)));
    EXPECT_TRUE(
        plugin.layer->setProperty("fill-extrusion-rounded-corner-distance", style::conversion::Convertible(value)));
    EXPECT_NE(builtin.layer->getTypeInfo(), plugin.layer->getTypeInfo());
}
} // namespace
