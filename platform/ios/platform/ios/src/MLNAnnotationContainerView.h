#import <UIKit/UIKit.h>

#import "MLNTypes.h"

@class MLNAnnotationView;

NS_ASSUME_NONNULL_BEGIN

@interface MLNAnnotationContainerView : UIView

+ (instancetype)annotationContainerViewWithAnnotationContainerView:(MLNAnnotationContainerView *)annotationContainerView;

- (void)addSubviews:(NSArray<MLNAnnotationView *> *)subviews;

@end

NS_ASSUME_NONNULL_END
