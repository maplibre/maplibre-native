#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/context.hpp>
#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/map.hpp>
#include <mln/map/map_observer.hpp>
#include <mln/map/map_options.hpp>
#include <mln/renderer/renderer.hpp>
#include <mln/storage/resource_options.hpp>
#include <mln/style/conversion_impl.hpp>
#include <mln/style/rapidjson_conversion.hpp>
#include <mln/style/layer.hpp>
#include <mln/style/style.hpp>
#include <mln/util/run_loop.hpp>
#include <Foundation/Foundation.hpp>
#ifdef MLN_WITH_PLUGINS
#include <mln/plugin/plugin_api.h>
#include <mln/plugin/plugin_performance.hpp>
#include <dlfcn.h>
#endif

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/resource.h>

using namespace mln;
using Clock = std::chrono::steady_clock;

namespace {
double milliseconds(Clock::time_point start) {
    return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
}

std::string fixture(size_t count, int layers, const std::string& paint, bool dense) {
    std::ostringstream json;
    json
        << R"({"version":8,"transition":{"duration":0,"delay":0},"sources":{"points":{"type":"geojson","data":{"type":"FeatureCollection","features":[)";
    // Fixed integer arithmetic: identical generated data in each process/build.
    for (size_t i = 0; i < count; ++i) {
        if (i) json << ',';
        const double scale = dense ? 0.02 : 1;
        const double x = (static_cast<int>((i * 7919) % 100003) / 100003.0 - 0.5) * 60 * scale;
        const double y = (static_cast<int>((i * 104729) % 100019) / 100019.0 - 0.5) * 40 * scale;
        json << R"({"type":"Feature","id":)" << i << R"(,"properties":{"size":)" << (i % 5 + 2)
             << R"(},"geometry":{"type":"Point","coordinates":[)" << x << ',' << y << "]}}";
    }
    json << "]}}},\"layers\":[";
    for (int i = 0; i < layers; ++i) {
        if (i) json << ',';
        json << R"({"id":"circles)" << i << R"(","type":"circle","source":"points","paint":{)";
        json << R"("circle-color":"#357ec7","circle-opacity":0.6,"circle-radius":)";
        if (paint == "constant")
            json << '4';
        else if (paint == "camera")
            json << R"(["interpolate",["linear"],["zoom"],0,2,8,8])";
        else if (paint == "composite")
            json << R"(["interpolate",["linear"],["zoom"],0,["get","size"],8,["*",2,["get","size"]]])";
        else if (paint == "state")
            json << R"(["coalesce",["feature-state","radius"],["get","size"]])";
        else if (paint == "feature")
            json << R"(["get","size"])";
        else
            throw std::runtime_error("unknown paint mode");
        json << "}}";
    }
    json << "]}";
    return json.str();
}
} // namespace

int main(int argc, char** argv) try {
    if (argc != 8 && !(argc == 9 && std::string(argv[8]) == "--profile")) {
        std::cerr << "Usage: circle-plugin-benchmark native|PLUGIN_DYLIB COUNT LAYERS "
                     "constant|camera|feature|composite|state FRAMES WARMUPS dense|spread [--profile]\n";
        return 2;
    }
    const std::string pluginPath = argv[1], paint = argv[4];
    const size_t count = std::stoull(argv[2]);
    const int layers = std::stoi(argv[3]), frames = std::stoi(argv[5]), warmups = std::stoi(argv[6]);
    if (!count || layers < 1 || frames < 1 || warmups < 0) throw std::runtime_error("invalid workload size");
    const bool dense = std::string(argv[7]) == "dense";
    const auto processPool = NS::TransferPtr(NS::AutoreleasePool::alloc()->init());
    const auto registrationStart = Clock::now();
#ifdef MLN_WITH_PLUGINS
    const bool profiling = argc == 9;
    plugin::performance::enabled.store(profiling, std::memory_order_relaxed);
    plugin::performance::Snapshot profileBefore;
    if (pluginPath != "native") {
        // Keep the DSO loaded for the process lifetime. Neither implementation
        // nor allocator ownership crosses this descriptor-only C boundary.
        auto* library = dlopen(pluginPath.c_str(), RTLD_NOW | RTLD_LOCAL);
        if (!library) throw std::runtime_error(dlerror());
        auto descriptor = reinterpret_cast<const mln_plugin_descriptor_v1* (*)()>(
            dlsym(library, "mln_circle_layer_descriptor"));
        if (!descriptor) throw std::runtime_error("missing circle descriptor symbol");
        char error[512]{};
        if (mln_plugin_register_v1(descriptor(), error, sizeof(error)) != MLN_PLUGIN_STATUS_OK)
            throw std::runtime_error(error);
    }
    const char* mode = pluginPath == "native" ? "enabled-native" : "enabled-plugin";
#else
    if (pluginPath != "native") throw std::runtime_error("this build disables plugins");
    const char* mode = "disabled-native";
#endif
    const double registrationMs = milliseconds(registrationStart);
    const auto json = fixture(count, layers, paint, dense);
    util::RunLoop loop;
    const auto startup = Clock::now();
    constexpr mln::Size size{1024, 768};
    HeadlessFrontend frontend{size, 1};
    Map map{frontend,
            MapObserver::nullObserver(),
            MapOptions().withMapMode(MapMode::Static).withSize(size).withPixelRatio(1),
            ResourceOptions()};
    map.getStyle().loadJSON(json);
    map.jumpTo(CameraOptions().withCenter(LatLng{0, 0}).withZoom(3));
    auto readback = [&] {
        const auto framePool = NS::TransferPtr(NS::AutoreleasePool::alloc()->init());
        return frontend.render(map);
    };
    const auto first = readback();
    const double startupMs = milliseconds(startup);
    if (!first.image.valid()) throw std::runtime_error("no first frame");
    auto submit = [&] {
        // Shader/drawable creation happens before Renderer::Impl's own pool.
        // Match an application's outer event-loop pool, not just the render pass.
        const auto framePool = NS::TransferPtr(NS::AutoreleasePool::alloc()->init());
        bool done = false;
        std::exception_ptr error;
        map.renderStill([&](std::exception_ptr e) {
            error = e;
            done = true;
        });
        while (!done) loop.runOnce();
        if (error) std::rethrow_exception(error);
    };
    auto stats = [&] {
        gfx::BackendScope scope{*frontend.getBackend()};
        return frontend.getBackend()->getContext().renderingStats();
    };
    std::cout << std::setprecision(10);
    auto report = [&](const char* phase, int frame, double elapsed, const gfx::RenderingStats& before) {
        const auto after = stats();
        std::cout << "{\"mode\":\"" << mode << "\",\"paint\":\"" << paint << "\",\"count\":" << count
                  << ",\"layers\":" << layers << ",\"dense\":" << dense << ",\"phase\":\"" << phase
                  << "\",\"frame\":" << frame << ",\"wall_ms\":" << elapsed
                  << ",\"encoding_ms\":" << after.encodingTime * 1000
                  << ",\"submit_wait_ms\":" << after.renderingTime * 1000 << ",\"draw_calls\":" << after.numDrawCalls
                  << ",\"buffer_bytes\":" << after.memBuffers
                  << ",\"upload_bytes\":" << after.bufferUpdateBytes - before.bufferUpdateBytes
                  << ",\"vertex_upload_bytes\":" << after.vertexUpdateBytes - before.vertexUpdateBytes
                  << ",\"index_upload_bytes\":" << after.indexUpdateBytes - before.indexUpdateBytes
                  << ",\"uniform_upload_bytes\":" << after.uniformUpdateBytes - before.uniformUpdateBytes
                  << ",\"metal_vertex_bytes_calls\":" << after.metalVertexBytesCalls - before.metalVertexBytesCalls
                  << ",\"metal_vertex_inline_bytes\":" << after.metalVertexInlineBytes - before.metalVertexInlineBytes
                  << ",\"metal_fragment_bytes_calls\":" << after.metalFragmentBytesCalls - before.metalFragmentBytesCalls
                  << ",\"metal_fragment_inline_bytes\":" << after.metalFragmentInlineBytes - before.metalFragmentInlineBytes
                  << ",\"metal_vertex_buffer_binds\":" << after.metalVertexBufferBinds - before.metalVertexBufferBinds
                  << ",\"metal_fragment_buffer_binds\":" << after.metalFragmentBufferBinds - before.metalFragmentBufferBinds;
#ifdef MLN_WITH_PLUGINS
        if (profiling && std::string_view(phase) != "registration") {
            const auto measured = plugin::performance::read();
            const char* names[] = {"geometry_layout", "snapshot", "binding", "state_update"};
            for (size_t i = 0; i < plugin::performance::Count; ++i)
                std::cout << ",\"plugin_" << names[i]
                          << "_ms\":" << (measured.nanoseconds[i] - profileBefore.nanoseconds[i]) / 1e6;
            std::cout << ",\"plugin_snapshot_count\":" << measured.snapshots - profileBefore.snapshots
                      << ",\"plugin_state_ranges\":" << measured.stateRanges - profileBefore.stateRanges;
        }
#endif
        std::cout << "}\n";
    };
    report("registration", 0, registrationMs, stats());
    report("startup_readback", 0, startupMs, {});
    for (int i = 0; i < warmups; ++i) submit();
    for (const std::string phase :
         {"warm_no_readback", "warm_readback", "zoom", "paint_update", "state_1pct", "state_100pct", "style_reload"}) {
        if (phase.starts_with("state_") && paint != "state") continue;
        const int samples = phase == "style_reload" ? std::min(frames, 10) : frames;
        for (int i = 0; i < samples; ++i) {
            const auto before = stats();
#ifdef MLN_WITH_PLUGINS
            if (profiling) profileBefore = plugin::performance::read();
#endif
            const auto start = Clock::now();
            if (phase == "zoom") map.jumpTo(CameraOptions().withZoom(3 + (i % 10) * 0.01));
            if (phase == "paint_update") {
                JSDocument value;
                value.SetDouble(0.5 + (i % 2) * 0.1);
                for (int layer = 0; layer < layers; ++layer) {
                    auto error = map.getStyle()
                                     .getLayer("circles" + std::to_string(layer))
                                     ->setProperty("circle-opacity",
                                                   style::conversion::Convertible(static_cast<const JSValue*>(&value)));
                    if (error) throw std::runtime_error(error->message);
                }
            }
            if (phase.starts_with("state_")) {
                const size_t changed = phase == "state_1pct" ? std::max(size_t{1}, count / 100) : count;
                for (size_t feature = 0; feature < changed; ++feature)
                    frontend.getRenderer()->setFeatureState(
                        "points", std::nullopt, std::to_string(feature), {{"radius", Value{double(3 + i % 2)}}});
            }
            if (phase == "style_reload") map.getStyle().loadJSON(json);
            if (phase == "warm_readback")
                readback();
            else
                submit();
            report(phase.c_str(), i, milliseconds(start), before);
        }
    }
    // Finish queued GPU work outside submission timings.
    readback();
    rusage usage{};
    getrusage(RUSAGE_SELF, &usage);
    std::cout << "{\"mode\":\"" << mode << "\",\"peak_rss_native_units\":" << usage.ru_maxrss << "}\n";
    return 0;
} catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
}
