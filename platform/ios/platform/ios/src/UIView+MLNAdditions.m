#import "UIView+MLNAdditions.h"

@implementation UIView (MLNAdditions)

- (UIViewController *)mgl_viewControllerForLayoutGuides
{
    // Per -[UIResponder nextResponder] documentation, a UIView’s next responder
    // is its managing UIViewController if applicable, or otherwise its
    // superview. UIWindow’s next responder is UIApplication, which has no next
    // responder.
    UIResponder *laterResponder = self;
    while ([laterResponder isKindOfClass:[UIView class]])
    {
        laterResponder = laterResponder.nextResponder;
    }
    if ([laterResponder isKindOfClass:[UIViewController class]])
    {
        return (UIViewController *)laterResponder;
    }
    return nil;
}

- (NSLayoutYAxisAnchor *)mgl_safeTopAnchor {
    return self.safeAreaLayoutGuide.topAnchor;
}

- (NSLayoutXAxisAnchor *)mgl_safeLeadingAnchor {
    return self.safeAreaLayoutGuide.leadingAnchor;
}

- (NSLayoutYAxisAnchor *)mgl_safeBottomAnchor {
    return self.safeAreaLayoutGuide.bottomAnchor;
}

- (NSLayoutXAxisAnchor *)mgl_safeTrailingAnchor {
    return self.safeAreaLayoutGuide.trailingAnchor;
}

- (CGRect)mgl_frameForIdentifyTransform {
    CGPoint center = self.center;
    CGSize size = self.bounds.size;

    return CGRectMake(
                      center.x - size.width / 2,
                      center.y - size.height / 2,
                      size.width,
                      size.height
                      );
}

@end
