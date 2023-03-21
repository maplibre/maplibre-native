#import "MLNAnnotationContainerView.h"
#import "MLNAnnotationView.h"

@interface MLNAnnotationContainerView ()

@property (nonatomic) NSMutableArray<MLNAnnotationView *> *annotationViews;

@end

@implementation MLNAnnotationContainerView

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        _annotationViews = [NSMutableArray array];
    }
    return self;
}

+ (instancetype)annotationContainerViewWithAnnotationContainerView:(nonnull MLNAnnotationContainerView *)annotationContainerView
{
    MLNAnnotationContainerView *newAnnotationContainerView = [[MLNAnnotationContainerView alloc] initWithFrame:annotationContainerView.frame];
    [newAnnotationContainerView addSubviews:annotationContainerView.subviews];
    return newAnnotationContainerView;
}

- (void)addSubviews:(NSArray<MLNAnnotationView *> *)subviews
{
    for (MLNAnnotationView *view in subviews)
    {
        [self addSubview:view];
        [self.annotationViews addObject:view];
    }
}

// MARK: UIAccessibility methods

- (UIAccessibilityTraits)accessibilityTraits {
    return UIAccessibilityTraitAdjustable;
}

- (void)accessibilityIncrement {
    [self.superview.superview accessibilityIncrement];
}

- (void)accessibilityDecrement {
    [self.superview.superview accessibilityDecrement];
}

@end
