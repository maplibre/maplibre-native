#include <mln/map/transform.hpp>
#include <mln/renderer/tile_parameters.hpp>
#include <mln/renderer/image_manager.hpp>
#include <mln/style/style.hpp>
#include <mln/test/fake_file_source.hpp>
#include <mln/text/glyph_manager.hpp>
#include <mln/util/run_loop.hpp>
#include <mln/gfx/dynamic_texture_atlas.hpp>

#include <memory>

#include <mln/test/util.hpp>
#include <mln/util/logging.hpp>
#include <mln/util/string.hpp>

namespace mln {

class VectorTileTest {
public:
    util::SimpleIdentity uniqueID;
    std::shared_ptr<FileSource> fileSource = std::make_shared<FakeFileSource>();
    TransformState transformState;
    util::RunLoop loop;
    AnnotationManager annotationManager{style};

    const std::shared_ptr<ImageManager> imageManager = ImageManager::create();
    const std::shared_ptr<GlyphManager> glyphManager = std::make_shared<GlyphManager>();
    gfx::DynamicTextureAtlasPtr dynamicTextureAtlas;
    TaggedScheduler threadPool;

    Tileset tileset{{"https://example.com"}, {0, 22}, "none"};
    TileParameters tileParameters;
    style::Style style;

    VectorTileTest()
        : threadPool(Scheduler::GetBackground(), uniqueID),
          tileParameters{.pixelRatio = 1.0,
                         .debugOptions = MapDebugOptions(),
                         .transformState = transformState,
                         .fileSource = fileSource,
                         .mode = MapMode::Continuous,
                         .annotationManager = annotationManager.makeWeakPtr(),
                         .imageManager = imageManager,
                         .glyphManager = glyphManager,
                         .prefetchZoomDelta = 0,
                         .threadPool = threadPool,
                         .dynamicTextureAtlas = dynamicTextureAtlas},
          style{fileSource, 1, threadPool} {}

    ~VectorTileTest() {
        // Ensure that deferred releases are complete before cleaning up
        loop.waitForEmpty();
        threadPool.waitForEmpty();
        threadPool.runRenderJobs(true);
    }
};

} // namespace mln
