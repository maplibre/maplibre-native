#import "MLNRasterDEMSource.h"

#import "MLNRasterTileSource_Private.h"
#import "MLNSource_Private.h"
#import "MLNTileSource_Private.h"
#import "NSURL+MLNAdditions.h"

#import <mln/style/sources/raster_dem_source.hpp>

const MLNTileSourceOption MLNTileSourceOptionDEMEncoding = @"MLNTileSourceOptionDEMEncoding";

@implementation MLNRasterDEMSource

- (instancetype)initWithIdentifier:(NSString *)identifier
                  configurationURL:(NSURL *)configurationURL
                          tileSize:(CGFloat)tileSize
                           options:(NSDictionary<MLNTileSourceOption, id> *)options {
  mln::style::SourceOptions sourceOptions;
  sourceOptions.rasterEncoding = MLNRasterEncodingFromOptions(options);
  NSString *configurationURLString = configurationURL.mgl_URLByStandardizingScheme.absoluteString;
  auto source = std::make_unique<mln::style::RasterDEMSource>(
      identifier.UTF8String, std::string(configurationURLString.UTF8String),
      uint16_t(round(tileSize)), sourceOptions);
  return self = [super initWithPendingSource:std::move(source)];
}

- (MLNDEMEncoding)encoding {
  MLNAssertStyleSourceIsValid();
  auto *source = static_cast<mln::style::RasterDEMSource *>(self.rawSource);
  return source->getEncoding() == mln::Tileset::RasterEncoding::Terrarium ? MLNDEMEncodingTerrarium
                                                                          : MLNDEMEncodingMapbox;
}

- (std::unique_ptr<mln::style::RasterSource>)
    pendingSourceWithIdentifier:(NSString *)identifier
                   urlOrTileset:(mln::variant<std::string, mln::Tileset>)urlOrTileset
                       tileSize:(uint16_t)tileSize {
  auto source =
      std::make_unique<mln::style::RasterDEMSource>(identifier.UTF8String, urlOrTileset, tileSize);
  return source;
}
@end
