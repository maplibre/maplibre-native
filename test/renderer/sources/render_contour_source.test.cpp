#include <mbgl/test/fake_file_source.hpp>
#include <mbgl/test/util.hpp>

#include <mbgl/actor/scheduler.hpp>
#include <mbgl/annotation/annotation_manager.hpp>
#include <mbgl/gfx/dynamic_texture_atlas.hpp>
#include <mbgl/map/transform.hpp>
#include <mbgl/renderer/image_manager.hpp>
#include <mbgl/renderer/sources/render_contour_source.hpp>
#include <mbgl/renderer/sources/render_raster_dem_source.hpp>
#include <mbgl/renderer/tile_parameters.hpp>
#include <mbgl/storage/file_source.hpp>
#include <mbgl/style/sources/contour_source.hpp>
#include <mbgl/style/sources/contour_source_impl.hpp>
#include <mbgl/style/sources/tile_source_impl.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/text/glyph_manager.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/tileset.hpp>

using namespace mbgl;

// Fixture for render-source-level tests of RenderContourSource. Builds the
// minimum scaffolding needed to drive update() without an orchestrator: a
// scheduler, file source, Style (for the AnnotationManager), and a
// TileParameters builder that takes a custom getRenderSource lambda.
//
// The cross-source lookup added for this feature (TileParameters::getRenderSource)
// makes RenderContourSource depend on a sibling RenderRasterDEMSource by ID
// at every update(). The lifetime tests below pin the contract that the
// contour source survives any destruction order between itself and the
// upstream — RenderOrchestrator destroys sources via std::unordered_map::erase,
// with no guarantee about sibling ordering.
class RenderContourSourceTest {
public:
    util::SimpleIdentity uniqueID;
    util::RunLoop loop;
    std::shared_ptr<FileSource> fileSource = std::make_shared<FakeFileSource>();
    Transform transform;
    TransformState transformState;
    std::shared_ptr<ImageManager> imageManager = std::make_shared<ImageManager>();
    std::shared_ptr<GlyphManager> glyphManager = std::make_shared<GlyphManager>();
    gfx::DynamicTextureAtlasPtr dynamicTextureAtlas;
    TaggedScheduler threadPool;
    style::Style style;
    AnnotationManager annotationManager{style};

    Tileset tileset{{"https://example.com/{z}/{x}/{y}.png"}, {0, 12}};

    RenderContourSourceTest()
        : threadPool(Scheduler::GetBackground(), uniqueID),
          style{fileSource, 1, threadPool} {
        transform.resize({512, 512});
        transform.jumpTo(CameraOptions().withCenter(LatLng()).withZoom(0.0));
        transformState = transform.getState();
    }

    ~RenderContourSourceTest() { threadPool.waitForEmpty(); }

    TileParameters makeParameters(std::function<RenderSource*(const std::string&)> getRenderSource = {}) {
        return {.pixelRatio = 1.0,
                .debugOptions = MapDebugOptions(),
                .transformState = transformState,
                .fileSource = fileSource,
                .mode = MapMode::Continuous,
                .annotationManager = annotationManager.makeWeakPtr(),
                .imageManager = imageManager,
                .glyphManager = glyphManager,
                .prefetchZoomDelta = 0,
                .threadPool = threadPool,
                .dynamicTextureAtlas = dynamicTextureAtlas,
                .getRenderSource = std::move(getRenderSource)};
    }

    Immutable<style::TileSource::Impl> makeRasterDEMImpl(const std::string& id) {
        // TileSource::Impl(SourceType, id, tileSize) constructs without a
        // tileset; the second-form constructor stamps in the tileset.
        return makeMutable<style::TileSource::Impl>(
            *makeMutable<style::TileSource::Impl>(style::SourceType::RasterDEM, id, util::tileSize_I), tileset);
    }

    Immutable<style::ContourSource::Impl> makeContourImpl(const std::string& id, const std::string& upstreamID) {
        style::ContourSourceOptions options;
        options.sourceID = upstreamID;
        options.intervals.stops = {100.0};
        return makeMutable<style::ContourSource::Impl>(id, std::move(options));
    }
};

TEST(RenderContourSource, ConstructsAndDestructsCleanly) {
    // No update() ever called — no listener registered. Pure smoke test for
    // the basic constructor/destructor pair.
    RenderContourSourceTest test;
    auto impl = test.makeContourImpl("contours", "dem");
    RenderContourSource source(impl, test.threadPool);
}

TEST(RenderContourSource, UpdateWithNoUpstreamAvailableIsSafe) {
    // First update() with getRenderSource returning nullptr (e.g. style still
    // loading, or the raster-dem source the contour source references just
    // doesn't exist). Must not crash; no listener registered.
    RenderContourSourceTest test;
    auto impl = test.makeContourImpl("contours", "dem");
    RenderContourSource source(impl, test.threadPool);

    auto params = test.makeParameters([](const std::string&) -> RenderSource* { return nullptr; });
    source.update(impl, {}, /*needsRendering=*/false, /*needsRelayout=*/false, params);
}

TEST(RenderContourSource, UpdateWithEmptyGetRenderSourceCallbackIsSafe) {
    // The TileParameters::getRenderSource std::function may be empty (e.g.
    // when constructing tile parameters in tests outside the orchestrator).
    // rebindUpstream must guard against the empty function and not invoke it.
    RenderContourSourceTest test;
    auto impl = test.makeContourImpl("contours", "dem");
    RenderContourSource source(impl, test.threadPool);

    auto params = test.makeParameters({}); // empty std::function
    source.update(impl, {}, /*needsRendering=*/false, /*needsRelayout=*/false, params);
}

TEST(RenderContourSource, DestructorIsSafeAfterUpstreamDestroyedFirst) {
    // The C1 lifetime contract: RenderOrchestrator destroys sources via
    // std::unordered_map::erase, which gives no guarantee about destruction
    // order between siblings. If the upstream raster-dem source is destroyed
    // BEFORE the contour source, the contour's cached `upstream` pointer is
    // dangling. The destructor must not dereference it.
    //
    // Earlier versions of this code called `upstream->removeTileLoadListener`
    // from the destructor — a use-after-free in this scenario. The current
    // destructor is a no-op; cleanup happens automatically because the
    // listener_set is owned by upstream and dies with it.
    RenderContourSourceTest test;
    auto demImpl = test.makeRasterDEMImpl("dem");
    auto upstream = std::make_unique<RenderRasterDEMSource>(demImpl, test.threadPool);

    auto contourImpl = test.makeContourImpl("contours", "dem");
    auto contour = std::make_unique<RenderContourSource>(contourImpl, test.threadPool);

    auto params = test.makeParameters(
        [&](const std::string& id) -> RenderSource* { return id == "dem" ? upstream.get() : nullptr; });

    // First update subscribes the contour source's listener with upstream.
    contour->update(contourImpl, {}, /*needsRendering=*/false, /*needsRelayout=*/false, params);

    // Destroy upstream first — the listener_set goes with it, taking the
    // contour source's listener closure along with it. The contour source's
    // cached `upstream` pointer is now dangling.
    upstream.reset();

    // Destroying the contour source after must not UAF. (The destructor
    // intentionally does not call removeTileLoadListener.)
    contour.reset();
}

TEST(RenderContourSource, DestructorIsSafeWhenContourDestroyedFirst) {
    // The other direction of the same contract: contour source destroyed
    // first while upstream is still alive. The listener closure captures a
    // WeakPtr, so any in-flight notify() on upstream after the contour is
    // gone will safely no-op.
    RenderContourSourceTest test;
    auto demImpl = test.makeRasterDEMImpl("dem");
    auto upstream = std::make_unique<RenderRasterDEMSource>(demImpl, test.threadPool);

    auto contourImpl = test.makeContourImpl("contours", "dem");
    auto contour = std::make_unique<RenderContourSource>(contourImpl, test.threadPool);

    auto params = test.makeParameters(
        [&](const std::string& id) -> RenderSource* { return id == "dem" ? upstream.get() : nullptr; });
    contour->update(contourImpl, {}, /*needsRendering=*/false, /*needsRelayout=*/false, params);

    // Destroy contour first.
    contour.reset();

    // Destroy upstream after — listener_set destruction must not crash.
    upstream.reset();
}

TEST(RenderContourSource, RebindsWhenUpstreamPointerChanges) {
    // rebindUpstream must drop the cached pointer when getRenderSource
    // returns a different RenderRasterDEMSource* than last time (e.g. user
    // removed and re-added the dem source under the same ID). The previous
    // upstream may already have been destroyed, so the rebind path must NOT
    // dereference the old cached pointer.
    RenderContourSourceTest test;
    auto demImpl = test.makeRasterDEMImpl("dem");
    auto upstream1 = std::make_unique<RenderRasterDEMSource>(demImpl, test.threadPool);
    auto upstream2 = std::make_unique<RenderRasterDEMSource>(demImpl, test.threadPool);

    auto contourImpl = test.makeContourImpl("contours", "dem");
    auto contour = std::make_unique<RenderContourSource>(contourImpl, test.threadPool);

    RenderRasterDEMSource* currentUpstream = upstream1.get();
    auto params = test.makeParameters(
        [&](const std::string& id) -> RenderSource* { return id == "dem" ? currentUpstream : nullptr; });
    contour->update(contourImpl, {}, /*needsRendering=*/false, /*needsRelayout=*/false, params);

    // Simulate orchestrator removing upstream1 entirely before contour's
    // next update sees the swap.
    upstream1.reset();
    currentUpstream = upstream2.get();
    contour->update(contourImpl, {}, /*needsRendering=*/false, /*needsRelayout=*/false, params);

    // Verify we can keep going with upstream2 and tear down cleanly.
    contour.reset();
    upstream2.reset();
}

TEST(RenderContourSource, RebindToNullClearsHandleWithoutDereferencingDeadUpstream) {
    // User removes the upstream raster-dem source from the style. On the
    // next frame, getRenderSource("dem") returns nullptr. rebindUpstream
    // must drop the cached pointer (now potentially dangling) without
    // dereferencing it.
    RenderContourSourceTest test;
    auto demImpl = test.makeRasterDEMImpl("dem");
    auto upstream = std::make_unique<RenderRasterDEMSource>(demImpl, test.threadPool);

    auto contourImpl = test.makeContourImpl("contours", "dem");
    auto contour = std::make_unique<RenderContourSource>(contourImpl, test.threadPool);

    bool upstreamAvailable = true;
    auto params = test.makeParameters([&](const std::string& id) -> RenderSource* {
        if (!upstreamAvailable) return nullptr;
        return id == "dem" ? upstream.get() : nullptr;
    });
    contour->update(contourImpl, {}, /*needsRendering=*/false, /*needsRelayout=*/false, params);

    // Orchestrator destroys upstream and stops returning it.
    upstreamAvailable = false;
    upstream.reset();

    // Next update sees nullptr from getRenderSource — must not dereference
    // the now-dangling cached pointer.
    contour->update(contourImpl, {}, /*needsRendering=*/false, /*needsRelayout=*/false, params);
}
