#import "MLNStyleLayer.h"

NS_ASSUME_NONNULL_BEGIN

@class MLNMapView;
@class MLNStyle;

MLN_EXPORT
@interface MLNCustomDrawableStyleLayer : MLNStyleLayer

@property (nonatomic, weak, readonly) MLNStyle *style;

- (instancetype)initWithIdentifier:(NSString *)identifier;

@end

NS_ASSUME_NONNULL_END
