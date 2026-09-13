#include <mln/test/map_adapter.hpp>
#include <mln/test/stub_file_source.hpp>
#include <mln/test/util.hpp>

#include <mln/gfx/headless_frontend.hpp>
#include <mln/map/map_options.hpp>
#include <mln/renderer/buckets/symbol_bucket.hpp>
#include <mln/style/image.hpp>
#include <mln/style/layers/symbol_layer.hpp>
#include <mln/style/style.hpp>
#include <mln/util/io.hpp>
#include <mln/util/run_loop.hpp>

#include <algorithm>
#include <sstream>

using namespace mln;

namespace {

enum class SymbolKind {
    Icon,
    SdfIcon,
    Text,
    TextAndIcon
};

std::string collidingSymbolsStyle(std::size_t count, double spacing, SymbolKind kind) {
    std::ostringstream json;
    json << R"({"version":8,"sources":{"points":{"type":"geojson","maxzoom":0,"data":{
        "type":"FeatureCollection","features":[)";
    for (std::size_t i = 0; i < count; ++i) {
        if (i) json << ',';
        json << R"({"type":"Feature","properties":{"rank":)" << i << R"(},"geometry":{"type":"Point","coordinates":[)"
             << 5 + i * spacing << R"(,5]}})";
    }
    json << R"(]}}},"glyphs":"glyphs/{fontstack}/{range}.pbf",
        "layers":[{"id":"symbols","type":"symbol","source":"points","layout":{
        "symbol-sort-key":["get","rank"])";
    if (kind != SymbolKind::Text) json << R"(,"icon-image":"marker")";
    if (kind == SymbolKind::Text || kind == SymbolKind::TextAndIcon) {
        json << R"(,"text-field":"Label","text-font":["Open Sans Regular"],"text-size":16)";
    }
    json << R"(},"paint":{"text-halo-color":"white","text-halo-width":1}}]})";
    return json.str();
}

class SymbolRenderingTest {
public:
    explicit SymbolRenderingTest(std::size_t count, double spacing = 0, SymbolKind kind = SymbolKind::Icon) {
        fileSource->glyphsResponse = [](const Resource&) {
            Response response;
            response.data = std::make_shared<const std::string>(util::read_file("test/fixtures/resources/glyphs.pbf"));
            return response;
        };
        map.getStyle().loadJSON(collidingSymbolsStyle(count, spacing, kind));
        PremultipliedImage marker({16, 16});
        std::fill_n(marker.data.get(), marker.bytes(), uint8_t{255});
        map.getStyle().addImage(
            std::make_unique<style::Image>("marker", std::move(marker), 1.0f, kind == SymbolKind::SdfIcon));
        map.jumpTo(CameraOptions().withCenter(LatLng{5, 5}).withZoom(4));
    }

    util::RunLoop loop;
    std::shared_ptr<StubFileSource> fileSource = std::make_shared<StubFileSource>();
    HeadlessFrontend frontend{Size{256, 256}, 1.0f};
    MapAdapter map{frontend,
                   MapObserver::nullObserver(),
                   fileSource,
                   MapOptions().withMapMode(MapMode::Static).withSize(frontend.getSize())};
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
    auto* blockerLayer = blocker.get();
    test.map.getStyle().addLayer(std::move(blocker));
    const auto hidden = test.frontend.render(test.map);
    EXPECT_EQ(hidden.stats.numDrawCalls, 1);
    EXPECT_EQ(initial.image, hidden.image);

    blockerLayer->setVisibility(style::VisibilityType::None);
    const auto visible = test.frontend.render(test.map);
    EXPECT_EQ(visible.stats.numDrawCalls, 1);
    EXPECT_EQ(initial.image, visible.image);
}
