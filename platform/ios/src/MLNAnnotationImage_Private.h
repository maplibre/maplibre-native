#import "MLNAnnotationImage.h"

NS_ASSUME_NONNULL_BEGIN

@protocol MLNAnnotationImageDelegate <NSObject>

@required
- (void)annotationImageNeedsRedisplay:(MLNAnnotationImage *)annotationImage;

@end

@interface MLNAnnotationImage (Private)

/// Unique identifier of the sprite image used by the style to represent the receiver’s `image`.
@property (nonatomic, strong, nullable) NSString *styleIconIdentifier;

@property (nonatomic, weak) id<MLNAnnotationImageDelegate> delegate;

@end

NS_ASSUME_NONNULL_END
