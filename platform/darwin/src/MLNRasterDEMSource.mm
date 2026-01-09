#import "MLNRasterDEMSource.h"

#import "MLNRasterTileSource_Private.h"
#import "NSURL+MLNAdditions.h"

#import <mbgl/style/sources/raster_dem_source.hpp>

const MLNTileSourceOption MLNTileSourceOptionDEMEncoding = @"MLNTileSourceOptionDEMEncoding";

@implementation MLNRasterDEMSource

- (std::unique_ptr<mbgl::style::RasterSource>)pendingSourceWithIdentifier:(NSString *)identifier urlOrTileset:(mbgl::variant<std::string, mbgl::Tileset>)urlOrTileset tileSize:(uint16_t)tileSize {
    auto source = std::make_unique<mbgl::style::RasterDEMSource>(identifier.UTF8String,
                                                                 urlOrTileset,
                                                                 tileSize);
    return source;
}
@end
