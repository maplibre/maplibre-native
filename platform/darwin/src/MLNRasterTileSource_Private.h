#import "MLNRasterTileSource.h"

#include <memory>
#include <mln/util/variant.hpp>

namespace mln {
class Tileset;
namespace style {
class RasterSource;
}
}  // namespace mln

NS_ASSUME_NONNULL_BEGIN

@interface MLNRasterTileSource (Private)

@property (nonatomic, readonly, nullable) mln::style::RasterSource *rawSource;

- (std::unique_ptr<mln::style::RasterSource>)
    pendingSourceWithIdentifier:(NSString *)identifier
                   urlOrTileset:(mln::variant<std::string, mln::Tileset>)urlOrTileset
                       tileSize:(uint16_t)tileSize;

@end

NS_ASSUME_NONNULL_END
