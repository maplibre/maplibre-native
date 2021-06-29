#import "MGLMapSnapshotter.h"

#import <mbgl/actor/actor.hpp>
#import <mbgl/actor/scheduler.hpp>
#import <mbgl/util/geo.hpp>
#import <mbgl/map/map_options.hpp>
#import <mbgl/map/map_snapshotter.hpp>
#import <mbgl/map/camera.hpp>
#import <mbgl/storage/resource_options.hpp>
#import <mbgl/util/string.hpp>

#import "MGLOfflineStorage_Private.h"
#import "MGLGeometry_Private.h"
#import "MGLStyle_Private.h"
#import "MGLAttributionInfo_Private.h"
#import "MGLLoggingConfiguration_Private.h"
#import "MGLRendererConfiguration.h"
#import "MGLMapSnapshotter_Private.h"
#import "MGLSettings_Private.h"

#if TARGET_OS_IPHONE
#import "UIImage+MGLAdditions.h"
#else
#import "NSImage+MGLAdditions.h"
#import <CoreGraphics/CoreGraphics.h>
#import <QuartzCore/QuartzCore.h>
#endif

#import "NSBundle+MGLAdditions.h"

const CGPoint MGLLogoImagePosition = CGPointMake(8, 8);
const CGFloat MGLSnapshotterMinimumPixelSize = 64;

MGLImage *MGLAttributedSnapshot(mbgl::MapSnapshotter::Attributions attributions, MGLImage *mglImage, mbgl::MapSnapshotter::PointForFn pointForFn, mbgl::MapSnapshotter::LatLngForFn latLngForFn, MGLMapSnapshotOptions *options, MGLMapSnapshotOverlayHandler overlayHandler);
MGLMapSnapshot *MGLSnapshotWithDecoratedImage(MGLImage *mglImage, MGLMapSnapshotOptions *options, mbgl::MapSnapshotter::Attributions attributions, mbgl::MapSnapshotter::PointForFn pointForFn, mbgl::MapSnapshotter::LatLngForFn latLngForFn, MGLMapSnapshotOverlayHandler overlayHandler, NSError * _Nullable *outError);
NSArray<MGLAttributionInfo *> *MGLAttributionInfosFromAttributions(mbgl::MapSnapshotter::Attributions attributions);

class MGLMapSnapshotterDelegateHost: public mbgl::MapSnapshotterObserver {
public:
    MGLMapSnapshotterDelegateHost(MGLMapSnapshotter *snapshotter_) : snapshotter(snapshotter_) {}
    
    void onDidFailLoadingStyle(const std::string& errorMessage) {
        MGLMapSnapshotter *strongSnapshotter = snapshotter;
        if ([strongSnapshotter.delegate respondsToSelector:@selector(mapSnapshotterDidFail:withError:)]) {
            NSString *description = @(errorMessage.c_str());
            NSDictionary *userInfo = @{
                NSLocalizedDescriptionKey: NSLocalizedStringWithDefaultValue(@"SNAPSHOT_LOAD_STYLE_FAILED_DESC", nil, nil, @"The snapshot failed because the style can’t be loaded.", @"User-friendly error description"),
                NSLocalizedFailureReasonErrorKey: description,
            };
            NSError *error = [NSError errorWithDomain:MGLErrorDomain code:MGLErrorCodeLoadStyleFailed userInfo:userInfo];
            [strongSnapshotter.delegate mapSnapshotterDidFail:snapshotter withError:error];
        }
    }
    
    void onDidFinishLoadingStyle() {
        MGLMapSnapshotter *strongSnapshotter = snapshotter;
        if ([strongSnapshotter.delegate respondsToSelector:@selector(mapSnapshotter:didFinishLoadingStyle:)]) {
            [strongSnapshotter.delegate mapSnapshotter:snapshotter didFinishLoadingStyle:snapshotter.style];
        }
    }
    
    void onStyleImageMissing(const std::string& imageName) {
        MGLMapSnapshotter *strongSnapshotter = snapshotter;
        if ([strongSnapshotter.delegate respondsToSelector:@selector(mapSnapshotter:didFailLoadingImageNamed:)]) {
            [strongSnapshotter.delegate mapSnapshotter:snapshotter didFailLoadingImageNamed:@(imageName.c_str())];
        }
    }
    
private:
    __weak MGLMapSnapshotter *snapshotter;
};

@interface MGLMapSnapshotOverlay() <MGLMapSnapshotProtocol>
@property (nonatomic, assign) CGFloat scale;
- (instancetype)initWithContext:(CGContextRef)context scale:(CGFloat)scale pointForFn:(mbgl::MapSnapshotter::PointForFn)pointForFn latLngForFn:(mbgl::MapSnapshotter::LatLngForFn)latLngForFn;

@end

@implementation MGLMapSnapshotOverlay {
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
    mbgl::ScreenCoordinate sc = _pointForFn(MGLLatLngFromLocationCoordinate2D(coordinate));
    return CGPointMake(sc.x, sc.y);
}

- (CLLocationCoordinate2D)coordinateForPoint:(CGPoint)point
{
    mbgl::LatLng latLng = _latLngForFn(mbgl::ScreenCoordinate(point.x, point.y));
    return MGLLocationCoordinate2DFromLatLng(latLng);
}

#else

- (NSPoint)pointForCoordinate:(CLLocationCoordinate2D)coordinate
{
    mbgl::ScreenCoordinate sc = _pointForFn(MGLLatLngFromLocationCoordinate2D(coordinate));
    CGFloat height = ((CGFloat)CGBitmapContextGetHeight(self.context))/self.scale;
    return NSMakePoint(sc.x, height - sc.y);
}

- (CLLocationCoordinate2D)coordinateForPoint:(NSPoint)point
{
    CGFloat height = ((CGFloat)CGBitmapContextGetHeight(self.context))/self.scale;
    auto screenCoord = mbgl::ScreenCoordinate(point.x, height - point.y);
    mbgl::LatLng latLng = _latLngForFn(screenCoord);
    return MGLLocationCoordinate2DFromLatLng(latLng);
}

#endif
@end

@interface MGLMapSnapshotOptions ()

/**
 :nodoc:
 Whether the Mapbox wordmark is displayed.

 @note The Mapbox terms of service, which governs the use of Mapbox-hosted
 vector tiles and styles,
 <a href="https://docs.mapbox.com/help/how-mapbox-works/attribution/">requires</a> most Mapbox
 customers to display the Mapbox wordmark. If this applies to you, do not
 hide the wordmark or change its contents.
 */
@property (nonatomic, readwrite) BOOL showsLogo;
@end

@implementation MGLMapSnapshotOptions

- (instancetype _Nonnull)initWithStyleURL:(nullable NSURL *)styleURL camera:(MGLMapCamera *)camera size:(CGSize)size
{
    MGLLogDebug(@"Initializing withStyleURL: %@ camera: %@ size: %@", styleURL, camera, MGLStringFromSize(size));
    self = [super init];
    if (self)
    {
        if ( !styleURL)
        {
            styleURL = [MGLStyle defaultStyleURL];
        }
        _styleURL = styleURL;
        _size = size;
        _camera = camera;
        _showsLogo = YES;
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
    return copy;
}

@end

@interface MGLMapSnapshot() <MGLMapSnapshotProtocol>
- (instancetype)initWithImage:(nullable MGLImage *)image scale:(CGFloat)scale pointForFn:(mbgl::MapSnapshotter::PointForFn)pointForFn latLngForFn:(mbgl::MapSnapshotter::LatLngForFn)latLngForFn;

@property (nonatomic) CGFloat scale;
@end

@implementation MGLMapSnapshot {
    mbgl::MapSnapshotter::PointForFn _pointForFn;
    mbgl::MapSnapshotter::LatLngForFn _latLngForFn;
}

- (instancetype)initWithImage:(nullable MGLImage *)image scale:(CGFloat)scale pointForFn:(mbgl::MapSnapshotter::PointForFn)pointForFn latLngForFn:(mbgl::MapSnapshotter::LatLngForFn)latLngForFn
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
    mbgl::ScreenCoordinate sc = _pointForFn(MGLLatLngFromLocationCoordinate2D(coordinate));
    return CGPointMake(sc.x, sc.y);
}

- (CLLocationCoordinate2D)coordinateForPoint:(CGPoint)point
{
    mbgl::LatLng latLng = _latLngForFn(mbgl::ScreenCoordinate(point.x, point.y));
    return MGLLocationCoordinate2DFromLatLng(latLng);
}

#else

- (NSPoint)pointForCoordinate:(CLLocationCoordinate2D)coordinate
{
    mbgl::ScreenCoordinate sc = _pointForFn(MGLLatLngFromLocationCoordinate2D(coordinate));
    return NSMakePoint(sc.x, self.image.size.height - sc.y);
}

- (CLLocationCoordinate2D)coordinateForPoint:(NSPoint)point
{
    auto screenCoord = mbgl::ScreenCoordinate(point.x, self.image.size.height - point.y);
    mbgl::LatLng latLng = _latLngForFn(screenCoord);
    return MGLLocationCoordinate2DFromLatLng(latLng);
}

#endif

@end

@interface MGLMapSnapshotter()
@property (nonatomic) BOOL loading;
@property (nonatomic) BOOL terminated;
@end

@implementation MGLMapSnapshotter {
    std::unique_ptr<mbgl::MapSnapshotter> _mbglMapSnapshotter;
    std::unique_ptr<MGLMapSnapshotterDelegateHost> _delegateHost;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [self cancel];
}

- (instancetype)init {
    NSAssert(NO, @"Please use -[MGLMapSnapshotter initWithOptions:]");
    [super doesNotRecognizeSelector:_cmd];
    return nil;
}

- (instancetype)initWithOptions:(MGLMapSnapshotOptions *)options
{
    MGLLogDebug(@"Initializing withOptions: %@", options);
    self = [super init];
    if (self) {
        self.options = options;
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

- (void)startWithCompletionHandler:(MGLMapSnapshotCompletionHandler)completion
{
    MGLLogDebug(@"Starting withCompletionHandler: %@", completion);
    [self startWithQueue:dispatch_get_main_queue() completionHandler:completion];
}

- (void)startWithQueue:(dispatch_queue_t)queue completionHandler:(MGLMapSnapshotCompletionHandler)completionHandler {
    [self startWithQueue:queue overlayHandler:nil completionHandler:completionHandler];
}

- (void)startWithOverlayHandler:(MGLMapSnapshotOverlayHandler)overlayHandler completionHandler:(MGLMapSnapshotCompletionHandler)completion {
    [self startWithQueue:dispatch_get_main_queue() overlayHandler:overlayHandler completionHandler:completion];
}

- (void)startWithQueue:(dispatch_queue_t)queue overlayHandler:(MGLMapSnapshotOverlayHandler)overlayHandler completionHandler:(MGLMapSnapshotCompletionHandler)completion
{
    if (!completion) {
        return;
    }
    
    // Ensure that offline storage has been initialized on the main thread, as MGLMapView and MGLOfflineStorage do when used directly.
    // https://github.com/mapbox/mapbox-gl-native-ios/issues/227
    if ([NSThread.currentThread isMainThread]) {
        (void)[MGLOfflineStorage sharedOfflineStorage];
    } else {
        [NSException raise:NSInvalidArgumentException
                    format:@"-[MGLMapSnapshotter startWithQueue:completionHandler:] must be called from the main thread, not %@.", NSThread.currentThread];
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
    
    MGLMapSnapshotOptions *options = [self.options copy];
    [self configureWithOptions:options];
    MGLLogDebug(@"Starting with options: %@", self.options);

    // Temporarily capture the snapshotter until the completion handler finishes executing, to keep standalone local usage of the snapshotter from becoming a no-op.
    // POSTCONDITION: Only refer to this variable in the final result queue.
    // POSTCONDITION: It is important to nil out this variable at some point in the future to avoid a leak. In cases where the completion handler gets called, the variable should be nilled out explicitly. If -cancel is called, mbgl releases the snapshot block below, causing the only remaining references to the snapshotter to go out of scope.
    __block MGLMapSnapshotter *strongSelf = self;
    _mbglMapSnapshotter->snapshot(^(std::exception_ptr mbglError, mbgl::PremultipliedImage image, mbgl::MapSnapshotter::Attributions attributions, mbgl::MapSnapshotter::PointForFn pointForFn, mbgl::MapSnapshotter::LatLngForFn latLngForFn) {
        if (mbglError) {
            NSString *description = @(mbgl::util::toString(mbglError).c_str());
            NSDictionary *userInfo = @{NSLocalizedDescriptionKey: description};
            NSError *error = [NSError errorWithDomain:MGLErrorDomain code:MGLErrorCodeSnapshotFailed userInfo:userInfo];

            // Dispatch to result queue
            dispatch_async(queue, ^{
                strongSelf.loading = NO;
                completion(nil, error);
                strongSelf = nil;
            });
        } else {
#if TARGET_OS_IPHONE
            MGLImage *mglImage = [[MGLImage alloc] initWithMGLPremultipliedImage:std::move(image) scale:options.scale];
#else
            MGLImage *mglImage = [[MGLImage alloc] initWithMGLPremultipliedImage:std::move(image)];
            mglImage.size = NSMakeSize(mglImage.size.width / options.scale,
                                       mglImage.size.height / options.scale);
#endif
            // Process image watermark in a work queue
            dispatch_queue_t workQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
            
            dispatch_async(workQueue, ^{
                // Call a function that cannot accidentally capture self.
                NSError *error;
                MGLMapSnapshot *snapshot = MGLSnapshotWithDecoratedImage(mglImage, options, attributions, pointForFn, latLngForFn, overlayHandler, &error);
                
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

MGLImage *MGLAttributedSnapshot(mbgl::MapSnapshotter::Attributions attributions, MGLImage *mglImage, MGLMapSnapshotOptions *options, void (^overlayHandler)()) {

    NSArray<MGLAttributionInfo *> *attributionInfo = MGLAttributionInfosFromAttributions(attributions);

#if TARGET_OS_IPHONE
    MGLAttributionInfoStyle attributionInfoStyle = MGLAttributionInfoStyleLong;
    for (NSUInteger styleValue = MGLAttributionInfoStyleLong; styleValue >= MGLAttributionInfoStyleShort; styleValue--) {
        attributionInfoStyle = (MGLAttributionInfoStyle)styleValue;
        CGSize attributionSize = [MGLMapSnapshotter attributionSizeWithLogoStyle:attributionInfoStyle sourceAttributionStyle:attributionInfoStyle attributionInfo:attributionInfo];
        if (attributionSize.width <= mglImage.size.width) {
            break;
        }
    }

    UIImage *logoImage = [MGLMapSnapshotter logoImageWithStyle:attributionInfoStyle];
    CGSize attributionBackgroundSize = [MGLMapSnapshotter attributionTextSizeWithStyle:attributionInfoStyle attributionInfo:attributionInfo];
    
    CGRect logoImageRect = CGRectMake(MGLLogoImagePosition.x, mglImage.size.height - (MGLLogoImagePosition.y + logoImage.size.height), logoImage.size.width, logoImage.size.height);
    CGPoint attributionOrigin = CGPointMake(mglImage.size.width - 10 - attributionBackgroundSize.width,
                                            logoImageRect.origin.y + (logoImageRect.size.height / 2) - (attributionBackgroundSize.height / 2) + 1);
    if (!logoImage) {
        CGSize defaultLogoSize = [MGLMapSnapshotter mapboxLongStyleLogo].size;
        logoImageRect = CGRectMake(0, mglImage.size.height - (MGLLogoImagePosition.y + defaultLogoSize.height), 0, defaultLogoSize.height);
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

    if (options.showsLogo) {
        [logoImage drawInRect:logoImageRect];
    }
    
    UIImage *currentImage = UIGraphicsGetImageFromCurrentImageContext();
    CGImageRef attributionImageRef = CGImageCreateWithImageInRect([currentImage CGImage], cropRect);
    UIImage *attributionImage = [UIImage imageWithCGImage:attributionImageRef];
    CGImageRelease(attributionImageRef);
    
    CIImage *ciAttributionImage = [[CIImage alloc] initWithCGImage:attributionImage.CGImage];
    
    UIImage *blurredAttributionBackground = [MGLMapSnapshotter blurredAttributionBackground:ciAttributionImage];
    
    [blurredAttributionBackground drawInRect:attributionBackgroundFrame];
    
    [MGLMapSnapshotter drawAttributionTextWithStyle:attributionInfoStyle origin:attributionTextPosition attributionInfo:attributionInfo];
    
    UIImage *compositedImage = UIGraphicsGetImageFromCurrentImageContext();
    
    UIGraphicsEndImageContext();

    return compositedImage;

#else
    NSRect targetFrame = { .origin = NSZeroPoint, .size = options.size };
    
    MGLAttributionInfoStyle attributionInfoStyle = MGLAttributionInfoStyleLong;
    for (NSUInteger styleValue = MGLAttributionInfoStyleLong; styleValue >= MGLAttributionInfoStyleShort; styleValue--) {
        attributionInfoStyle = (MGLAttributionInfoStyle)styleValue;
        CGSize attributionSize = [MGLMapSnapshotter attributionSizeWithLogoStyle:attributionInfoStyle sourceAttributionStyle:attributionInfoStyle attributionInfo:attributionInfo];
        if (attributionSize.width <= mglImage.size.width) {
            break;
        }
    }
    
    NSImage *logoImage = [MGLMapSnapshotter logoImageWithStyle:attributionInfoStyle];
    CGSize attributionBackgroundSize = [MGLMapSnapshotter attributionTextSizeWithStyle:attributionInfoStyle attributionInfo:attributionInfo];
    NSImage *sourceImage = mglImage;
    
    CGRect logoImageRect = CGRectMake(MGLLogoImagePosition.x, MGLLogoImagePosition.y, logoImage.size.width, logoImage.size.height);
    CGPoint attributionOrigin = CGPointMake(targetFrame.size.width - 10 - attributionBackgroundSize.width,
                                            MGLLogoImagePosition.y + 1);
    if (!logoImage) {
        CGSize defaultLogoSize = [MGLMapSnapshotter mapboxLongStyleLogo].size;
        logoImageRect = CGRectMake(0, MGLLogoImagePosition.y, 0, defaultLogoSize.height);
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
    
    if (logoImage) {
        [logoImage drawInRect:logoImageRect];
    }
    
    NSBitmapImageRep *attributionBackground = [[NSBitmapImageRep alloc] initWithFocusedViewRect:attributionBackgroundFrame];
    
    CIImage *attributionBackgroundImage = [[CIImage alloc] initWithCGImage:[attributionBackground CGImage]];
    
    NSImage *blurredAttributionBackground = [MGLMapSnapshotter blurredAttributionBackground:attributionBackgroundImage];
    
    [blurredAttributionBackground drawInRect:attributionBackgroundFrame];
    
    [MGLMapSnapshotter drawAttributionTextWithStyle:attributionInfoStyle origin:attributionTextPosition attributionInfo:attributionInfo];
    
    [compositedImage unlockFocus];

    return compositedImage;
#endif
}

MGLMapSnapshot *MGLSnapshotWithDecoratedImage(MGLImage *mglImage, MGLMapSnapshotOptions *options, mbgl::MapSnapshotter::Attributions attributions, mbgl::MapSnapshotter::PointForFn pointForFn, mbgl::MapSnapshotter::LatLngForFn latLngForFn, MGLMapSnapshotOverlayHandler overlayHandler, NSError * _Nullable *outError) {
    MGLImage *compositedImage = MGLAttributedSnapshot(attributions, mglImage, options, ^{
        if (!overlayHandler) {
            return;
        }
#if TARGET_OS_IPHONE
        CGContextRef context = UIGraphicsGetCurrentContext();
        if (!context) {
            return;
        }
        MGLMapSnapshotOverlay *snapshotOverlay = [[MGLMapSnapshotOverlay alloc] initWithContext:context
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
        MGLMapSnapshotOverlay *snapshotOverlay = [[MGLMapSnapshotOverlay alloc] initWithContext:context.CGContext
                                                                                          scale:options.scale
                                                                                     pointForFn:pointForFn
                                                                                    latLngForFn:latLngForFn];
        [context saveGraphicsState];
        overlayHandler(snapshotOverlay);
        [context restoreGraphicsState];
#endif
    });
    
    if (compositedImage) {
        return [[MGLMapSnapshot alloc] initWithImage:compositedImage
                                               scale:options.scale
                                          pointForFn:pointForFn
                                         latLngForFn:latLngForFn];
    } else {
        if (outError) {
            NSDictionary *userInfo = @{NSLocalizedDescriptionKey: @"Failed to generate composited snapshot."};
            *outError = [NSError errorWithDomain:MGLErrorDomain
                                        code:MGLErrorCodeSnapshotFailed
                                    userInfo:userInfo];
        }
        return nil;
    }
}

NSArray<MGLAttributionInfo *> *MGLAttributionInfosFromAttributions(mbgl::MapSnapshotter::Attributions attributions) {
    NSMutableArray *infos = [NSMutableArray array];
    
#if TARGET_OS_IPHONE
    CGFloat fontSize = [UIFont smallSystemFontSize];
    UIColor *attributeFontColor = [UIColor blackColor];
#else
    CGFloat fontSize = [NSFont systemFontSizeForControlSize:NSMiniControlSize];
    NSColor *attributeFontColor = [NSColor blackColor];
#endif
    for (auto attribution : attributions) {
        NSString *attributionHTMLString = @(attribution.c_str());
        NSArray *tileSetInfos = [MGLAttributionInfo attributionInfosFromHTMLString:attributionHTMLString
                                                                          fontSize:fontSize
                                                                         linkColor:attributeFontColor];
        [infos growArrayByAddingAttributionInfosFromArray:tileSetInfos];
    }
    return infos;
}

+ (void)drawAttributionTextWithStyle:(MGLAttributionInfoStyle)attributionInfoStyle origin:(CGPoint)origin attributionInfo:(NSArray<MGLAttributionInfo *>*)attributionInfo
{
    for (MGLAttributionInfo *info in attributionInfo) {
        if (info.isFeedbackLink) {
            continue;
        }
        NSAttributedString *attribution = [info titleWithStyle:attributionInfoStyle];
        [attribution drawAtPoint:origin];
        
        origin.x += [attribution size].width + 10;
    }
}

+ (MGLImage *)blurredAttributionBackground:(CIImage *)backgroundImage
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
    MGLImage *image;
    
#if TARGET_OS_IPHONE
    image = [UIImage imageWithCGImage:cgimg];
#else
    image = [[NSImage alloc] initWithCGImage:cgimg size:[backgroundImage extent].size];
#endif

    CGImageRelease(cgimg);
    return image;
}

+ (MGLImage *)logoImageWithStyle:(MGLAttributionInfoStyle)style
{
    MGLImage *logoImage;
    switch (style) {
        case MGLAttributionInfoStyleLong:
            logoImage = [MGLMapSnapshotter mapboxLongStyleLogo];
            break;
        case MGLAttributionInfoStyleMedium:
#if TARGET_OS_IPHONE
            logoImage = [UIImage imageNamed:@"mapbox_helmet" inBundle:[NSBundle mgl_frameworkBundle] compatibleWithTraitCollection:nil];
#else
            logoImage = [[NSImage alloc] initWithContentsOfFile:[[NSBundle mgl_frameworkBundle] pathForResource:@"mapbox_helmet" ofType:@"pdf"]];
#endif
            break;
        case MGLAttributionInfoStyleShort:
            logoImage = nil;
            break;
    }
    return logoImage;
}

+ (MGLImage *)mapboxLongStyleLogo
{
    MGLImage *logoImage;
#if TARGET_OS_IPHONE
    logoImage =[UIImage imageNamed:@"mapbox" inBundle:[NSBundle mgl_frameworkBundle] compatibleWithTraitCollection:nil];
#else
    logoImage = [[NSImage alloc] initWithContentsOfFile:[[NSBundle mgl_frameworkBundle] pathForResource:@"mapbox" ofType:@"pdf"]];
#endif
    return logoImage;
}

+ (CGSize)attributionSizeWithLogoStyle:(MGLAttributionInfoStyle)logoStyle sourceAttributionStyle:(MGLAttributionInfoStyle)attributionStyle attributionInfo:(NSArray<MGLAttributionInfo *>*)attributionInfo
{
    MGLImage *logoImage = [self logoImageWithStyle:logoStyle];
    
    CGSize attributionBackgroundSize = [MGLMapSnapshotter attributionTextSizeWithStyle:attributionStyle attributionInfo:attributionInfo];
    
    CGSize attributionSize = CGSizeZero;
    
    if (logoImage) {
        attributionSize.width = MGLLogoImagePosition.x + logoImage.size.width + 10;
    }
    attributionSize.width = attributionSize.width + 10 + attributionBackgroundSize.width + 10;
    attributionSize.height = MAX(logoImage.size.height, attributionBackgroundSize.height);
    
    return attributionSize;
}

+ (CGSize)attributionTextSizeWithStyle:(MGLAttributionInfoStyle)attributionStyle attributionInfo:(NSArray<MGLAttributionInfo *>*)attributionInfo
{
    CGSize attributionBackgroundSize = CGSizeMake(10, 0);
    for (MGLAttributionInfo *info in attributionInfo) {
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
    MGLLogInfo(@"Cancelling snapshotter.");
    
    if (_mbglMapSnapshotter) {
        _mbglMapSnapshotter->cancel();
    }
    _mbglMapSnapshotter.reset();
    _delegateHost.reset();
}

- (void)configureWithOptions:(MGLMapSnapshotOptions *)options {
    auto mbglFileSource = [[MGLOfflineStorage sharedOfflineStorage] mbglFileSource];
    
    // Size; taking into account the minimum texture size for OpenGL ES
    // For non retina screens the ratio is 1:1 MGLSnapshotterMinimumPixelSize
    mbgl::Size size = {
        static_cast<uint32_t>(MAX(options.size.width, MGLSnapshotterMinimumPixelSize)),
        static_cast<uint32_t>(MAX(options.size.height, MGLSnapshotterMinimumPixelSize))
    };
    
    float pixelRatio = MAX(options.scale, 1);
    
    // App-global configuration
    MGLRendererConfiguration *config = [MGLRendererConfiguration currentConfiguration];

    auto tileServerOptions = [[MGLSettings sharedSettings] tileServerOptionsInternal];
    mbgl::ResourceOptions resourceOptions;
    resourceOptions
        .withTileServerOptions(*tileServerOptions)
        .withCachePath(MGLOfflineStorage.sharedOfflineStorage.databasePath.UTF8String)
        .withAssetPath(NSBundle.mainBundle.resourceURL.path.UTF8String);
    auto apiKey = [[MGLSettings sharedSettings] apiKey];
    if (apiKey) {
        resourceOptions.withApiKey([apiKey UTF8String]);
    }                   

    // Create the snapshotter
    auto localFontFamilyName = config.localFontFamilyName ? std::string(config.localFontFamilyName.UTF8String) : nullptr;
    _delegateHost = std::make_unique<MGLMapSnapshotterDelegateHost>(self);
    _mbglMapSnapshotter = std::make_unique<mbgl::MapSnapshotter>(
                                                                 size, pixelRatio, resourceOptions, *_delegateHost, localFontFamilyName);
    
    _mbglMapSnapshotter->setStyleURL(std::string(options.styleURL.absoluteString.UTF8String));
    
    // Camera options
    mbgl::CameraOptions cameraOptions;
    if (CLLocationCoordinate2DIsValid(options.camera.centerCoordinate)) {
        cameraOptions.center = MGLLatLngFromLocationCoordinate2D(options.camera.centerCoordinate);
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
    if (!MGLCoordinateBoundsIsEmpty(options.coordinateBounds)) {
        _mbglMapSnapshotter->setRegion(MGLLatLngBoundsFromCoordinateBounds(options.coordinateBounds));
    }
}

- (MGLStyle *)style {
    if (!_mbglMapSnapshotter) {
        return nil;
    }
    return [[MGLStyle alloc] initWithRawStyle:&_mbglMapSnapshotter->getStyle() stylable:self];
}

@end
