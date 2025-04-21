#include <mbgl/test/util.hpp>
#include <mbgl/test/fake_file_source.hpp>
#include <mbgl/tile/raster_tile.hpp>
#include <mbgl/tile/tile_loader_impl.hpp>

#include <mbgl/style/style.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/map/transform.hpp>
#include <mbgl/annotation/annotation_manager.hpp>
#include <mbgl/renderer/tile_parameters.hpp>
#include <mbgl/renderer/buckets/raster_bucket.hpp>
#include <mbgl/renderer/image_manager.hpp>
#include <mbgl/text/glyph_manager.hpp>
#include <mbgl/gfx/dynamic_texture_atlas.hpp>

using namespace mbgl;

class RasterTileTest {
public:
    util::SimpleIdentity uniqueID;
    std::shared_ptr<FileSource> fileSource = std::make_shared<FakeFileSource>();
    TransformState transformState;
    util::RunLoop loop;
    AnnotationManager annotationManager{style};
    std::shared_ptr<ImageManager> imageManager = std::make_shared<ImageManager>();
    std::shared_ptr<GlyphManager> glyphManager = std::make_shared<GlyphManager>();
    gfx::DynamicTextureAtlasPtr dynamicTextureAtlas;

    Tileset tileset{{"https://example.com"}, {0, 22}, "none"};
    TileParameters tileParameters;
    style::Style style;

    RasterTileTest()
        : tileParameters{1.0,
                         MapDebugOptions(),
                         transformState,
                         fileSource,
                         MapMode::Continuous,
                         annotationManager.makeWeakPtr(),
                         imageManager,
                         glyphManager,
                         0,
                         {Scheduler::GetBackground(), uniqueID},
                         dynamicTextureAtlas},
          style{fileSource, 1, tileParameters.threadPool} {}
};

TEST(RasterTile, setError) {
    RasterTileTest test;
    RasterTile tile(OverscaledTileID(0, 0, 0), "testSource", test.tileParameters, test.tileset);
    tile.setError(std::make_exception_ptr(std::runtime_error("test")));
    EXPECT_FALSE(tile.isRenderable());
    EXPECT_TRUE(tile.isLoaded());
    EXPECT_TRUE(tile.isComplete());
}

TEST(RasterTile, onError) {
    RasterTileTest test;
    RasterTile tile(OverscaledTileID(0, 0, 0), "testSource", test.tileParameters, test.tileset);
    tile.onError(std::make_exception_ptr(std::runtime_error("test")), 0);
    EXPECT_FALSE(tile.isRenderable());
    EXPECT_TRUE(tile.isLoaded());
    EXPECT_TRUE(tile.isComplete());
}

TEST(RasterTile, onParsed) {
    RasterTileTest test;
    RasterTile tile(OverscaledTileID(0, 0, 0), "testSource", test.tileParameters, test.tileset);
    tile.onParsed(std::make_unique<RasterBucket>(PremultipliedImage{}), 0);
    EXPECT_TRUE(tile.isRenderable());
    EXPECT_TRUE(tile.isLoaded());
    EXPECT_TRUE(tile.isComplete());

    // Make sure that once we've had a renderable tile and then receive
    // erroneous data, we retain the previously rendered data and keep the tile
    // renderable.
    tile.setError(std::make_exception_ptr(std::runtime_error("Connection offline")));
    EXPECT_TRUE(tile.isRenderable());
    EXPECT_TRUE(tile.isLoaded());
    EXPECT_TRUE(tile.isComplete());

    // Then simulate a parsing failure and make sure that we keep it renderable
    // in this situation as well.
    tile.onError(std::make_exception_ptr(std::runtime_error("Parse error")), 0);
    ASSERT_TRUE(tile.isRenderable());
    EXPECT_TRUE(tile.isLoaded());
    EXPECT_TRUE(tile.isComplete());
}

TEST(RasterTile, onParsedEmpty) {
    RasterTileTest test;
    RasterTile tile(OverscaledTileID(0, 0, 0), "testSource", test.tileParameters, test.tileset);
    tile.onParsed(nullptr, 0);
    EXPECT_FALSE(tile.isRenderable());
    EXPECT_TRUE(tile.isLoaded());
    EXPECT_TRUE(tile.isComplete());
}
