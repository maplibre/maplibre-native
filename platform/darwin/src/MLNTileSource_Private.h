#import <CoreGraphics/CoreGraphics.h>

#import "MLNFoundation.h"
#import "MLNTileSource.h"

#include <mln/util/tileset.hpp>

#include <optional>

NS_ASSUME_NONNULL_BEGIN

@class MLNAttributionInfo;

@interface MLNTileSource (Private)

/**
 An HTML string to be displayed as attribution when the map is shown to a user.

 The default value is `nil`. If the source is initialized with a configuration
 URL, this property is also `nil` until the configuration JSON file is loaded.
 */
@property (nonatomic, copy, nullable, readonly) NSString *attributionHTMLString;

/**
 A structured representation of the `attribution` property. The default value is
 `nil`.

 @param fontSize The default text size in points, or 0 to use the default.
 @param linkColor The default link color, or `nil` to use the default.
 */
- (NSArray<MLNAttributionInfo *> *)attributionInfosWithFontSize:(CGFloat)fontSize
                                                      linkColor:(nullable MLNColor *)linkColor;

@end

MLN_EXPORT
mln::Tileset MLNTileSetFromTileURLTemplates(
    NSArray<NSString *> *tileURLTemplates,
    NSDictionary<MLNTileSourceOption, id> *_Nullable options);

/**
 The raster-dem encoding that `MLNTileSourceOptionDEMEncoding` selects in `options`, or no value
 if the option is absent. Raises `NSInvalidArgumentException` for a value of the wrong type.
 */
MLN_EXPORT
std::optional<mln::Tileset::RasterEncoding> MLNRasterEncodingFromOptions(
    NSDictionary<MLNTileSourceOption, id> *_Nullable options);

NS_ASSUME_NONNULL_END
