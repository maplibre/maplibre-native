#import "MLNRasterDEMSource.h"

#import "MLNRasterTileSource_Private.h"
#import "NSURL+MLNAdditions.h"

#import <mln/style/sources/raster_dem_source.hpp>

const MLNTileSourceOption MLNTileSourceOptionDEMEncoding = @"MLNTileSourceOptionDEMEncoding";

@implementation MLNRasterDEMSource

- (std::unique_ptr<mln::style::RasterSource>)
    pendingSourceWithIdentifier:(NSString *)identifier
                   urlOrTileset:(mln::variant<std::string, mln::Tileset>)urlOrTileset
                       tileSize:(uint16_t)tileSize {
  auto source =
      std::make_unique<mln::style::RasterDEMSource>(identifier.UTF8String, urlOrTileset, tileSize);
  return source;
}
@end
