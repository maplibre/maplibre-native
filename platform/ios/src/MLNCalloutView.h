#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@protocol MLNCalloutViewDelegate;
@protocol MLNAnnotation;

/**
 A protocol for a `UIView` subclass that displays information about a selected
 annotation near that annotation.

 To receive updates from an object that conforms to the `MLNCalloutView` protocol,
 use the optional methods available in the `MLNCalloutViewDelegate` protocol.

 #### Related examples
 TODO: Display custom views as callouts by customizing an
 `MLNCalloutView`.
 */
@protocol MLNCalloutView <NSObject>

/**
 An object conforming to the `MLNAnnotation` protocol whose details this callout
 view displays.
 */
@property (nonatomic, strong) id<MLNAnnotation> representedObject;

/**
 A view that the user may tap to perform an action. This view is conventionally
 positioned on the left side of the callout view.
 */
@property (nonatomic, strong) UIView *leftAccessoryView;

/**
 A view that the user may tap to perform an action. This view is conventionally
 positioned on the right side of the callout view.
 */
@property (nonatomic, strong) UIView *rightAccessoryView;

/**
 An object conforming to the `MLNCalloutViewDelegate` method that receives
 messages related to the callout view’s interactive subviews.
 */
@property (nonatomic, weak) id<MLNCalloutViewDelegate> delegate;

/**
 Presents a callout view by adding it to `view` and pointing at the given rect
 of `view`’s bounds. Constrains the callout to the bounds of the given view.
 */
- (void)presentCalloutFromRect:(CGRect)rect
                        inView:(UIView *)view
             constrainedToView:(UIView *)constrainedView
                      animated:(BOOL)animated
    __attribute__((
        unavailable("Use `-presentCalloutFromRect:inView:constrainedToRect:animated:` instead.")));

/**
 Presents a callout view by adding it to `view` and pointing at the given rect
 of `view`’s bounds. Constrains the callout to the rect in the space of `view`.
 */
- (void)presentCalloutFromRect:(CGRect)rect
                        inView:(UIView *)view
             constrainedToRect:(CGRect)constrainedRect
                      animated:(BOOL)animated;

/**
 Dismisses the callout view.
 */
- (void)dismissCalloutAnimated:(BOOL)animated;

@optional

/**
 If implemented, should provide margins to expand the rect the callout is presented from.

 These are used to determine positioning. Currently only the top and bottom properties of the return
 value are used. For example, `{ .top = -50.0, .left = -10.0, .bottom = 0.0, .right = -10.0 }`
 indicates a 50 point margin above the presentation origin rect (and 10 point margins to the left
 and the right) in which the callout is assumed to be displayed.

 There are no assumed defaults for these margins, as they should be calculated from the callout that
 is to be presented. For example, `SMCalloutView` generates the top margin from the callout height,
 but the left and right margins from a minimum width that the callout should have.

 @param rect Rect that the callout is presented from. This should be the same as the one passed in
 `-[MLNCalloutView presentCalloutFromRect:inView:constrainedToRect:animated:]`
 @return `UIEdgeInsets` representing the margins. Values should be negative.
 */
- (UIEdgeInsets)marginInsetsHintForPresentationFromRect:(CGRect)rect
    NS_SWIFT_NAME(marginInsetsHintForPresentation(from:));

/**
 A Boolean value indicating whether the callout view should be anchored to
 the corresponding annotation. You can adjust the callout view’s precise location by
 overriding -[UIView setCenter:]. The callout view will not be anchored to the
 annotation if this optional property is unimplemented.
 */
@property (nonatomic, readonly, assign, getter=isAnchoredToAnnotation) BOOL anchoredToAnnotation;

/**
 A Boolean value indicating whether the callout view should be dismissed automatically
 when the map view’s viewport changes. Note that a single tap on the map view
 still dismisses the callout view regardless of the value of this property.
 The callout view will be dismissed if this optional property is unimplemented.
 */
@property (nonatomic, readonly, assign) BOOL dismissesAutomatically;

@end

/**
 The `MLNCalloutViewDelegate` protocol defines a set of optional methods that
 you can use to receive messages from an object that conforms to the
 `MLNCalloutView` protocol. The callout view uses these methods to inform the
 delegate that the user has interacted with the the callout view.
 */
@protocol MLNCalloutViewDelegate <NSObject>

@optional
/**
 Returns a Boolean value indicating whether the entire callout view “highlights”
 when tapped. The default value is `YES`, which means the callout view
 highlights when tapped.

 The return value of this method is ignored unless the delegate also responds to
 the `-calloutViewTapped` method.
 */
- (BOOL)calloutViewShouldHighlight:(UIView<MLNCalloutView> *)calloutView;

/**
 Tells the delegate that the callout view has been tapped.
 */
- (void)calloutViewTapped:(UIView<MLNCalloutView> *)calloutView;

/**
 Called before the callout view appears on screen, or before the appearance
 animation will start.
 */
- (void)calloutViewWillAppear:(UIView<MLNCalloutView> *)calloutView;

/**
 Called after the callout view appears on screen, or after the appearance
 animation is complete.
 */
- (void)calloutViewDidAppear:(UIView<MLNCalloutView> *)calloutView;

@end

NS_ASSUME_NONNULL_END
