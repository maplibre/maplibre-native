#import "MLNStyle.h"

#import "MLNFillStyleLayer.h"
#import "MLNStyleLayer.h"

NS_ASSUME_NONNULL_BEGIN

namespace mln {
namespace style {
class Style;
}
}  // namespace mln

@class MLNAttributionInfo;
@class MLNMapView;
@class MLNCustomStyleLayer;
@class MLNVectorTileSource;
@class MLNVectorStyleLayer;

@interface MLNStyle (Private)

- (instancetype)initWithRawStyle:(mln::style::Style *)rawStyle stylable:(id<MLNStylable>)stylable;

@property (nonatomic, readonly, weak) id<MLNStylable> stylable;
@property (nonatomic, readonly) mln::style::Style *rawStyle;

- (nullable NSArray<MLNAttributionInfo *> *)attributionInfosWithFontSize:(CGFloat)fontSize
                                                               linkColor:
                                                                   (nullable MLNColor *)linkColor;
@property (nonatomic, readonly, strong)
    NSMutableDictionary<NSString *, MLNCustomStyleLayer *> *customLayers;
- (void)setStyleClasses:(NSArray<NSString *> *)appliedClasses
     transitionDuration:(NSTimeInterval)transitionDuration;

@end

@interface MLNStyle (MLNStreetsAdditions)

@property (nonatomic, readonly, copy) NSArray<MLNVectorStyleLayer *> *placeStyleLayers;
@property (nonatomic, readonly, copy) NSArray<MLNVectorStyleLayer *> *roadStyleLayers;

@end

NS_ASSUME_NONNULL_END
