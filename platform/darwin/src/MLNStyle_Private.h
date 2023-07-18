#import "MLNStyle.h"

#import "MLNStyleLayer.h"
#import "MLNFillStyleLayer.h"

NS_ASSUME_NONNULL_BEGIN

namespace mbgl {
    namespace style {
        class Style;
    }
}

@class MLNAttributionInfo;
@class MLNMapView;
@class MLNOpenGLStyleLayer;
@class MLNVectorTileSource;
@class MLNVectorStyleLayer;

@interface MLNStyle (Private)

- (instancetype)initWithRawStyle:(mbgl::style::Style *)rawStyle stylable:(id <MLNStylable>)stylable;

@property (nonatomic, readonly, weak) id <MLNStylable> stylable;
@property (nonatomic, readonly) mbgl::style::Style *rawStyle;

- (nullable NSArray<MLNAttributionInfo *> *)attributionInfosWithFontSize:(CGFloat)fontSize linkColor:(nullable MLNColor *)linkColor;
@property (nonatomic, readonly, strong) NSMutableDictionary<NSString *, MLNOpenGLStyleLayer *> *openGLLayers;
- (void)setStyleClasses:(NSArray<NSString *> *)appliedClasses transitionDuration:(NSTimeInterval)transitionDuration;

@end

@interface MLNStyle (MLNStreetsAdditions)

@property (nonatomic, readonly, copy) NSArray<MLNVectorStyleLayer *> *placeStyleLayers;
@property (nonatomic, readonly, copy) NSArray<MLNVectorStyleLayer *> *roadStyleLayers;

@end

NS_ASSUME_NONNULL_END
