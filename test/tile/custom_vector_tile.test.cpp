#include <mln/test/util.hpp>
#include <mln/test/fake_file_source.hpp>
#include <mln/style/sources/custom_vector_source.hpp>
#include <mln/tile/custom_vector_tile.hpp>
#include <mln/style/custom_vector_tile_loader.hpp>

#include <mln/util/run_loop.hpp>
#include <mln/map/transform.hpp>
#include <mln/renderer/tile_parameters.hpp>
#include <mln/style/style.hpp>
#include <mln/annotation/annotation_manager.hpp>
#include <mln/renderer/image_manager.hpp>
#include <mln/text/glyph_manager.hpp>
#include <mln/gfx/dynamic_texture_atlas.hpp>

#include <memory>
#include <stdexcept>

using namespace mln;
using namespace mln::style;

class CustomVectorTileTest {
public:
    util::SimpleIdentity uniqueID;
    std::shared_ptr<FileSource> fileSource = std::make_shared<FakeFileSource>();
    TransformState transformState;
    util::RunLoop loop;
    AnnotationManager annotationManager{style};
    std::shared_ptr<ImageManager> imageManager = ImageManager::create();
    std::shared_ptr<GlyphManager> glyphManager = std::make_shared<GlyphManager>();
    gfx::DynamicTextureAtlasPtr dynamicTextureAtlas;

    TileParameters tileParameters;
    style::Style style;

    CustomVectorTileTest()
        : tileParameters{.pixelRatio = 1.0,
                         .debugOptions = MapDebugOptions(),
                         .transformState = transformState,
                         .fileSource = fileSource,
                         .mode = MapMode::Continuous,
                         .annotationManager = annotationManager.makeWeakPtr(),
                         .imageManager = imageManager,
                         .glyphManager = glyphManager,
                         .prefetchZoomDelta = 0,
                         .threadPool = {Scheduler::GetBackground(), uniqueID},
                         .dynamicTextureAtlas = dynamicTextureAtlas},
          style{fileSource, 1, tileParameters.threadPool} {}
};

TEST(CustomVectorTile, InvalidateFailedTile) {
    CustomVectorTileTest test;

    int fetches = 0;
    CustomVectorTileLoader loader([&](const CanonicalTileID&) { fetches++; }, nullptr);
    auto mb = std::make_shared<Mailbox>(*Scheduler::GetCurrent());
    ActorRef<CustomVectorTileLoader> loaderActor(loader, mb);

    CustomVectorTile tile(OverscaledTileID(0, 0, 0), "source", test.tileParameters, loaderActor);

    tile.setNecessity(TileNecessity::Required);
    test.loop.waitForEmpty();
    ASSERT_EQ(1, fetches);

    loader.setTileError(CanonicalTileID(0, 0, 0), std::make_exception_ptr(std::runtime_error("failed")));
    test.loop.waitForEmpty();

    loader.invalidateTile(CanonicalTileID(0, 0, 0));
    test.loop.waitForEmpty();

    // The render source updates the necessity of a changed tile.
    tile.setNecessity(TileNecessity::Required);
    test.loop.waitForEmpty();
    EXPECT_EQ(2, fetches);
}
