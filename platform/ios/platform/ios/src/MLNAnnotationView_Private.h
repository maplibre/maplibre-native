#import "MLNAnnotationView.h"
#import "MLNAnnotation.h"

NS_ASSUME_NONNULL_BEGIN

@class MLNMapView;

@interface MLNAnnotationView (Private)

@property (nonatomic, readwrite, nullable) NSString *reuseIdentifier;
@property (nonatomic, weak) MLNMapView *mapView;

@end

NS_ASSUME_NONNULL_END
