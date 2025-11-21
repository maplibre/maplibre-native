#import "MLNMapSnapshotter.h"

#import <mbgl/actor/actor.hpp>
#import <mbgl/actor/scheduler.hpp>
#import <mbgl/util/geo.hpp>
#import <mbgl/map/map_options.hpp>
#import <mbgl/map/map_snapshotter.hpp>
#import <mbgl/map/camera.hpp>
#import <mbgl/storage/resource_options.hpp>
#import <mbgl/util/client_options.hpp>
#import <mbgl/util/string.hpp>

#import "MLNOfflineStorage_Private.h"
#import "MLNGeometry_Private.h"
#import "MLNStyle_Private.h"
#import "MLNAttributionInfo_Private.h"
#import "MLNLoggingConfiguration_Private.h"
#import "MLNRendererConfiguration.h"
#import "MLNMapSnapshotter_Private.h"
#import "MLNSettings_Private.h"
#import "MLNShape.h"
#import "MLNPolyline.h"
#import "MLNPolygon.h"
#import "MLNMultiPoint_Private.h"
#import "MLNShapeCollection.h"
#import "MLNPointCollection.h"
#import "MLNAnnotationImage_Private.h"

#if TARGET_OS_IPHONE
#import "UIImage+MLNAdditions.h"
#import "UIColor+MLNAdditions.h"
#else
#import "NSImage+MLNAdditions.h"
#import "NSColor+MLNAdditions.h"
#import <CoreGraphics/CoreGraphics.h>
#import <CoreImage/CIContext.h>
#import <CoreImage/CIFilter.h>
#import <QuartzCore/QuartzCore.h>
#endif

#import "NSBundle+MLNAdditions.h"

const CGPoint MLNLogoImagePosition = CGPointMake(8, 8);
const CGFloat MLNSnapshotterMinimumPixelSize = 64;
NSString * const MLNSnapshotterAnnotationSpritePrefix = @"org.maplibre.sprites.";

MLNImage *MLNAttributedSnapshot(mbgl::MapSnapshotter::Attributions attributions, MLNImage *mglImage, mbgl::MapSnapshotter::PointForFn pointForFn, mbgl::MapSnapshotter::LatLngForFn latLngForFn, MLNMapSnapshotOptions *options, MLNMapSnapshotOverlayHandler overlayHandler);
MLNMapSnapshot *MLNSnapshotWithDecoratedImage(MLNImage *mglImage, MLNMapSnapshotOptions *options, mbgl::MapSnapshotter::Attributions attributions, mbgl::MapSnapshotter::PointForFn pointForFn, mbgl::MapSnapshotter::LatLngForFn latLngForFn, MLNMapSnapshotOverlayHandler overlayHandler, NSError * _Nullable *outError);
NSArray<MLNAttributionInfo *> *MLNAttributionInfosFromAttributions(mbgl::MapSnapshotter::Attributions attributions);

class MLNMapSnapshotterDelegateHost: public mbgl::MapSnapshotterObserver {
public:
    MLNMapSnapshotterDelegateHost(MLNMapSnapshotter *snapshotter_) : snapshotter(snapshotter_) {}

    void onDidFailLoadingStyle(const std::string& errorMessage) {
        MLNMapSnapshotter *strongSnapshotter = snapshotter;
        if ([strongSnapshotter.delegate respondsToSelector:@selector(mapSnapshotterDidFail:withError:)]) {
            NSString *description = @(errorMessage.c_str());
            NSDictionary *userInfo = @{
                NSLocalizedDescriptionKey: NSLocalizedStringWithDefaultValue(@"SNAPSHOT_LOAD_STYLE_FAILED_DESC", nil, nil, @"The snapshot failed because the style can’t be loaded.", @"User-friendly error description"),
                NSLocalizedFailureReasonErrorKey: description,
            };
            NSError *error = [NSError errorWithDomain:MLNErrorDomain code:MLNErrorCodeLoadStyleFailed userInfo:userInfo];
            [strongSnapshotter.delegate mapSnapshotterDidFail:snapshotter withError:error];
        }
    }

    void onDidFinishLoadingStyle() {
        MLNMapSnapshotter *strongSnapshotter = snapshotter;
        if ([strongSnapshotter.delegate respondsToSelector:@selector(mapSnapshotter:didFinishLoadingStyle:)]) {
            [strongSnapshotter.delegate mapSnapshotter:snapshotter didFinishLoadingStyle:snapshotter.style];
        }
    }

    void onStyleImageMissing(const std::string& imageName) {
        MLNMapSnapshotter *strongSnapshotter = snapshotter;
        if ([strongSnapshotter.delegate respondsToSelector:@selector(mapSnapshotter:didFailLoadingImageNamed:)]) {
            [strongSnapshotter.delegate mapSnapshotter:snapshotter didFailLoadingImageNamed:@(imageName.c_str())];
        }
    }

private:
    __weak MLNMapSnapshotter *snapshotter;
};

@interface MLNMapSnapshotOverlay() <MLNMapSnapshotProtocol>
@property (nonatomic, assign) CGFloat scale;
- (instancetype)initWithContext:(CGContextRef)context scale:(CGFloat)scale pointForFn:(mbgl::MapSnapshotter::PointForFn)pointForFn latLngForFn:(mbgl::MapSnapshotter::LatLngForFn)latLngForFn;

@end

@implementation MLNMapSnapshotOverlay {
    mbgl::MapSnapshotter::PointForFn _pointForFn;
    mbgl::MapSnapshotter::LatLngForFn _latLngForFn;
}

- (instancetype) initWithContext:(CGContextRef)context scale:(CGFloat)scale pointForFn:(mbgl::MapSnapshotter::PointForFn)pointForFn latLngForFn:(mbgl::MapSnapshotter::LatLngForFn)latLngForFn {
    self = [super init];
    if (self) {
        _context = context;
        _scale = scale;
        _pointForFn = pointForFn;
        _latLngForFn = latLngForFn;
    }

    return self;
}

#if TARGET_OS_IPHONE

- (CGPoint)pointForCoordinate:(CLLocationCoordinate2D)coordinate
{
    mbgl::ScreenCoordinate sc = _pointForFn(MLNLatLngFromLocationCoordinate2D(coordinate));
    return CGPointMake(sc.x, sc.y);
}

- (CLLocationCoordinate2D)coordinateForPoint:(CGPoint)point
{
    mbgl::LatLng latLng = _latLngForFn(mbgl::ScreenCoordinate(point.x, point.y));
    return MLNLocationCoordinate2DFromLatLng(latLng);
}

#else

- (NSPoint)pointForCoordinate:(CLLocationCoordinate2D)coordinate
{
    mbgl::ScreenCoordinate sc = _pointForFn(MLNLatLngFromLocationCoordinate2D(coordinate));
    CGFloat height = ((CGFloat)CGBitmapContextGetHeight(self.context))/self.scale;
    return NSMakePoint(sc.x, height - sc.y);
}

- (CLLocationCoordinate2D)coordinateForPoint:(NSPoint)point
{
    CGFloat height = ((CGFloat)CGBitmapContextGetHeight(self.context))/self.scale;
    auto screenCoord = mbgl::ScreenCoordinate(point.x, height - point.y);
    mbgl::LatLng latLng = _latLngForFn(screenCoord);
    return MLNLocationCoordinate2DFromLatLng(latLng);
}

#endif
@end

@implementation MLNMapSnapshotOptions

- (instancetype _Nonnull)initWithStyleURL:(nullable NSURL *)styleURL camera:(MLNMapCamera *)camera size:(CGSize)size
{
    MLNLogDebug(@"Initializing withStyleURL: %@ camera: %@ size: %@", styleURL, camera, MLNStringFromSize(size));
    self = [super init];
    if (self)
    {
        if ( !styleURL)
        {
            styleURL = [MLNStyle defaultStyleURL];
        }
        _styleURL = styleURL;
        _size = size;
        _camera = camera;
        _showsLogo = YES;
        _showsAttribution = YES;
#if TARGET_OS_IPHONE
        _scale = [UIScreen mainScreen].scale;
#else
        _scale = [NSScreen mainScreen].backingScaleFactor;
#endif
    }
    return self;
}

- (nonnull id)copyWithZone:(nullable NSZone *)zone {
    __typeof__(self) copy = [[[self class] alloc] initWithStyleURL:_styleURL camera:_camera size:_size];
    copy.zoomLevel = _zoomLevel;
    copy.coordinateBounds = _coordinateBounds;
    copy.scale = _scale;
    copy.showsLogo = _showsLogo;
    copy.showsAttribution = _showsAttribution;
    return copy;
}

@end

@interface MLNMapSnapshot() <MLNMapSnapshotProtocol>
- (instancetype)initWithImage:(nullable MLNImage *)image scale:(CGFloat)scale pointForFn:(mbgl::MapSnapshotter::PointForFn)pointForFn latLngForFn:(mbgl::MapSnapshotter::LatLngForFn)latLngForFn;

@property (nonatomic) CGFloat scale;
@end

@implementation MLNMapSnapshot {
    mbgl::MapSnapshotter::PointForFn _pointForFn;
    mbgl::MapSnapshotter::LatLngForFn _latLngForFn;
}

- (instancetype)initWithImage:(nullable MLNImage *)image scale:(CGFloat)scale pointForFn:(mbgl::MapSnapshotter::PointForFn)pointForFn latLngForFn:(mbgl::MapSnapshotter::LatLngForFn)latLngForFn
{
    self = [super init];
    if (self) {
        _pointForFn = pointForFn;
        _latLngForFn = latLngForFn;
        _scale = scale;
        _image = image;
    }
    return self;
}

#if TARGET_OS_IPHONE

- (CGPoint)pointForCoordinate:(CLLocationCoordinate2D)coordinate
{
    mbgl::ScreenCoordinate sc = _pointForFn(MLNLatLngFromLocationCoordinate2D(coordinate));
    return CGPointMake(sc.x, sc.y);
}

- (CLLocationCoordinate2D)coordinateForPoint:(CGPoint)point
{
    mbgl::LatLng latLng = _latLngForFn(mbgl::ScreenCoordinate(point.x, point.y));
    return MLNLocationCoordinate2DFromLatLng(latLng);
}

#else

- (NSPoint)pointForCoordinate:(CLLocationCoordinate2D)coordinate
{
    mbgl::ScreenCoordinate sc = _pointForFn(MLNLatLngFromLocationCoordinate2D(coordinate));
    return NSMakePoint(sc.x, self.image.size.height - sc.y);
}

- (CLLocationCoordinate2D)coordinateForPoint:(NSPoint)point
{
    auto screenCoord = mbgl::ScreenCoordinate(point.x, self.image.size.height - point.y);
    mbgl::LatLng latLng = _latLngForFn(screenCoord);
    return MLNLocationCoordinate2DFromLatLng(latLng);
}

#endif

@end

@interface MLNMapSnapshotter() <MLNMultiPointDelegate>
@property (nonatomic) BOOL loading;
@property (nonatomic) BOOL terminated;
@end

@implementation MLNMapSnapshotter {
    std::unique_ptr<mbgl::MapSnapshotter> _mbglMapSnapshotter;
    std::unique_ptr<MLNMapSnapshotterDelegateHost> _delegateHost;
    NSMutableArray<id<MLNAnnotation>> *_annotationsToInstall;
    NSMutableSet<NSString *> *_installedImages;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [self cancel];
}

- (instancetype)init {
    NSAssert(NO, @"Please use -[MLNMapSnapshotter initWithOptions:]");
    [super doesNotRecognizeSelector:_cmd];
    return nil;
}

- (instancetype)initWithOptions:(MLNMapSnapshotOptions *)options
{
    MLNLogDebug(@"Initializing withOptions: %@", options);
    self = [super init];
    if (self) {
        self.options = options;
        _installedImages = [NSMutableSet set];
#if TARGET_OS_IPHONE
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillTerminate:) name:UIApplicationWillTerminateNotification object:nil];
#else
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillTerminate:) name:NSApplicationWillTerminateNotification object:nil];
#endif
    }
    return self;
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
    [self cancel];
    self.terminated = YES;
}

- (void)addAnnotation:(id<MLNAnnotation>)annotation
{
    MLNLogDebug(@"Adding annotation: %@", annotation);
    if ( ! annotation) return;
    return [self addAnnotations:@[annotation]];
}

- (void)addAnnotations:(NSArray<id<MLNAnnotation>> *)annotations {
    MLNLogDebug(@"Adding: %lu annotations", annotations.count);

    if (!_annotationsToInstall) {
        _annotationsToInstall = [NSMutableArray array];
    }

    [_annotationsToInstall addObjectsFromArray:annotations];
}

- (void)installAnnotations:(NSArray<id<MLNAnnotation>> *)annotations
{
    MLNLogDebug(@"install: %lu annotations", annotations.count);
    if ( ! annotations) return;

    for (id <MLNAnnotation> annotation in annotations)
    {
        MLNAssert([annotation conformsToProtocol:@protocol(MLNAnnotation)], @"annotation should conform to MLNAnnotation");

        if ([annotation isKindOfClass:[MLNMultiPoint class]])
        {
            MLNMultiPoint *multiPoint = (MLNMultiPoint *)annotation;
            if (!multiPoint.pointCount) {
                continue;
            }
            _mbglMapSnapshotter->addAnnotation([multiPoint annotationObjectWithDelegate:self]);
        } else if (! [annotation isKindOfClass:[MLNMultiPolyline class]]
                   && ![annotation isKindOfClass:[MLNMultiPolygon class]]
                   && ![annotation isKindOfClass:[MLNShapeCollection class]]
                   && ![annotation isKindOfClass:[MLNPointCollection class]]) {
            NSString *symbolName;
            NSValue *annotationValue = [NSValue valueWithNonretainedObject:annotation];
            MLNAnnotationImage *annotationImage;

            if ([self.delegate respondsToSelector:@selector(mapSnapshotter:imageForAnnotation:)]) {
                annotationImage = [self.delegate mapSnapshotter:self imageForAnnotation:annotation];
            }
            if (!annotationImage) {
                // doesn't support no icon point for snapshotter
                continue;
            }

            symbolName = annotationImage.styleIconIdentifier;
            if ( ! symbolName)
            {
                symbolName = [MLNSnapshotterAnnotationSpritePrefix stringByAppendingString:annotationImage.reuseIdentifier];
                annotationImage.styleIconIdentifier = symbolName;
            }

            [self installAnnotationImage:annotationImage];

            _mbglMapSnapshotter->addAnnotation(mbgl::SymbolAnnotation {
                MLNPointFromLocationCoordinate2D(annotation.coordinate),
                symbolName.UTF8String
            });
        }
    }
}

- (void)installAnnotationImage:(MLNAnnotationImage *)annotationImage {
    NSString *iconIdentifier = annotationImage.styleIconIdentifier;
    if (![_installedImages containsObject:iconIdentifier]) {
        _mbglMapSnapshotter->addAnnotationImage([annotationImage.image mgl_styleImageWithIdentifier:iconIdentifier]);
    }
}

- (void)startWithCompletionHandler:(MLNMapSnapshotCompletionHandler)completion
{
    MLNLogDebug(@"Starting withCompletionHandler: %@", completion);
    [self startWithQueue:dispatch_get_main_queue() completionHandler:completion];
}

- (void)startWithQueue:(dispatch_queue_t)queue completionHandler:(MLNMapSnapshotCompletionHandler)completionHandler {
    [self startWithQueue:queue overlayHandler:nil completionHandler:completionHandler];
}

- (void)startWithOverlayHandler:(MLNMapSnapshotOverlayHandler)overlayHandler completionHandler:(MLNMapSnapshotCompletionHandler)completion {
    [self startWithQueue:dispatch_get_main_queue() overlayHandler:overlayHandler completionHandler:completion];
}

- (void)startWithQueue:(dispatch_queue_t)queue overlayHandler:(MLNMapSnapshotOverlayHandler)overlayHandler completionHandler:(MLNMapSnapshotCompletionHandler)completion
{
    if (!completion) {
        return;
    }

    // Ensure that offline storage has been initialized on the main thread, as MLNMapView and MLNOfflineStorage do when used directly.
    // https://github.com/mapbox/mapbox-gl-native-ios/issues/227
    if ([NSThread.currentThread isMainThread]) {
        (void)[MLNOfflineStorage sharedOfflineStorage];
    } else {
        [NSException raise:NSInvalidArgumentException
                    format:@"-[MLNMapSnapshotter startWithQueue:completionHandler:] must be called from the main thread, not %@.", NSThread.currentThread];
    }

    if (self.loading) {
        // Consider replacing this exception with an error passed to the completion block.
        [NSException raise:NSInternalInconsistencyException
                    format:@"Already started this snapshotter."];
    }
    self.loading = YES;

    if (self.terminated) {
        [NSException raise:NSInternalInconsistencyException
                    format:@"Starting a snapshotter after application termination is not supported."];
    }

    MLNMapSnapshotOptions *options = [self.options copy];
    [self configureWithOptions:options];
    MLNLogDebug(@"Starting with options: %@", self.options);

    // Temporarily capture the snapshotter until the completion handler finishes executing, to keep standalone local usage of the snapshotter from becoming a no-op.
    // POSTCONDITION: Only refer to this variable in the final result queue.
    // POSTCONDITION: It is important to nil out this variable at some point in the future to avoid a leak. In cases where the completion handler gets called, the variable should be nilled out explicitly. If -cancel is called, mbgl releases the snapshot block below, causing the only remaining references to the snapshotter to go out of scope.
    __block MLNMapSnapshotter *strongSelf = self;
    _mbglMapSnapshotter->snapshot(^(std::exception_ptr mbglError, mbgl::PremultipliedImage image, mbgl::MapSnapshotter::Attributions attributions, mbgl::MapSnapshotter::PointForFn pointForFn, mbgl::MapSnapshotter::LatLngForFn latLngForFn) {
        if (mbglError) {
            NSString *description = @(mbgl::util::toString(mbglError).c_str());
            NSDictionary *userInfo = @{NSLocalizedDescriptionKey: description};
            NSError *error = [NSError errorWithDomain:MLNErrorDomain code:MLNErrorCodeSnapshotFailed userInfo:userInfo];

            // Dispatch to result queue
            dispatch_async(queue, ^{
                strongSelf.loading = NO;
                completion(nil, error);
                strongSelf = nil;
            });
        } else {
#if TARGET_OS_IPHONE
            MLNImage *mglImage = [[MLNImage alloc] initWithMLNPremultipliedImage:std::move(image) scale:options.scale];
#else
            MLNImage *mglImage = [[MLNImage alloc] initWithMLNPremultipliedImage:std::move(image)];
            mglImage.size = NSMakeSize(mglImage.size.width / options.scale,
                                       mglImage.size.height / options.scale);
#endif
            // Process image watermark in a work queue
            dispatch_queue_t workQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

            dispatch_async(workQueue, ^{
                // Call a function that cannot accidentally capture self.
                NSError *error;
                MLNMapSnapshot *snapshot = MLNSnapshotWithDecoratedImage(mglImage, options, attributions, pointForFn, latLngForFn, overlayHandler, &error);

                // Dispatch result to result queue
                dispatch_async(queue, ^{
                    strongSelf.loading = NO;
                    completion(snapshot, error);
                    strongSelf = nil;
                });
            });
        }
    });
}

MLNImage *MLNAttributedSnapshot(mbgl::MapSnapshotter::Attributions attributions, MLNImage *mglImage, MLNMapSnapshotOptions *options, void (^overlayHandler)()) {

    NSArray<MLNAttributionInfo *> *attributionInfo = MLNAttributionInfosFromAttributions(attributions);

#if TARGET_OS_IPHONE
    MLNAttributionInfoStyle attributionInfoStyle = MLNAttributionInfoStyleLong;
    for (NSUInteger styleValue = MLNAttributionInfoStyleLong; styleValue >= MLNAttributionInfoStyleShort; styleValue--) {
        attributionInfoStyle = (MLNAttributionInfoStyle)styleValue;
        CGSize attributionSize = [MLNMapSnapshotter attributionSizeWithLogoStyle:attributionInfoStyle sourceAttributionStyle:attributionInfoStyle attributionInfo:attributionInfo];
        if (attributionSize.width <= mglImage.size.width) {
            break;
        }
    }

    UIImage *logoImage = [MLNMapSnapshotter logoImageWithStyle:attributionInfoStyle];
    CGSize attributionBackgroundSize = [MLNMapSnapshotter attributionTextSizeWithStyle:attributionInfoStyle attributionInfo:attributionInfo];

    CGRect logoImageRect = CGRectMake(MLNLogoImagePosition.x, mglImage.size.height - (MLNLogoImagePosition.y + logoImage.size.height), logoImage.size.width, logoImage.size.height);
    CGPoint attributionOrigin = CGPointMake(mglImage.size.width - 10 - attributionBackgroundSize.width,
                                            logoImageRect.origin.y + (logoImageRect.size.height / 2) - (attributionBackgroundSize.height / 2) + 1);
    if (!logoImage) {
        CGSize defaultLogoSize = [MLNMapSnapshotter maplibreLongStyleLogo].size;
        logoImageRect = CGRectMake(0, mglImage.size.height - (MLNLogoImagePosition.y + defaultLogoSize.height), 0, defaultLogoSize.height);
        attributionOrigin = CGPointMake(10, logoImageRect.origin.y + (logoImageRect.size.height / 2) - (attributionBackgroundSize.height / 2) + 1);
    }

    CGRect attributionBackgroundFrame = CGRectMake(attributionOrigin.x,
                                                   attributionOrigin.y,
                                                   attributionBackgroundSize.width,
                                                   attributionBackgroundSize.height);
    CGPoint attributionTextPosition = CGPointMake(attributionBackgroundFrame.origin.x + 10,
                                                  attributionBackgroundFrame.origin.y - 1);

    CGRect cropRect = CGRectMake(attributionBackgroundFrame.origin.x * mglImage.scale,
                                 attributionBackgroundFrame.origin.y * mglImage.scale,
                                 attributionBackgroundSize.width * mglImage.scale,
                                 attributionBackgroundSize.height * mglImage.scale);

    UIGraphicsBeginImageContextWithOptions(mglImage.size, NO, options.scale);

    [mglImage drawInRect:CGRectMake(0, 0, mglImage.size.width, mglImage.size.height)];

    overlayHandler();
    CGContextRef currentContext = UIGraphicsGetCurrentContext();

    if (!currentContext) {
        // If the current context has been corrupted by the user,
        // return nil so we can generate an error later.
        return nil;
    }

    if (options.showsLogo && logoImage) {
        [logoImage drawInRect:logoImageRect];
    }

    if (options.showsAttribution) {
        UIImage *currentImage = UIGraphicsGetImageFromCurrentImageContext();
        CGImageRef attributionImageRef = CGImageCreateWithImageInRect([currentImage CGImage], cropRect);
        UIImage *attributionImage = [UIImage imageWithCGImage:attributionImageRef];
        CGImageRelease(attributionImageRef);

        CIImage *ciAttributionImage = [[CIImage alloc] initWithCGImage:attributionImage.CGImage];

        UIImage *blurredAttributionBackground = [MLNMapSnapshotter blurredAttributionBackground:ciAttributionImage];

        [blurredAttributionBackground drawInRect:attributionBackgroundFrame];

        [MLNMapSnapshotter drawAttributionTextWithStyle:attributionInfoStyle origin:attributionTextPosition attributionInfo:attributionInfo];
    }

    UIImage *compositedImage = UIGraphicsGetImageFromCurrentImageContext();

    UIGraphicsEndImageContext();

    return compositedImage;

#else
    NSRect targetFrame = { .origin = NSZeroPoint, .size = options.size };

    MLNAttributionInfoStyle attributionInfoStyle = MLNAttributionInfoStyleLong;
    for (NSUInteger styleValue = MLNAttributionInfoStyleLong; styleValue >= MLNAttributionInfoStyleShort; styleValue--) {
        attributionInfoStyle = (MLNAttributionInfoStyle)styleValue;
        CGSize attributionSize = [MLNMapSnapshotter attributionSizeWithLogoStyle:attributionInfoStyle sourceAttributionStyle:attributionInfoStyle attributionInfo:attributionInfo];
        if (attributionSize.width <= mglImage.size.width) {
            break;
        }
    }

    NSImage *logoImage = [MLNMapSnapshotter logoImageWithStyle:attributionInfoStyle];
    CGSize attributionBackgroundSize = [MLNMapSnapshotter attributionTextSizeWithStyle:attributionInfoStyle attributionInfo:attributionInfo];
    NSImage *sourceImage = mglImage;

    CGRect logoImageRect = CGRectMake(MLNLogoImagePosition.x, MLNLogoImagePosition.y, logoImage.size.width, logoImage.size.height);
    CGPoint attributionOrigin = CGPointMake(targetFrame.size.width - 10 - attributionBackgroundSize.width,
                                            MLNLogoImagePosition.y + 1);
    if (!logoImage) {
        CGSize defaultLogoSize = [MLNMapSnapshotter maplibreLongStyleLogo].size;
        logoImageRect = CGRectMake(0, MLNLogoImagePosition.y, 0, defaultLogoSize.height);
        attributionOrigin = CGPointMake(10, attributionOrigin.y);
    }

    CGRect attributionBackgroundFrame = CGRectMake(attributionOrigin.x,
                                                   attributionOrigin.y,
                                                   attributionBackgroundSize.width,
                                                   attributionBackgroundSize.height);
    CGPoint attributionTextPosition = CGPointMake(attributionBackgroundFrame.origin.x + 10,
                                                  logoImageRect.origin.y + (logoImageRect.size.height / 2) - (attributionBackgroundSize.height / 2));


    NSImage *compositedImage = nil;
    NSImageRep *sourceImageRep = [sourceImage bestRepresentationForRect:targetFrame
                                                                context:nil
                                                                  hints:nil];
    compositedImage = [[NSImage alloc] initWithSize:targetFrame.size];

    [compositedImage lockFocus];

    [sourceImageRep drawInRect: targetFrame];

    overlayHandler();

    NSGraphicsContext *currentContext = [NSGraphicsContext currentContext];
    if (!currentContext) {
        // If the current context has been corrupted by the user,
        // return nil so we can generate an error later.
        return nil;
    }

    if (options.showsLogo) {
        [logoImage drawInRect:logoImageRect];
    }

    if (options.showsAttribution) {
        NSBitmapImageRep *attributionBackground = [[NSBitmapImageRep alloc] initWithFocusedViewRect:attributionBackgroundFrame];

        CIImage *attributionBackgroundImage = [[CIImage alloc] initWithCGImage:[attributionBackground CGImage]];

        NSImage *blurredAttributionBackground = [MLNMapSnapshotter blurredAttributionBackground:attributionBackgroundImage];

        [blurredAttributionBackground drawInRect:attributionBackgroundFrame];

        [MLNMapSnapshotter drawAttributionTextWithStyle:attributionInfoStyle origin:attributionTextPosition attributionInfo:attributionInfo];
    }

    [compositedImage unlockFocus];

    return compositedImage;
#endif
}

MLNMapSnapshot *MLNSnapshotWithDecoratedImage(MLNImage *mglImage, MLNMapSnapshotOptions *options, mbgl::MapSnapshotter::Attributions attributions, mbgl::MapSnapshotter::PointForFn pointForFn, mbgl::MapSnapshotter::LatLngForFn latLngForFn, MLNMapSnapshotOverlayHandler overlayHandler, NSError * _Nullable *outError) {
    MLNImage *compositedImage = MLNAttributedSnapshot(attributions, mglImage, options, ^{
        if (!overlayHandler) {
            return;
        }
#if TARGET_OS_IPHONE
        CGContextRef context = UIGraphicsGetCurrentContext();
        if (!context) {
            return;
        }
        MLNMapSnapshotOverlay *snapshotOverlay = [[MLNMapSnapshotOverlay alloc] initWithContext:context
                                                                                          scale:options.scale
                                                                                     pointForFn:pointForFn
                                                                                    latLngForFn:latLngForFn];
        CGContextSaveGState(context);
        overlayHandler(snapshotOverlay);
        CGContextRestoreGState(context);
#else
        NSGraphicsContext *context = [NSGraphicsContext currentContext];
        if (!context) {
            return;
        }
        MLNMapSnapshotOverlay *snapshotOverlay = [[MLNMapSnapshotOverlay alloc] initWithContext:context.CGContext
                                                                                          scale:options.scale
                                                                                     pointForFn:pointForFn
                                                                                    latLngForFn:latLngForFn];
        [context saveGraphicsState];
        overlayHandler(snapshotOverlay);
        [context restoreGraphicsState];
#endif
    });

    if (compositedImage) {
        return [[MLNMapSnapshot alloc] initWithImage:compositedImage
                                               scale:options.scale
                                          pointForFn:pointForFn
                                         latLngForFn:latLngForFn];
    } else {
        if (outError) {
            NSDictionary *userInfo = @{NSLocalizedDescriptionKey: @"Failed to generate composited snapshot."};
            *outError = [NSError errorWithDomain:MLNErrorDomain
                                        code:MLNErrorCodeSnapshotFailed
                                    userInfo:userInfo];
        }
        return nil;
    }
}

NSArray<MLNAttributionInfo *> *MLNAttributionInfosFromAttributions(mbgl::MapSnapshotter::Attributions attributions) {
    NSMutableArray *infos = [NSMutableArray array];

#if TARGET_OS_IPHONE
    CGFloat fontSize = [UIFont smallSystemFontSize];
    UIColor *attributeFontColor = [UIColor blackColor];
#else
    CGFloat fontSize = [NSFont systemFontSizeForControlSize:NSControlSizeMini];
    NSColor *attributeFontColor = [NSColor blackColor];
#endif
    for (auto attribution : attributions) {
        NSString *attributionHTMLString = @(attribution.c_str());
        NSArray *tileSetInfos = [MLNAttributionInfo attributionInfosFromHTMLString:attributionHTMLString
                                                                          fontSize:fontSize
                                                                         linkColor:attributeFontColor];
        [infos growArrayByAddingAttributionInfosFromArray:tileSetInfos];
    }
    return infos;
}

+ (void)drawAttributionTextWithStyle:(MLNAttributionInfoStyle)attributionInfoStyle origin:(CGPoint)origin attributionInfo:(NSArray<MLNAttributionInfo *>*)attributionInfo
{
    for (MLNAttributionInfo *info in attributionInfo) {
        if (info.isFeedbackLink) {
            continue;
        }
        NSAttributedString *attribution = [info titleWithStyle:attributionInfoStyle];
        [attribution drawAtPoint:origin];

        origin.x += [attribution size].width + 10;
    }
}

+ (MLNImage *)blurredAttributionBackground:(CIImage *)backgroundImage
{
    CGAffineTransform transform = CGAffineTransformIdentity;
    CIFilter *clamp = [CIFilter filterWithName:@"CIAffineClamp"];
    [clamp setValue:backgroundImage forKey:kCIInputImageKey];
    [clamp setValue:[NSValue valueWithBytes:&transform objCType:@encode(CGAffineTransform)] forKey:@"inputTransform"];

    CIFilter *attributionBlurFilter = [CIFilter filterWithName:@"CIGaussianBlur"];
    [attributionBlurFilter setValue:[clamp outputImage] forKey:kCIInputImageKey];
    [attributionBlurFilter setValue:@10 forKey:kCIInputRadiusKey];

    CIFilter *attributionColorFilter = [CIFilter filterWithName:@"CIColorControls"];
    [attributionColorFilter setValue:[attributionBlurFilter outputImage] forKey:kCIInputImageKey];
    [attributionColorFilter setValue:@(0.1) forKey:kCIInputBrightnessKey];

    CIImage *blurredImage = attributionColorFilter.outputImage;

    CIContext *ctx = [CIContext contextWithOptions:nil];
    CGImageRef cgimg = [ctx createCGImage:blurredImage fromRect:[backgroundImage extent]];
    MLNImage *image;

#if TARGET_OS_IPHONE
    image = [UIImage imageWithCGImage:cgimg];
#else
    image = [[NSImage alloc] initWithCGImage:cgimg size:[backgroundImage extent].size];
#endif

    CGImageRelease(cgimg);
    return image;
}

+ (MLNImage *)logoImageWithStyle:(MLNAttributionInfoStyle)style
{
    MLNImage *logoImage;
    switch (style) {
        case MLNAttributionInfoStyleLong:
            logoImage = [MLNMapSnapshotter maplibreLongStyleLogo];
            break;
        case MLNAttributionInfoStyleMedium:
#if TARGET_OS_IPHONE
            logoImage = [UIImage imageNamed:@"maplibre-logo-icon" inBundle:[NSBundle mgl_frameworkBundle] compatibleWithTraitCollection:nil];
#else
            logoImage = [[NSImage alloc] initWithContentsOfFile:[[NSBundle mgl_frameworkBundle] pathForResource:@"mapbox_helmet" ofType:@"pdf"]];
#endif
            break;
        case MLNAttributionInfoStyleShort:
            logoImage = nil;
            break;
    }
    return logoImage;
}

+ (MLNImage *)maplibreLongStyleLogo
{
    MLNImage *logoImage;
#if TARGET_OS_IPHONE
    logoImage =[UIImage imageNamed:@"maplibre-logo-stroke-gray" inBundle:[NSBundle mgl_frameworkBundle] compatibleWithTraitCollection:nil];
#else
    logoImage = [[NSImage alloc] initWithContentsOfFile:[[NSBundle mgl_frameworkBundle] pathForResource:@"mapbox" ofType:@"pdf"]];
#endif
    return logoImage;
}

+ (CGSize)attributionSizeWithLogoStyle:(MLNAttributionInfoStyle)logoStyle sourceAttributionStyle:(MLNAttributionInfoStyle)attributionStyle attributionInfo:(NSArray<MLNAttributionInfo *>*)attributionInfo
{
    MLNImage *logoImage = [self logoImageWithStyle:logoStyle];

    CGSize attributionBackgroundSize = [MLNMapSnapshotter attributionTextSizeWithStyle:attributionStyle attributionInfo:attributionInfo];

    CGSize attributionSize = CGSizeZero;

    if (logoImage) {
        attributionSize.width = MLNLogoImagePosition.x + logoImage.size.width + 10;
    }
    attributionSize.width = attributionSize.width + 10 + attributionBackgroundSize.width + 10;
    attributionSize.height = MAX(logoImage.size.height, attributionBackgroundSize.height);

    return attributionSize;
}

+ (CGSize)attributionTextSizeWithStyle:(MLNAttributionInfoStyle)attributionStyle attributionInfo:(NSArray<MLNAttributionInfo *>*)attributionInfo
{
    CGSize attributionBackgroundSize = CGSizeMake(10, 0);
    for (MLNAttributionInfo *info in attributionInfo) {
        if (info.isFeedbackLink) {
            continue;
        }
        CGSize attributionSize = [info titleWithStyle:attributionStyle].size;
        attributionBackgroundSize.width += attributionSize.width + 10;
        attributionBackgroundSize.height = MAX(attributionSize.height, attributionBackgroundSize.height);
    }

    return attributionBackgroundSize;
}

- (void)cancel
{
    MLNLogInfo(@"Cancelling snapshotter.");

    if (_mbglMapSnapshotter) {
        _mbglMapSnapshotter->cancel();
    }
    _mbglMapSnapshotter.reset();
    _delegateHost.reset();
}

- (void)configureWithOptions:(MLNMapSnapshotOptions *)options {
    auto mbglFileSource = [[MLNOfflineStorage sharedOfflineStorage] mbglFileSource];

    // Size; taking into account the minimum texture size for OpenGL ES
    // For non retina screens the ratio is 1:1 MLNSnapshotterMinimumPixelSize
    mbgl::Size size = {
        static_cast<uint32_t>(MAX(options.size.width, MLNSnapshotterMinimumPixelSize)),
        static_cast<uint32_t>(MAX(options.size.height, MLNSnapshotterMinimumPixelSize))
    };

    float pixelRatio = MAX(options.scale, 1);

    // App-global configuration
    MLNRendererConfiguration *config = [MLNRendererConfiguration currentConfiguration];

    auto tileServerOptions = [[MLNSettings sharedSettings] tileServerOptionsInternal];
    mbgl::ResourceOptions resourceOptions;
    resourceOptions
        .withTileServerOptions(*tileServerOptions)
        .withCachePath(MLNOfflineStorage.sharedOfflineStorage.databasePath.UTF8String)
        .withAssetPath(NSBundle.mainBundle.resourceURL.path.UTF8String);
    mbgl::ClientOptions clientOptions;

    auto apiKey = [[MLNSettings sharedSettings] apiKey];
    if (apiKey) {
        resourceOptions.withApiKey([apiKey UTF8String]);
    }

    // Create the snapshotter
    auto localFontFamilyName = config.localFontFamilyName ? std::string(config.localFontFamilyName.UTF8String) : nullptr;
    _delegateHost = std::make_unique<MLNMapSnapshotterDelegateHost>(self);
    _mbglMapSnapshotter = std::make_unique<mbgl::MapSnapshotter>(
                                                                 size, pixelRatio, resourceOptions, clientOptions, *_delegateHost, localFontFamilyName);

    _mbglMapSnapshotter->setStyleURL(std::string(options.styleURL.absoluteString.UTF8String));

    // Camera options
    mbgl::CameraOptions cameraOptions;
    if (CLLocationCoordinate2DIsValid(options.camera.centerCoordinate)) {
        cameraOptions.center = MLNLatLngFromLocationCoordinate2D(options.camera.centerCoordinate);
    }
    if (options.camera.heading >= 0) {
        cameraOptions.bearing = MAX(0, options.camera.heading);
    }
    if (options.zoomLevel >= 0) {
        cameraOptions.zoom = MAX(0, options.zoomLevel);
    }
    if (options.camera.pitch >= 0) {
        cameraOptions.pitch = MAX(0, options.camera.pitch);
    }
    if (cameraOptions != mbgl::CameraOptions()) {
        _mbglMapSnapshotter->setCameraOptions(cameraOptions);
    }

    // Region
    if (!MLNCoordinateBoundsIsEmpty(options.coordinateBounds)) {
        _mbglMapSnapshotter->setRegion(MLNLatLngBoundsFromCoordinateBounds(options.coordinateBounds));
    }

    // Annotations
    if (_annotationsToInstall) {
        [self installAnnotations:_annotationsToInstall];
    }
}

- (MLNStyle *)style {
    if (!_mbglMapSnapshotter) {
        return nil;
    }
    return [[MLNStyle alloc] initWithRawStyle:&_mbglMapSnapshotter->getStyle() stylable:self];
}

- (double)alphaForShapeAnnotation:(MLNShape *)annotation
{
    if ([self.delegate respondsToSelector:@selector(mapSnapshotter:alphaForShapeAnnotation:)]) {
        return [self.delegate mapSnapshotter:self alphaForShapeAnnotation:annotation];
    }
    return 1.0;
}

- (mbgl::Color)strokeColorForShapeAnnotation:(MLNShape *)annotation
{
    if ([self.delegate respondsToSelector:@selector(mapSnapshotter:strokeColorForShapeAnnotation:)]) {
        MLNColor *color = [self.delegate mapSnapshotter:self strokeColorForShapeAnnotation:annotation];
        return color.mgl_color;
    }

    return mbgl::Color::white();
}

- (mbgl::Color)fillColorForPolygonAnnotation:(MLNPolygon *)annotation
{
    if ([self.delegate respondsToSelector:@selector(mapSnapshotter:fillColorForPolygonAnnotation:)]){
        MLNColor *color = [self.delegate mapSnapshotter:self fillColorForPolygonAnnotation:annotation];
        return color.mgl_color;
    }
    return mbgl::Color::white();
}

- (CGFloat)lineWidthForPolylineAnnotation:(MLNPolyline *)annotation
{
    if ([self.delegate respondsToSelector:@selector(mapSnapshotter:lineWidthForPolylineAnnotation:)]) {
        return [self.delegate mapSnapshotter:self lineWidthForPolylineAnnotation:annotation];
    }

    return 3.0;
}

@end
