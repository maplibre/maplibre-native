#include <mln/test/map_adapter.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/stub_map_observer.hpp>
#include <mln/test/util.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/map_options.hpp>
#include <mln/renderer/buckets/symbol_bucket.hpp>
#include <mln/style/expression/dsl.hpp>
#include <mln/style/image.hpp>
#include <mln/style/layers/symbol_layer.hpp>
#include <mln/style/sources/geojson_source.hpp>
#include <mln/style/style.hpp>
#include <mln/util/io.hpp>
#include <mln/util/run_loop.hpp>

#if MLN_RENDER_BACKEND_METAL
#include <Foundation/Foundation.hpp>
#endif

#include <algorithm>

using namespace mln;

namespace {

enum class SymbolKind {
    Icon,
    SdfIcon,
    Text,
    TextAndIcon
};

void setupCollidingSymbolsStyle(style::Style& style, std::size_t count, double spacing, SymbolKind kind) {
    style.loadJSON(R"({
        "version": 8,
        "sources": {},
        "glyphs": "glyphs/{fontstack}/{range}.pbf",
        "layers": []
    })");

    FeatureCollection features;
    features.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        features.emplace_back(mln::Point<double>{5 + i * spacing, 5});
        features.back().properties["rank"] = static_cast<uint64_t>(i);
    }
    auto options = makeMutable<style::GeoJSONOptions>();
    options->maxzoom = 0;
    auto source = std::make_unique<style::GeoJSONSource>("points", std::move(options));
    source->setGeoJSON(features);
    style.addSource(std::move(source));

    auto layer = std::make_unique<style::SymbolLayer>("symbols", "points");
    using namespace style::expression::dsl;
    layer->setSymbolSortKey(style::PropertyExpression<float>(number(get("rank"))));
    if (kind != SymbolKind::Text) {
        layer->setIconImage({"marker"});
    }
    if (kind == SymbolKind::Text || kind == SymbolKind::TextAndIcon) {
        layer->setTextField({"Label"});
        layer->setTextFont(std::vector<std::string>{"Open Sans Regular"});
        layer->setTextSize(16.0f);
    }
    layer->setTextHaloColor(Color::white());
    layer->setTextHaloWidth(1.0f);
    style.addLayer(std::move(layer));
}

class SymbolRenderingTest {
public:
    explicit SymbolRenderingTest(std::size_t count,
                                 double spacing = 0,
                                 SymbolKind kind = SymbolKind::Icon,
                                 MapMode mode = MapMode::Static,
                                 MapObserver& observer = MapObserver::nullObserver())
        : map{frontend, observer, fileSource, MapOptions().withMapMode(mode).withSize(frontend.getSize())} {
        fileSource->glyphsResponse = [](const Resource&) {
            Response response;
            response.data = std::make_shared<const std::string>(util::read_file("test/fixtures/resources/glyphs.pbf"));
            return response;
        };
        setupCollidingSymbolsStyle(map.getStyle(), count, spacing, kind);
        PremultipliedImage marker({16, 16});
        std::fill_n(marker.data.get(), marker.bytes(), uint8_t{255});
        map.getStyle().addImage(
            std::make_unique<style::Image>("marker", std::move(marker), 1.0f, kind == SymbolKind::SdfIcon));
        map.jumpTo(CameraOptions().withCenter(LatLng{5, 5}).withZoom(4));
    }

    util::RunLoop loop;
    std::shared_ptr<StubFileSource> fileSource = std::make_shared<StubFileSource>();
    HeadlessFrontend frontend{mln::Size{256, 256}, 1.0f};
    MapAdapter map;
};

} // namespace

TEST(SymbolRendering, CollisionRejectedSymbolsDoNotDraw) {
    for (const auto kind : {SymbolKind::Icon, SymbolKind::SdfIcon, SymbolKind::Text, SymbolKind::TextAndIcon}) {
        SCOPED_TRACE(static_cast<int>(kind));
        const auto reference = [kind] {
            SymbolRenderingTest test(1, 0, kind);
            return test.frontend.render(test.map);
        }();

        SymbolRenderingTest test(256, 0, kind);
        const auto result = test.frontend.render(test.map);
        EXPECT_GT(result.stats.numDrawCalls, 0);
        EXPECT_EQ(result.stats.numDrawCalls, reference.stats.numDrawCalls);
        EXPECT_EQ(result.image, reference.image);

        const auto next = test.frontend.render(test.map);
        EXPECT_EQ(next.stats.numDrawCalls, reference.stats.numDrawCalls);
        EXPECT_EQ(result.image, next.image);
    }
}

TEST(SymbolRendering, CollisionVisibilityUpdatesExistingDrawables) {
    SymbolRenderingTest test(2, 0.5);
    test.map.jumpTo(CameraOptions().withCenter(LatLng{5, 5.25}).withZoom(4.1));
    const auto overlapping = test.frontend.render(test.map);
    EXPECT_EQ(overlapping.stats.numDrawCalls, 1);

    test.map.jumpTo(CameraOptions().withZoom(4.9));
    const auto separated = test.frontend.render(test.map);
    EXPECT_EQ(separated.stats.numDrawCalls, 2);

    test.map.jumpTo(CameraOptions().withZoom(4.1));
    const auto overlappingAgain = test.frontend.render(test.map);
    EXPECT_EQ(overlappingAgain.stats.numDrawCalls, 1);
    EXPECT_EQ(overlapping.image, overlappingAgain.image);
}

TEST(SymbolRendering, ContinuousCollisionVisibilityFadesAndReappears) {
    StubMapObserver observer;
    SymbolRenderingTest test(2, 0.5, SymbolKind::Icon, MapMode::Continuous, observer);
    test.map.jumpTo(CameraOptions().withCenter(LatLng{5, 5.25}).withZoom(4.9));

    enum class Phase {
        Initial,
        Colliding,
        Reappearing,
        Complete
    };
    auto phase = Phase::Initial;
    bool sawFadeOut = false;
    PremultipliedImage initialImage;

    observer.didFinishRenderingFrameCallback = [&](MapObserver::RenderFrameStatus status) {
        if (status.mode != MapObserver::RenderMode::Full) return;

        const auto drawCalls = status.renderingStats.numDrawCalls;
        switch (phase) {
            case Phase::Initial:
                if (status.needsRepaint) return;
                EXPECT_EQ(drawCalls, 2);
                initialImage = test.frontend.readStillImage();
                phase = Phase::Colliding;
                test.map.jumpTo(CameraOptions().withZoom(4.1));
                break;
            case Phase::Colliding:
                // A committed collision must keep the fading symbol drawable enabled.
                if (status.placementChanged && status.needsRepaint && drawCalls == 2) {
                    sawFadeOut = true;
                }
                if (drawCalls != 1) return;
                EXPECT_TRUE(sawFadeOut);
                phase = Phase::Reappearing;
                test.map.jumpTo(CameraOptions().withZoom(4.9));
                break;
            case Phase::Reappearing:
                if (status.needsRepaint) return;
                EXPECT_EQ(drawCalls, 2);
                EXPECT_EQ(test.frontend.readStillImage(), initialImage);
                phase = Phase::Complete;
                break;
            case Phase::Complete:
                break;
        }
    };

    // Exercise the real repaint/placement scheduling, but fail rather than hang
    // if hidden drawables never stop drawing or never become visible again.
    const auto deadline = Clock::now() + Seconds{10};
    while (phase != Phase::Complete && Clock::now() < deadline) {
#if MLN_RENDER_BACKEND_METAL
        // Headless continuous rendering has no application autorelease pool.
        const auto pool = NS::TransferPtr(NS::AutoreleasePool::alloc()->init());
#endif
        test.loop.runOnce();
    }
    EXPECT_EQ(phase, Phase::Complete);
}

TEST(SymbolRendering, KeepFadingSymbolsVisible) {
    SymbolBucket::Buffer buffer;
    // Without placement results, leave rendering enabled.
    EXPECT_TRUE(buffer.hasVisibleVertices(0, 1));

    buffer.opacityAttributeData().emplace_back(SymbolBucket::opacityAttributes(false, 0.0f),
                                               SymbolBucket::opacityAttributes(true, 0.0f),
                                               SymbolBucket::opacityAttributes(false, 0.5f),
                                               SymbolBucket::opacityAttributes(true, 1.0f));
    EXPECT_FALSE(buffer.hasVisibleVertices(0, 1));
    EXPECT_TRUE(buffer.hasVisibleVertices(1, 1)); // Just starting to fade in.
    EXPECT_TRUE(buffer.hasVisibleVertices(2, 1)); // Still fading out.
    EXPECT_TRUE(buffer.hasVisibleVertices(3, 1));
    EXPECT_TRUE(buffer.hasVisibleVertices(0, 4)); // Mixed visibility in one segment.
}

TEST(SymbolRendering, FullyHiddenLayerCanReappear) {
    SymbolRenderingTest test(256);
    const auto initial = test.frontend.render(test.map);

    auto blocker = std::make_unique<style::SymbolLayer>("blocker", "points");
    blocker->setIconImage({"marker"});
    test.map.getStyle().addLayer(std::move(blocker));
    const auto hidden = test.frontend.render(test.map);
    EXPECT_EQ(hidden.stats.numDrawCalls, 1);
    EXPECT_EQ(initial.image, hidden.image);

    auto* blockerLayer = test.map.getStyle().getLayer("blocker");
    ASSERT_NE(blockerLayer, nullptr);
    blockerLayer->setVisibility(style::VisibilityType::None);
    const auto visible = test.frontend.render(test.map);
    EXPECT_EQ(visible.stats.numDrawCalls, 1);
    EXPECT_EQ(initial.image, visible.image);
}
