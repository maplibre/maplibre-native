#include <fill_extrusion.hpp>
#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/map_options.hpp>
#include <mln/plugin/plugin_registry.hpp>
#include <mln/renderer/renderer.hpp>
#include <mln/style/style.hpp>
#include <mln/style/image.hpp>
#include <mln/style/layers/fill_extrusion_layer.hpp>
#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/layer.hpp>
#include <mln/style/rapidjson_conversion.hpp>
#include <mln/test/map_adapter.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/util/run_loop.hpp>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace mln;
namespace {
using Clock = std::chrono::steady_clock;
double milliseconds(Clock::time_point start) {
    return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
}
std::string sceneJSON(unsigned side) {
    std::ostringstream json;
    json.precision(12);
    json
        << R"({"version":8,"center":[0,0],"zoom":14.5,"pitch":55,"bearing":25,"sources":{"s":{"type":"geojson","data":{"type":"FeatureCollection","features":[)";
    for (unsigned y = 0; y < side; ++y)
        for (unsigned x = 0; x < side; ++x) {
            if (x || y) json << ',';
            const double step = 0.022 / side, a = (x - side / 2.0) * step, b = (y - side / 2.0) * step;
            const double c = a + step * 0.7, d = b + step * 0.7;
            json << R"({"type":"Feature","id":)" << y * side + x << R"(,"properties":{"h":)" << 10 + (x + y) % 30
                 << R"(},"geometry":{"type":"Polygon","coordinates":[[[)" << a << ',' << b << "],[" << c << ',' << b
                 << "],[" << c << ',' << d << "],[" << a << ',' << d << "],[" << a << ',' << b << "]]]}}";
        }
    json << R"(]}}},"layers":[{"id":"background","type":"background","paint":{"background-color":"#bcd"}}]})";
    return json.str();
}
void set(style::Layer& layer, const char* name, const char* json) {
    JSDocument document;
    document.Parse(json);
    const JSValue* value = &document;
    if (auto error = layer.setProperty(name, style::conversion::Convertible(value)))
        throw std::runtime_error(error->message);
}
double percentile(std::vector<double> values, double fraction) {
    std::sort(values.begin(), values.end());
    return values.at(static_cast<size_t>((values.size() - 1) * fraction));
}
struct Sample {
    std::vector<double> wall, encode, render;
    gfx::RenderingStats stats;
    void add(double time, const gfx::RenderingStats& s) {
        wall.push_back(time);
        encode.push_back(s.encodingTime * 1000);
        render.push_back(s.renderingTime * 1000);
        stats = s;
    }
    void print(const char* implementation, const char* scenario, const char* phase, unsigned features) const {
        std::cout << "{\"implementation\":\"" << implementation << "\",\"scenario\":\"" << scenario << "\",\"phase\":\""
                  << phase << "\",\"features\":" << features << ",\"samples\":" << wall.size()
                  << ",\"wall_ms\":" << percentile(wall, .5) << ",\"wall_p95_ms\":" << percentile(wall, .95)
                  << ",\"encode_ms\":" << percentile(encode, .5) << ",\"render_ms\":" << percentile(render, .5)
                  << ",\"draw_calls\":" << stats.numDrawCalls << ",\"vertex_bytes\":" << stats.memVertexBuffers
                  << ",\"index_bytes\":" << stats.memIndexBuffers << ",\"uniform_bytes\":" << stats.memUniformBuffers
                  << "}\n";
    }
};
} // namespace
int main(int argc, char** argv) try {
    if (argc != 4 || (std::string(argv[1]) != "builtin" && std::string(argv[1]) != "plugin")) {
        std::cerr << "Usage: mln-fill-extrusion-benchmark builtin|plugin grid-side frame-count\n";
        return 1;
    }
    const bool usePlugin = std::string(argv[1]) == "plugin";
    const auto side = static_cast<unsigned>(std::stoul(argv[2])), frames = static_cast<unsigned>(std::stoul(argv[3]));
    if (!side || side > 1000 || !frames) throw std::runtime_error("invalid sample size");
    util::RunLoop loop;
    if (usePlugin) {
        char error[512]{};
        if (mln_fill_extrusion_register(mln_plugin_register_v1, error, sizeof(error)) != MLN_PLUGIN_STATUS_OK)
            throw std::runtime_error(error);
    }
    const auto json = sceneJSON(side);
    for (const auto* scenario : {"solid", "data", "translucent", "rounded", "pattern"}) {
        const std::string name(scenario);
        auto source = std::make_shared<StubFileSource>();
        MapObserver observer;
        HeadlessFrontend frontend{Size{512, 512}, 1};
        MapAdapter map{frontend, observer, source, MapOptions().withMapMode(MapMode::Static).withSize({512, 512})};
        const auto start = Clock::now();
        map.getStyle().loadJSON(json);
        std::unique_ptr<style::Layer> layer;
        if (usePlugin) {
            style::conversion::Error error;
            auto parsed = style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
                R"({"id":"extrusion","source":"s","type":"fill-extrusion"})", error);
            if (!parsed) throw std::runtime_error(error.message);
            layer = std::move(*parsed);
            if (layer->getTypeInfo() != &plugin::PluginRegistry::get().findLayerType("fill-extrusion")->info)
                throw std::runtime_error("benchmark did not instantiate plugin");
        } else
            layer = std::make_unique<style::FillExtrusionLayer>("extrusion", "s");
        set(*layer,
            "fill-extrusion-height",
            name == "data" ? R"(["coalesce",["feature-state","h"],["get","h"]])" : "20");
        set(*layer,
            "fill-extrusion-color",
            name == "data" ? R"(["interpolate",["linear"],["get","h"],10,"#be5577",40,"#4ea6be"])" : R"("#be5577")");
        if (name == "translucent") set(*layer, "fill-extrusion-opacity", "0.5");
        if (name == "rounded") set(*layer, "fill-extrusion-rounded-corner-distance", "4");
        if (name == "pattern") {
            PremultipliedImage image({16, 16});
            for (size_t i = 0; i < image.bytes(); i += 4) {
                image.data[i] = (i / 4) % 16 < 8 ? 220 : 60;
                image.data[i + 1] = 100;
                image.data[i + 2] = 160;
                image.data[i + 3] = 255;
            }
            map.getStyle().addImage(std::make_unique<style::Image>("p", std::move(image), 1));
            set(*layer, "fill-extrusion-pattern", R"("p")");
        }
        auto* liveLayer = layer.get();
        map.getStyle().addLayer(std::move(layer));
        const auto first = frontend.render(map);
        Sample cold;
        cold.add(milliseconds(start), first.stats);
        cold.print(argv[1], scenario, "load", side * side);
        for (int warmup = 0; warmup < 10; ++warmup) frontend.render(map);
        for (const auto* phase : {"steady", "paint", "state"}) {
            if (std::string(phase) == "state" && name != "data") continue;
            Sample sample;
            for (unsigned frame = 0; frame < frames; ++frame) {
                const auto begin = Clock::now();
                if (std::string(phase) == "paint") set(*liveLayer, "fill-extrusion-opacity", frame % 2 ? "1" : "0.5");
                if (std::string(phase) == "state")
                    for (unsigned i = 0; i < std::min(side * side, 16u); ++i)
                        frontend.getRenderer()->setFeatureState(
                            "s", {}, std::to_string(i), {{"h", double(10 + frame % 30)}});
                const auto result = frontend.render(map);
                sample.add(milliseconds(begin), result.stats);
            }
            sample.print(argv[1], scenario, phase, side * side);
        }
    }
    return 0;
} catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
}
