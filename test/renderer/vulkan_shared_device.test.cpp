#if MLN_RENDER_BACKEND_VULKAN
#include <mln/test/util.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/map_adapter.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/map_options.hpp>
#include <mln/style/style.hpp>
#include <mln/util/run_loop.hpp>
#include <mln/vulkan/headless_backend.hpp>

#include <memory>
#include <thread>

using namespace mln;

namespace {

constexpr Size size{64, 64};
constexpr float pixelRatio = 1;

constexpr const char* style = R"({
  "version": 8,
  "sources": {
    "shape": {
      "type": "geojson",
      "data": {"type": "Polygon", "coordinates": [[[-40, -30], [50, -10], [10, 45], [-40, -30]]]}
    }
  },
  "layers": [
    {"id": "background", "type": "background", "paint": {"background-color": "#f0e8d8"}},
    {"id": "fill", "type": "fill", "source": "shape", "paint": {"fill-color": "#3070c0", "fill-opacity": 0.7}},
    {"id": "line", "type": "line", "source": "shape", "paint": {"line-color": "#c03030", "line-width": 3}}
  ]
})";

vk::Device deviceOf(HeadlessFrontend& frontend) {
    return static_cast<vulkan::RendererBackend*>(frontend.getBackend())->getDevice().get();
}

class TestMap {
public:
    explicit TestMap(std::shared_ptr<vulkan::SharedDevice> device = nullptr)
        : frontend(std::make_unique<vulkan::HeadlessBackend>(
                       size, gfx::HeadlessBackend::SwapBehaviour::NoFlush, gfx::ContextMode::Unique, std::move(device)),
                   size,
                   pixelRatio),
          map(frontend,
              MapObserver::nullObserver(),
              std::make_shared<StubFileSource>(ResourceOptions::Default(), ClientOptions()),
              MapOptions().withMapMode(MapMode::Static).withSize(size).withPixelRatio(pixelRatio)) {
        map.getStyle().loadJSON(style);
    }

    vk::Device device() { return deviceOf(frontend); }

    PremultipliedImage render(double zoom) {
        map.jumpTo(CameraOptions().withZoom(zoom));
        return frontend.render(map).image;
    }

private:
    HeadlessFrontend frontend;
    MapAdapter map;
};

} // namespace

TEST(VulkanSharedDevice, BackendsUseTheDeviceTheyShare) {
    util::RunLoop loop;
    const auto device = std::make_shared<vulkan::SharedDevice>();
    TestMap first(device);
    TestMap second(device);

    EXPECT_EQ(first.device(), second.device());
}

TEST(VulkanSharedDevice, BackendsWithoutOneCreateTheirOwnDevice) {
    util::RunLoop loop;
    TestMap first;
    TestMap second;

    EXPECT_NE(first.device(), second.device());
}

TEST(VulkanSharedDevice, HeadlessFrontendsOfAThreadShareItsDevice) {
    util::RunLoop loop;
    HeadlessFrontend first{size, pixelRatio};
    HeadlessFrontend second{size, pixelRatio};

    EXPECT_EQ(deviceOf(first), deviceOf(second));
}

TEST(VulkanSharedDevice, HeadlessFrontendsOfAnotherThreadUseAnotherDevice) {
    util::RunLoop loop;
    HeadlessFrontend first{size, pixelRatio};

    std::thread([&] {
        util::RunLoop threadLoop;
        HeadlessFrontend second{size, pixelRatio};
        EXPECT_NE(deviceOf(first), deviceOf(second));
    }).join();
}

TEST(VulkanSharedDevice, BackendsTakingTurnsRenderAsWithTheirOwnDevices) {
    util::RunLoop loop;
    const auto device = std::make_shared<vulkan::SharedDevice>();
    TestMap sharedFirst(device);
    TestMap sharedSecond(device);
    TestMap ownFirst;
    TestMap ownSecond;

    for (const double zoom : {0.0, 1.0, 2.0}) {
        EXPECT_EQ(ownFirst.render(zoom), sharedFirst.render(zoom));
        EXPECT_EQ(ownSecond.render(zoom + 0.5), sharedSecond.render(zoom + 0.5));
    }
}

TEST(VulkanSharedDevice, OutlivesTheBackendThatCreatedIt) {
    util::RunLoop loop;
    TestMap own;
    const auto device = std::make_shared<vulkan::SharedDevice>();
    auto first = std::make_unique<TestMap>(device);
    TestMap second(device);
    first.reset();

    EXPECT_EQ(own.render(1), second.render(1));
}

TEST(VulkanSharedDevice, OutlivesItsLastBackend) {
    util::RunLoop loop;
    TestMap own;
    const auto device = std::make_shared<vulkan::SharedDevice>();
    std::make_unique<TestMap>(device).reset();
    TestMap next(device);

    EXPECT_EQ(own.render(1), next.render(1));
}

#endif
