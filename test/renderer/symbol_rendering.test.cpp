#include <mln/test/map_adapter.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/stub_map_observer.hpp>
#include <mln/test/util.hpp>

#include <mln/gfx/backend_scope.hpp>
#include <mln/gfx/context.hpp>
#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/map_options.hpp>
#include <mln/renderer/buckets/symbol_bucket.hpp>
#include <mln/renderer/renderer.hpp>
#include <mln/renderer/update_parameters.hpp>
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

// Keep resource loading asynchronous, but let tests choose when each frame is rendered.
class TimedHeadlessFrontend : public HeadlessFrontend {
public:
    TimedHeadlessFrontend(mln::Size size_, float pixelRatio_)
        : HeadlessFrontend(size_,
                           pixelRatio_,
                           gfx::HeadlessBackend::SwapBehaviour::NoFlush,
                           gfx::ContextMode::Unique,
                           std::nullopt,
                           false) {}

    void update(std::shared_ptr<UpdateParameters> parameters) override { latestParameters = std::move(parameters); }

    RenderResult renderAt(TimePoint time) {
        assert(latestParameters);
        const auto& p = *latestParameters;
        auto parameters = std::make_shared<UpdateParameters>(UpdateParameters{
            .styleLoaded = p.styleLoaded,
            .mode = p.mode,
            .pixelRatio = p.pixelRatio,
            .debugOptions = p.debugOptions,
            .timePoint = time,
            .transformState = p.transformState,
            .glyphURL = p.glyphURL,
            .fontFaces = p.fontFaces,
            .spriteLoaded = p.spriteLoaded,
            .transitionOptions = p.transitionOptions,
            .light = p.light,
            .images = p.images,
            .sources = p.sources,
            .layers = p.layers,
            .annotationManager = p.annotationManager,
            .fileSource = p.fileSource,
            .prefetchZoomDelta = p.prefetchZoomDelta,
            .stillImageRequest = p.stillImageRequest,
            .crossSourceCollisions = p.crossSourceCollisions,
            .fastPFOREnabled = p.fastPFOREnabled,
            .tileLodMinRadius = p.tileLodMinRadius,
            .tileLodScale = p.tileLodScale,
            .tileLodPitchThreshold = p.tileLodPitchThreshold,
            .tileLodZoomShift = p.tileLodZoomShift,
            .tileLodMode = p.tileLodMode,
        });
        gfx::BackendScope scope{*getBackend()};
        getRenderer()->render(parameters);
        return {readStillImage(), getBackend()->getContext().renderingStats()};
    }

private:
    std::shared_ptr<UpdateParameters> latestParameters;
};

template <class Frontend = HeadlessFrontend>
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
    Frontend frontend{mln::Size{256, 256}, 1.0f};
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

        const auto drawCalls = status.renderingStats->numDrawCalls;
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

TEST(SymbolRendering, CollisionFadeIntermediateFrames) {
    StubMapObserver observer;
    SymbolRenderingTest<TimedHeadlessFrontend> test(2, 0.5, SymbolKind::Icon, MapMode::Continuous, observer);
    test.map.jumpTo(CameraOptions().withCenter(LatLng{5, 5.25}).withZoom(4.9));

    bool fullyLoaded = false;
    observer.didFinishRenderingFrameCallback = [&](MapObserver::RenderFrameStatus status) {
        fullyLoaded = status.mode == MapObserver::RenderMode::Full;
    };
    const auto start = TimePoint{} + Seconds{100};
    const auto frame = [&](int milliseconds) {
        test.loop.runOnce();
        return test.frontend.renderAt(start + Milliseconds{milliseconds});
    };

    // Wait only for resources; simulated time stays fixed while loading.
    const auto deadline = Clock::now() + Seconds{10};
    while (!fullyLoaded && Clock::now() < deadline) {
        frame(0);
    }
    ASSERT_TRUE(fullyLoaded);
    // Commit the initial placement, then let its fade finish before changing collisions.
    frame(1000);
    const auto initial = frame(1400);
    ASSERT_EQ(initial.stats.numDrawCalls, 2);

    const auto alphaAtSymbol = [&](const PremultipliedImage& image, double longitude) {
        const auto pixel = test.map.pixelForLatLng(LatLng{5, longitude});
        const auto x = static_cast<uint32_t>(pixel.x);
        const auto y = static_cast<uint32_t>(pixel.y);
        return image.data[4 * (y * image.size.width + x) + 3];
    };
    ASSERT_EQ(alphaAtSymbol(initial.image, 5.5), 255);

    // Rotation makes the icon collision boxes overlap without zoom's fade adjustment.
    test.map.jumpTo(CameraOptions().withBearing(45));
    const auto fadeOutStart = frame(2000);
    const auto fadeOutMiddle = frame(2150);
    const auto fadeOutEnd = frame(2400);
    EXPECT_EQ(fadeOutStart.stats.numDrawCalls, 2);
    EXPECT_EQ(fadeOutMiddle.stats.numDrawCalls, 2);
    EXPECT_NEAR(alphaAtSymbol(fadeOutStart.image, 5.5), 255, 1);
    EXPECT_NEAR(alphaAtSymbol(fadeOutMiddle.image, 5.5), 128, 2);
    EXPECT_EQ(alphaAtSymbol(fadeOutEnd.image, 5.5), 0);
    EXPECT_EQ(alphaAtSymbol(fadeOutMiddle.image, 5.0), 255);
    const auto hidden = frame(2600);
    EXPECT_EQ(hidden.stats.numDrawCalls, 1);
    EXPECT_EQ(hidden.image, fadeOutEnd.image);

    test.map.jumpTo(CameraOptions().withBearing(0));
    const auto fadeInStart = frame(3000);
    const auto fadeInMiddle = frame(3150);
    const auto fadeInEnd = frame(3400);
    EXPECT_EQ(fadeInStart.stats.numDrawCalls, 2);
    EXPECT_EQ(fadeInMiddle.stats.numDrawCalls, 2);
    EXPECT_EQ(alphaAtSymbol(fadeInStart.image, 5.5), 0);
    EXPECT_NEAR(alphaAtSymbol(fadeInMiddle.image, 5.5), 128, 2);
    EXPECT_EQ(alphaAtSymbol(fadeInEnd.image, 5.5), 255);
    EXPECT_EQ(alphaAtSymbol(fadeInMiddle.image, 5.0), 255);
    EXPECT_EQ(fadeInEnd.image, initial.image);
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
