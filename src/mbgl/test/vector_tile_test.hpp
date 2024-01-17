#include <mbgl/map/transform.hpp>
#include <mbgl/renderer/tile_parameters.hpp>
#include <mbgl/renderer/image_manager.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/test/fake_file_source.hpp>
#include <mbgl/text/glyph_manager.hpp>
#include <mbgl/util/run_loop.hpp>

#include <memory>

#include <mbgl/test/util.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/string.hpp>

namespace mbgl {

class VectorTileTest {
public:
    std::shared_ptr<FileSource> fileSource = std::make_shared<FakeFileSource>();
    TransformState transformState;
    util::RunLoop loop;
    style::Style style{fileSource, 1};
    AnnotationManager annotationManager{style};
    ImageManager imageManager;
    GlyphManager glyphManager;
    Tileset tileset{{"https://example.com"}, {0, 22}, "none"};

    TileParameters tileParameters{1.0,
                                  MapDebugOptions(),
                                  transformState,
                                  fileSource,
                                  MapMode::Continuous,
                                  annotationManager.makeWeakPtr(),
                                  imageManager,
                                  glyphManager,
                                  0};
};

} // namespace mbgl
