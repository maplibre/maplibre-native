#import "MLNAnnotationContainerView.h"
#import "MLNAnnotationView.h"

@class MLNAnnotationView;

NS_ASSUME_NONNULL_BEGIN

@interface MLNAnnotationContainerView (Private)

@property (nonatomic) NSMutableArray<MLNAnnotationView *> *annotationViews;

@end

NS_ASSUME_NONNULL_END
