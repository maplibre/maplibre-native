#import "MLNMapView_Private.h"

#import "MLNAttributionButton.h"
#import "MLNCompassCell.h"
#import "MLNStyle.h"
#import "MLNRendererFrontend.h"
#import "MLNRendererConfiguration.h"

#import "MLNAnnotationImage_Private.h"
#import "MLNAttributionInfo_Private.h"
#import "MLNFeature_Private.h"
#import "MLNFoundation_Private.h"
#import "MLNGeometry_Private.h"
#import "MLNMultiPoint_Private.h"
#import "MLNOfflineStorage_Private.h"
#import "MLNStyle_Private.h"
#import "MLNShape_Private.h"

#import "MLNSettings.h"
#import "MLNMapCamera.h"
#import "MLNPolygon.h"
#import "MLNPolyline.h"
#import "MLNAnnotationImage.h"
#import "MLNMapViewDelegate.h"
#import "MLNImageSource.h"

#import <mbgl/map/map.hpp>
#import <mbgl/map/map_options.hpp>
#import <mbgl/style/style.hpp>
#import <mbgl/annotation/annotation.hpp>
#import <mbgl/map/camera.hpp>
#import <mbgl/style/image.hpp>
#import <mbgl/renderer/renderer.hpp>
#import <mbgl/storage/network_status.hpp>
#import <mbgl/storage/resource_options.hpp>
#import <mbgl/math/wrap.hpp>
#import <mbgl/util/action_journal.hpp>
#import <mbgl/util/client_options.hpp>
#import <mbgl/util/constants.hpp>
#import <mbgl/util/chrono.hpp>
#import <mbgl/util/exception.hpp>
#import <mbgl/util/run_loop.hpp>
#import <mbgl/util/string.hpp>
#import <mbgl/util/projection.hpp>

#import <map>
#import <unordered_map>
#import <unordered_set>

#import "MLNMapView+Impl.h"
#import "NSBundle+MLNAdditions.h"
#import "NSDate+MLNAdditions.h"
#import "NSProcessInfo+MLNAdditions.h"
#import "NSException+MLNAdditions.h"
#import "NSString+MLNAdditions.h"
#import "NSURL+MLNAdditions.h"
#import "NSColor+MLNAdditions.h"
#import "NSImage+MLNAdditions.h"
#import "NSPredicate+MLNPrivateAdditions.h"
#import "MLNNetworkConfiguration_Private.h"
#import "MLNLoggingConfiguration_Private.h"
#import "MLNReachability.h"
#import "MLNActionJournalOptions_Private.h"
#import "MLNRenderingStats_Private.h"
#import "MLNSettings_Private.h"

#import <CoreImage/CIFilter.h>

class MLNAnnotationContext;

/// Distance from the edge of the view to ornament views (logo, attribution, etc.).
const CGFloat MLNOrnamentPadding = 12;

/// Alpha value of the ornament views (logo, attribution, etc.).
const CGFloat MLNOrnamentOpacity = 0.9;

/// Default duration for programmatic animations.
const NSTimeInterval MLNAnimationDuration = 0.3;

/// Distance in points that a single press of the panning keyboard shortcut pans the map by.
const CGFloat MLNKeyPanningIncrement = 150;

/// Degrees that a single press of the rotation keyboard shortcut rotates the map by.
const CLLocationDegrees MLNKeyRotationIncrement = 25;

/// Key for the user default that, when true, causes the map view to zoom in and out on scroll wheel events.
NSString * const MLNScrollWheelZoomsMapViewDefaultKey = @"MLNScrollWheelZoomsMapView";

/// Reuse identifier and file name of the default point annotation image.
static NSString * const MLNDefaultStyleMarkerSymbolName = @"default_marker";

/// Prefix that denotes a sprite installed by MLNMapView, to avoid collisions
/// with style-defined sprites.
static NSString * const MLNAnnotationSpritePrefix = @"org.maplibre.sprites.";

/// Slop area around the hit testing point, allowing for imprecise annotation selection.
const CGFloat MLNAnnotationImagePaddingForHitTest = 4;

/// Distance from the callout’s anchor point to the annotation it points to.
const CGFloat MLNAnnotationImagePaddingForCallout = 4;

/// Padding to edge of view that an offscreen annotation must have when being brought onscreen (by being selected)
const NSEdgeInsets MLNMapViewOffscreenAnnotationPadding = NSEdgeInsetsMake(-30.0f, -30.0f, -30.0f, -30.0f);

/// Unique identifier representing a single annotation in mbgl.
typedef uint64_t MLNAnnotationTag;

/// An indication that the requested annotation was not found or is nonexistent.
enum { MLNAnnotationTagNotFound = UINT64_MAX };

/// Mapping from an annotation tag to metadata about that annotation, including
/// the annotation itself.
typedef std::unordered_map<MLNAnnotationTag, MLNAnnotationContext> MLNAnnotationTagContextMap;

/// Mapping from an annotation object to an annotation tag.
typedef std::map<id<MLNAnnotation>, MLNAnnotationTag> MLNAnnotationObjectTagMap;

/// Returns an NSImage for the default marker image.
NSImage *MLNDefaultMarkerImage() {
    NSString *path = [[NSBundle mgl_frameworkBundle] pathForResource:MLNDefaultStyleMarkerSymbolName
                                                              ofType:@"pdf"];
    return [[NSImage alloc] initWithContentsOfFile:path];
}

/// Converts a media timing function into a unit bezier object usable in mbgl.
mbgl::util::UnitBezier MLNUnitBezierForMediaTimingFunction(CAMediaTimingFunction *function) {
    if (!function) {
        function = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionDefault];
    }
    float p1[2], p2[2];
    [function getControlPointAtIndex:0 values:p1];
    [function getControlPointAtIndex:1 values:p2];
    return { p1[0], p1[1], p2[0], p2[1] };
}

/// Lightweight container for metadata about an annotation, including the annotation itself.
class MLNAnnotationContext {
public:
    id <MLNAnnotation> annotation;
    /// The annotation’s image’s reuse identifier.
    NSString *imageReuseIdentifier;
};

@interface MLNMapView () <NSPopoverDelegate, MLNMultiPointDelegate, NSGestureRecognizerDelegate>

@property (nonatomic, readwrite) NSSegmentedControl *zoomControls;
@property (nonatomic, readwrite) NSSlider *compass;
@property (nonatomic, readwrite) NSImageView *logoView;
@property (nonatomic, readwrite) NSView *attributionView;

@property (nonatomic, readwrite) MLNStyle *style;

/// Mapping from reusable identifiers to annotation images.
@property (nonatomic) NSMutableDictionary<NSString *, MLNAnnotationImage *> *annotationImagesByIdentifier;
/// Currently shown popover representing the selected annotation.
@property (nonatomic) NSPopover *calloutForSelectedAnnotation;

@property (nonatomic, readwrite, getter=isDormant) BOOL dormant;

@end

@implementation MLNMapView {
    /// Cross-platform map view controller.
    std::unique_ptr<mbgl::Map> _mbglMap;
    std::unique_ptr<MLNMapViewImpl> _mbglView;
    std::unique_ptr<MLNRenderFrontend> _rendererFrontend;

    NSPanGestureRecognizer *_panGestureRecognizer;
    NSMagnificationGestureRecognizer *_magnificationGestureRecognizer;
    NSRotationGestureRecognizer *_rotationGestureRecognizer;
    NSClickGestureRecognizer *_singleClickRecognizer;
    double _zoomAtBeginningOfGesture;
    CLLocationDirection _directionAtBeginningOfGesture;
    CGFloat _pitchAtBeginningOfGesture;
    BOOL _didHideCursorDuringGesture;

    MLNAnnotationTagContextMap _annotationContextsByAnnotationTag;
    MLNAnnotationObjectTagMap _annotationTagsByAnnotation;
    MLNAnnotationTag _selectedAnnotationTag;
    MLNAnnotationTag _lastSelectedAnnotationTag;
    /// Size of the rectangle formed by unioning the maximum slop area around every annotation image.
    NSSize _unionedAnnotationImageSize;
    std::vector<MLNAnnotationTag> _annotationsNearbyLastClick;
    /// True if any annotations that have tooltips have been installed.
    BOOL _wantsToolTipRects;
    /// True if any annotation images that have custom cursors have been installed.
    BOOL _wantsCursorRects;
    /// True if a willChange notification has been issued for shape annotation layers and a didChange notification is pending.
    BOOL _isChangingAnnotationLayers;

    // Cached checks for delegate method implementations that may be called from
    // MLNMultiPointDelegate methods.

    BOOL _delegateHasAlphasForShapeAnnotations;
    BOOL _delegateHasStrokeColorsForShapeAnnotations;
    BOOL _delegateHasFillColorsForShapeAnnotations;
    BOOL _delegateHasLineWidthsForShapeAnnotations;

    /// True if the current process is the Interface Builder designable
    /// renderer. When drawing the designable, the map is paused, so any call to
    /// it may hang the process.
    BOOL _isTargetingInterfaceBuilder;
    CLLocationDegrees _pendingLatitude;
    CLLocationDegrees _pendingLongitude;

    /// True if the view is currently printing itself.
    BOOL _isPrinting;

    /// reachability instance
    MLNReachability *_reachability;

    MLNRenderingStats* _renderingStats;
}

// MARK: Lifecycle

+ (void)initialize {
    if (self == [MLNMapView class]) {
        [[NSUserDefaults standardUserDefaults] registerDefaults:@{
            MLNScrollWheelZoomsMapViewDefaultKey: @NO,
        }];
    }
}

- (instancetype)initWithFrame:(NSRect)frameRect {
    if (self = [super initWithFrame:frameRect]) {
        MLNLogInfo(@"Starting %@ initialization.", NSStringFromClass([self class]));
        MLNLogDebug(@"Initializing frame: %@", NSStringFromRect(frameRect));
        [self commonInitWithOptions:nil];
        self.styleURL = nil;
        MLNLogInfo(@"Finalizing %@ initialization.", NSStringFromClass([self class]));
    }
    return self;
}

- (instancetype)initWithFrame:(NSRect)frame styleURL:(nullable NSURL *)styleURL {
    if (self = [super initWithFrame:frame]) {
        MLNLogInfo(@"Starting %@ initialization.", NSStringFromClass([self class]));
        MLNLogDebug(@"Initializing frame: %@ styleURL: %@", NSStringFromRect(frame), styleURL);
        [self commonInitWithOptions:nil];
        self.styleURL = styleURL;
        MLNLogInfo(@"Finalizing %@ initialization.", NSStringFromClass([self class]));
    }
    return self;
}

- (instancetype)initWithFrame:(CGRect)frame options:(MLNMapOptions *)options
{
    if (self = [super initWithFrame:frame])
    {
        MLNLogInfo(@"Starting %@ initialization.", NSStringFromClass([self class]));
        MLNLogDebug(@"Initializing frame: %@ with options", NSStringFromRect(frame));
        [self commonInitWithOptions:options];

        if (options) {
            if (options.styleURL) {
                self.styleURL = options.styleURL;
            }
        } else {
            self.styleURL = nil;
        }

        MLNLogInfo(@"Finalizing %@ initialization.", NSStringFromClass([self class]));
    }
    return self;
}

- (instancetype)initWithCoder:(nonnull NSCoder *)decoder {
    if (self = [super initWithCoder:decoder]) {
        MLNLogInfo(@"Starting %@ initialization.", NSStringFromClass([self class]));
        [self commonInitWithOptions:nil];
        MLNLogInfo(@"Finalizing %@ initialization.", NSStringFromClass([self class]));
    }
    return self;
}

- (void)awakeFromNib {
    [super awakeFromNib];

    // If the Style URL inspectable was not set, make sure to go through
    // -setStyleURL: to load the default style.
    if (_mbglMap->getStyle().getURL().empty()) {
        self.styleURL = nil;
    }
}

+ (NSArray *)restorableStateKeyPaths {
    return @[@"camera", @"debugMask"];
}

- (void)commonInitWithOptions:(MLNMapOptions*)mlnMapoptions
{
    if (mlnMapoptions == nil)
    {
        mlnMapoptions = [[MLNMapOptions alloc] init];
    }

    [MLNNetworkConfiguration sharedManager];

    _isTargetingInterfaceBuilder = NSProcessInfo.processInfo.mgl_isInterfaceBuilderDesignablesAgent;

    // Set up cross-platform controllers and resources.
    _mbglView = MLNMapViewImpl::Create(self);

    // Delete the pre-offline ambient cache at
    // ~/Library/Caches/com.mapbox.MapboxGL/cache.db.
    NSURL *cachesDirectoryURL = [[NSFileManager defaultManager] URLForDirectory:NSCachesDirectory
                                                                       inDomain:NSUserDomainMask
                                                              appropriateForURL:nil
                                                                         create:NO
                                                                          error:nil];
    cachesDirectoryURL = [cachesDirectoryURL URLByAppendingPathComponent:@"com.mapbox.MapboxGL"];
    NSURL *legacyCacheURL = [cachesDirectoryURL URLByAppendingPathComponent:@"cache.db"];
    [[NSFileManager defaultManager] removeItemAtURL:legacyCacheURL error:NULL];

    MLNRendererConfiguration *config = [MLNRendererConfiguration currentConfiguration];

    auto localFontFamilyName = config.localFontFamilyName ? std::string(config.localFontFamilyName.UTF8String) : nullptr;
    auto renderer = std::make_unique<mbgl::Renderer>(_mbglView->getRendererBackend(), config.scaleFactor, localFontFamilyName);
    BOOL enableCrossSourceCollisions = !config.perSourceCollisions;
    _rendererFrontend = std::make_unique<MLNRenderFrontend>(std::move(renderer), self, _mbglView->getRendererBackend(), true);

    mbgl::MapOptions mapOptions;
    mapOptions.withMapMode(mbgl::MapMode::Continuous)
              .withSize(self.size)
              .withPixelRatio(config.scaleFactor)
              .withConstrainMode(mbgl::ConstrainMode::None)
              .withViewportMode(mbgl::ViewportMode::Default)
              .withCrossSourceCollisions(enableCrossSourceCollisions);

    auto tileServerOptions = [[MLNSettings sharedSettings] tileServerOptionsInternal];
    mbgl::ResourceOptions resourceOptions;
    resourceOptions.withTileServerOptions(*tileServerOptions)
                   .withCachePath(MLNOfflineStorage.sharedOfflineStorage.databasePath.UTF8String)
                   .withAssetPath([NSBundle mainBundle].resourceURL.path.UTF8String);
    mbgl::ClientOptions clientOptions;

    auto apiKey = [[MLNSettings sharedSettings] apiKey];
    if (apiKey) {
        resourceOptions.withApiKey([apiKey UTF8String]);
    }

    const mbgl::util::ActionJournalOptions& actionJournalOptions = [mlnMapoptions.actionJournalOptions getCoreOptions];
    _mbglMap = std::make_unique<mbgl::Map>(*_rendererFrontend, *_mbglView, mapOptions, resourceOptions, clientOptions, actionJournalOptions);

    // Notify map object when network reachability status changes.
    _reachability = [MLNReachability reachabilityForInternetConnection];
    _reachability.reachableBlock = ^(MLNReachability *) {
        mbgl::NetworkStatus::Reachable();
    };
    [_reachability startNotifier];

    // Install ornaments and gesture recognizers.
    [self installZoomControls];
    [self installCompass];
    [self installLogoView];
    [self installAttributionView];
    [self installGestureRecognizers];

    // Set up annotation management and selection state.
    _annotationImagesByIdentifier = [NSMutableDictionary dictionary];
    _annotationContextsByAnnotationTag = {};
    _annotationTagsByAnnotation = {};
    _selectedAnnotationTag = MLNAnnotationTagNotFound;
    _lastSelectedAnnotationTag = MLNAnnotationTagNotFound;
    _annotationsNearbyLastClick = {};

    // Jump to Null Island initially.
    self.automaticallyAdjustsContentInsets = YES;
    mbgl::CameraOptions options;
    options.center = mbgl::LatLng(0, 0);
    options.padding = MLNEdgeInsetsFromNSEdgeInsets(self.contentInsets);
    options.zoom = *_mbglMap->getBounds().minZoom;
    _mbglMap->jumpTo(options);
    _pendingLatitude = NAN;
    _pendingLongitude = NAN;
}

- (mbgl::Size)size {
    // check for minimum texture size supported by OpenGL ES 2.0
    //
    CGSize size = CGSizeMake(MAX(self.bounds.size.width, 64), MAX(self.bounds.size.height, 64));
    return { static_cast<uint32_t>(size.width),
             static_cast<uint32_t>(size.height) };
}

- (mbgl::Size)framebufferSize {
    NSRect bounds = [self convertRectToBacking:self.bounds];
    return { static_cast<uint32_t>(bounds.size.width), static_cast<uint32_t>(bounds.size.height) };
}

/// Adds zoom controls to the lower-right corner.
- (void)installZoomControls {
    _zoomControls = [[NSSegmentedControl alloc] initWithFrame:NSZeroRect];
    _zoomControls.wantsLayer = YES;
    _zoomControls.layer.opacity = MLNOrnamentOpacity;
    [(NSSegmentedCell *)_zoomControls.cell setTrackingMode:NSSegmentSwitchTrackingMomentary];
    _zoomControls.continuous = YES;
    _zoomControls.segmentCount = 2;
    [_zoomControls setLabel:NSLocalizedStringWithDefaultValue(@"ZOOM_OUT_LABEL", nil, nil, @"−", @"Label of Zoom Out button; U+2212 MINUS SIGN") forSegment:0];
    [(NSSegmentedCell *)_zoomControls.cell setTag:0 forSegment:0];
    [(NSSegmentedCell *)_zoomControls.cell setToolTip:NSLocalizedStringWithDefaultValue(@"ZOOM_OUT_TOOLTIP", nil, nil, @"Zoom Out", @"Tooltip of Zoom Out button") forSegment:0];
    [_zoomControls setLabel:NSLocalizedStringWithDefaultValue(@"ZOOM_IN_LABEL", nil, nil, @"+", @"Label of Zoom In button") forSegment:1];
    [(NSSegmentedCell *)_zoomControls.cell setTag:1 forSegment:1];
    [(NSSegmentedCell *)_zoomControls.cell setToolTip:NSLocalizedStringWithDefaultValue(@"ZOOM_IN_TOOLTIP", nil, nil, @"Zoom In", @"Tooltip of Zoom In button") forSegment:1];
    _zoomControls.target = self;
    _zoomControls.action = @selector(zoomInOrOut:);
    _zoomControls.controlSize = NSControlSizeRegular;
    [_zoomControls sizeToFit];
    _zoomControls.translatesAutoresizingMaskIntoConstraints = NO;
    [self addSubview:_zoomControls];
}

/// Adds a rudimentary compass control to the lower-right corner.
- (void)installCompass {
    _compass = [[NSSlider alloc] initWithFrame:NSZeroRect];
    _compass.wantsLayer = YES;
    _compass.layer.opacity = MLNOrnamentOpacity;
    _compass.cell = [[MLNCompassCell alloc] init];
    _compass.continuous = YES;
    _compass.target = self;
    _compass.action = @selector(rotate:);
    [_compass sizeToFit];
    _compass.translatesAutoresizingMaskIntoConstraints = NO;
    [self addSubview:_compass];
}

/// Adds a Mapbox logo to the lower-left corner.
- (void)installLogoView {
    _logoView = [[NSImageView alloc] initWithFrame:NSZeroRect];
    _logoView.wantsLayer = YES;
    NSImage *logoImage = [[NSImage alloc] initWithContentsOfFile:
                          [[NSBundle mgl_frameworkBundle] pathForResource:@"mapbox" ofType:@"pdf"]];
    // Account for the image’s built-in padding when aligning other controls to the logo.
    logoImage.alignmentRect = NSOffsetRect(logoImage.alignmentRect, 0, 3);
    _logoView.image = logoImage;
    _logoView.translatesAutoresizingMaskIntoConstraints = NO;
    _logoView.accessibilityTitle = NSLocalizedStringWithDefaultValue(@"MAP_A11Y_TITLE", nil, nil, @"Mapbox", @"Accessibility title");
    [self addSubview:_logoView];
}

/// Adds legally required map attribution to the lower-left corner.
- (void)installAttributionView {
    [_attributionView removeFromSuperview];
    _attributionView = [[NSView alloc] initWithFrame:NSZeroRect];
    _attributionView.wantsLayer = YES;

    // Make the background and foreground translucent to be unobtrusive.
    _attributionView.layer.opacity = 0.6;

    // Blur the background to prevent text underneath the view from running into
    // the text in the view, rendering it illegible.
    CIFilter *attributionBlurFilter = [CIFilter filterWithName:@"CIGaussianBlur"];
    [attributionBlurFilter setDefaults];

    // Brighten the background. This is similar to applying a translucent white
    // background on the view, but the effect is a bit more subtle and works
    // well with the blur above.
    CIFilter *attributionColorFilter = [CIFilter filterWithName:@"CIColorControls"];
    [attributionColorFilter setDefaults];
    [attributionColorFilter setValue:@(0.1) forKey:kCIInputBrightnessKey];

    // Apply the background effects and a standard button corner radius.
    _attributionView.backgroundFilters = @[attributionColorFilter, attributionBlurFilter];
    _attributionView.layer.cornerRadius = 4;

    _attributionView.translatesAutoresizingMaskIntoConstraints = NO;
    [self addSubview:_attributionView];
    [self updateAttributionView];
}

/// Adds gesture recognizers for manipulating the viewport and selecting annotations.
- (void)installGestureRecognizers {
    _scrollEnabled = YES;
    _zoomEnabled = YES;
    _rotateEnabled = YES;
    _pitchEnabled = YES;

    _panGestureRecognizer = [[NSPanGestureRecognizer alloc] initWithTarget:self action:@selector(handlePanGesture:)];
    _panGestureRecognizer.delaysKeyEvents = YES;
    [self addGestureRecognizer:_panGestureRecognizer];

    _singleClickRecognizer = [[NSClickGestureRecognizer alloc] initWithTarget:self action:@selector(handleClickGesture:)];
    _singleClickRecognizer.delaysPrimaryMouseButtonEvents = NO;
    _singleClickRecognizer.delegate = self;
    [self addGestureRecognizer:_singleClickRecognizer];

    NSClickGestureRecognizer *rightClickGestureRecognizer = [[NSClickGestureRecognizer alloc] initWithTarget:self action:@selector(handleRightClickGesture:)];
    rightClickGestureRecognizer.buttonMask = 0x2;
    [self addGestureRecognizer:rightClickGestureRecognizer];

    NSClickGestureRecognizer *doubleClickGestureRecognizer = [[NSClickGestureRecognizer alloc] initWithTarget:self action:@selector(handleDoubleClickGesture:)];
    doubleClickGestureRecognizer.numberOfClicksRequired = 2;
    doubleClickGestureRecognizer.delaysPrimaryMouseButtonEvents = NO;
    [self addGestureRecognizer:doubleClickGestureRecognizer];

    _magnificationGestureRecognizer = [[NSMagnificationGestureRecognizer alloc] initWithTarget:self action:@selector(handleMagnificationGesture:)];
    [self addGestureRecognizer:_magnificationGestureRecognizer];

    _rotationGestureRecognizer = [[NSRotationGestureRecognizer alloc] initWithTarget:self action:@selector(handleRotationGesture:)];
    [self addGestureRecognizer:_rotationGestureRecognizer];
}

/// Updates the attribution view to reflect the sources used. For now, this is
/// hard-coded to the standard Mapbox and OpenStreetMap attribution.
- (void)updateAttributionView {
    NSView *attributionView = self.attributionView;
    for (NSView *button in attributionView.subviews) {
        [button removeConstraints:button.constraints];
    }
    attributionView.subviews = @[];
    [attributionView removeConstraints:attributionView.constraints];

    // Make the whole string mini by default.
    // Force links to be black, because the default blue is distracting.
    CGFloat miniSize = [NSFont systemFontSizeForControlSize:NSControlSizeMini];
    NSArray *attributionInfos = [self.style attributionInfosWithFontSize:miniSize linkColor:[NSColor blackColor]];
    for (MLNAttributionInfo *info in attributionInfos) {
        // Feedback links are added to the Help menu.
        if (info.feedbackLink) {
            continue;
        }

        // For each attribution, add a borderless button that responds to clicks
        // and feels like a hyperlink.
        NSButton *button = [[MLNAttributionButton alloc] initWithAttributionInfo:info];
        button.controlSize = NSControlSizeMini;
        button.translatesAutoresizingMaskIntoConstraints = NO;

        // Set the new button flush with the buttom of the container and to the
        // right of the previous button, with standard spacing. If there is no
        // previous button, align to the container instead.
        NSView *previousView = attributionView.subviews.lastObject;
        [attributionView addSubview:button];
        [attributionView addConstraint:
         [NSLayoutConstraint constraintWithItem:button
                                      attribute:NSLayoutAttributeBottom
                                      relatedBy:NSLayoutRelationEqual
                                         toItem:attributionView
                                      attribute:NSLayoutAttributeBottom
                                     multiplier:1
                                       constant:0]];
        [attributionView addConstraint:
         [NSLayoutConstraint constraintWithItem:button
                                      attribute:NSLayoutAttributeLeading
                                      relatedBy:NSLayoutRelationEqual
                                         toItem:previousView ? previousView : attributionView
                                      attribute:previousView ? NSLayoutAttributeTrailing : NSLayoutAttributeLeading
                                     multiplier:1
                                       constant:8]];
        [attributionView addConstraint:
         [NSLayoutConstraint constraintWithItem:button
                                      attribute:NSLayoutAttributeTop
                                      relatedBy:NSLayoutRelationEqual
                                         toItem:attributionView
                                      attribute:NSLayoutAttributeTop
                                     multiplier:1
                                       constant:0]];
    }

    if (attributionInfos.count) {
        [attributionView addConstraint:
         [NSLayoutConstraint constraintWithItem:attributionView
                                      attribute:NSLayoutAttributeTrailing
                                      relatedBy:NSLayoutRelationEqual
                                         toItem:attributionView.subviews.lastObject
                                      attribute:NSLayoutAttributeTrailing
                                     multiplier:1
                                       constant:8]];
    }
}

- (void)dealloc {

    [_reachability stopNotifier];


    [self.window removeObserver:self forKeyPath:@"contentLayoutRect"];
    [self.window removeObserver:self forKeyPath:@"titlebarAppearsTransparent"];

    // Close any annotation callout immediately.
    [self.calloutForSelectedAnnotation close];
    self.calloutForSelectedAnnotation = nil;

    // Removing the annotations unregisters any outstanding KVO observers.
    [self removeAnnotations:self.annotations];

    _mbglMap.reset();
    _mbglView.reset();
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(__unused NSDictionary *)change context:(void *)context {
    if ([keyPath isEqualToString:@"contentLayoutRect"] ||
        [keyPath isEqualToString:@"titlebarAppearsTransparent"]) {
        [self adjustContentInsets];
    } else if ([keyPath isEqualToString:@"coordinate"] &&
               [object conformsToProtocol:@protocol(MLNAnnotation)] &&
               ![object isKindOfClass:[MLNMultiPoint class]]) {
        id <MLNAnnotation> annotation = object;
        MLNAnnotationTag annotationTag = (MLNAnnotationTag)(NSUInteger)context;
        // We can get here because a subclass registered itself as an observer
        // of the coordinate key path of a non-multipoint annotation but failed
        // to handle the change. This check deters us from treating the
        // subclass’s context as an annotation tag. If the context happens to
        // match a valid annotation tag, the annotation will be unnecessarily
        // but safely updated.
        if (annotation == [self annotationWithTag:annotationTag]) {
            const mbgl::Point<double> point = MLNPointFromLocationCoordinate2D(annotation.coordinate);
            MLNAnnotationImage *annotationImage = [self imageOfAnnotationWithTag:annotationTag];
            _mbglMap->updateAnnotation(annotationTag, mbgl::SymbolAnnotation { point, annotationImage.styleIconIdentifier.UTF8String ?: "" });
            [self updateAnnotationCallouts];
        }
    } else if ([keyPath isEqualToString:@"coordinates"] &&
               [object isKindOfClass:[MLNMultiPoint class]]) {
        MLNMultiPoint *annotation = object;
        MLNAnnotationTag annotationTag = (MLNAnnotationTag)(NSUInteger)context;
        // We can get here because a subclass registered itself as an observer
        // of the coordinates key path of a multipoint annotation but failed
        // to handle the change. This check deters us from treating the
        // subclass’s context as an annotation tag. If the context happens to
        // match a valid annotation tag, the annotation will be unnecessarily
        // but safely updated.
        if (annotation == [self annotationWithTag:annotationTag]) {
            _mbglMap->updateAnnotation(annotationTag, [annotation annotationObjectWithDelegate:self]);
            [self updateAnnotationCallouts];
        }
    }
}

+ (BOOL)automaticallyNotifiesObserversForKey:(NSString *)key {
    return [key isEqualToString:@"annotations"] ? YES : [super automaticallyNotifiesObserversForKey:key];
}

- (void)setDelegate:(id<MLNMapViewDelegate>)delegate {
    _delegate = delegate;

    // Cache checks for delegate method implementations that may be called in a
    // hot loop, namely the annotation style methods.
    _delegateHasAlphasForShapeAnnotations = [_delegate respondsToSelector:@selector(mapView:alphaForShapeAnnotation:)];
    _delegateHasStrokeColorsForShapeAnnotations = [_delegate respondsToSelector:@selector(mapView:strokeColorForShapeAnnotation:)];
    _delegateHasFillColorsForShapeAnnotations = [_delegate respondsToSelector:@selector(mapView:fillColorForPolygonAnnotation:)];
    _delegateHasLineWidthsForShapeAnnotations = [_delegate respondsToSelector:@selector(mapView:lineWidthForPolylineAnnotation:)];

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundeclared-selector"
    if ([self.delegate respondsToSelector:@selector(mapView:regionWillChangeAnimated:)]) {
        NSLog(@"-mapView:regionWillChangeAnimated: is not supported by the macOS SDK, but %@ implements it anyways. "
              @"Please implement -[%@ mapView:cameraWillChangeAnimated:] instead.",
              NSStringFromClass([delegate class]), NSStringFromClass([delegate class]));
    }
    if ([self.delegate respondsToSelector:@selector(mapViewRegionIsChanging:)]) {
        NSLog(@"-mapViewRegionIsChanging: is not supported by the macOS SDK, but %@ implements it anyways. "
              @"Please implement -[%@ mapViewCameraIsChanging:] instead.",
              NSStringFromClass([delegate class]), NSStringFromClass([delegate class]));
    }
    if ([self.delegate respondsToSelector:@selector(mapView:regionDidChangeAnimated:)]) {
        NSLog(@"-mapView:regionDidChangeAnimated: is not supported by the macOS SDK, but %@ implements it anyways. "
              @"Please implement -[%@ mapView:cameraDidChangeAnimated:] instead.",
              NSStringFromClass([delegate class]), NSStringFromClass([delegate class]));
    }
#pragma clang diagnostic pop
}

// MARK: Style

+ (NSSet<NSString *> *)keyPathsForValuesAffectingStyle {
    return [NSSet setWithObject:@"styleURL"];
}

- (nonnull NSURL *)styleURL {
    NSString *styleURLString = @(_mbglMap->getStyle().getURL().c_str()).mgl_stringOrNilIfEmpty;
    return styleURLString ? [NSURL URLWithString:styleURLString] : [MLNStyle defaultStyleURL];
}

- (void)setStyleURL:(nullable NSURL *)styleURL {
    if (_isTargetingInterfaceBuilder) {
        return;
    }

    if (!styleURL) {
        styleURL = [MLNStyle defaultStyleURL];
    }
    MLNLogDebug(@"Setting styleURL: %@", styleURL);

    // An access token is required to load any default style, including Streets.
    if ([[MLNSettings sharedSettings] tileServerOptionsInternal]->requiresApiKey() && ![MLNSettings apiKey]) {
        NSLog(@"Cannot set the style URL to %@ because no API key has been specified.", styleURL);
        return;
    }

    styleURL = styleURL.mgl_URLByStandardizingScheme;
    self.style = nil;
    _mbglMap->getStyle().loadURL(styleURL.absoluteString.UTF8String);
}

- (IBAction)reloadStyle:(__unused id)sender {
    MLNLogInfo(@"Reloading style.");
    NSURL *styleURL = self.styleURL;
    _mbglMap->getStyle().loadURL("");
    self.styleURL = styleURL;
}

- (void)setPrefetchesTiles:(BOOL)prefetchesTiles
{
    _mbglMap->setPrefetchZoomDelta(prefetchesTiles ? mbgl::util::DEFAULT_PREFETCH_ZOOM_DELTA : 0);
}

- (BOOL)prefetchesTiles
{
    return _mbglMap->getPrefetchZoomDelta() > 0 ? YES : NO;
}

- (void)setTileLodMinRadius:(double)tileLodMinRadius
{
    _mbglMap->setTileLodMinRadius(tileLodMinRadius);
}

- (double)tileLodMinRadius
{
    return _mbglMap->getTileLodMinRadius();
}

- (void)setTileLodScale:(double)tileLodScale
{
    _mbglMap->setTileLodScale(tileLodScale);
}

- (double)tileLodScale
{
    return _mbglMap->getTileLodScale();
}

-(void)setTileLodPitchThreshold:(double)tileLodPitchThreshold
{
    _mbglMap->setTileLodPitchThreshold(tileLodPitchThreshold);
}

-(double)tileLodPitchThreshold
{
    return _mbglMap->getTileLodPitchThreshold();
}

-(void)setTileLodZoomShift:(double)tileLodZoomShift
{
    _mbglMap->setTileLodZoomShift(tileLodZoomShift);
}

-(double)tileLodZoomShift
{
    return _mbglMap->getTileLodZoomShift();
}

- (mbgl::Renderer *)renderer {
    return _rendererFrontend->getRenderer();
}

// MARK: View hierarchy and drawing

- (void)viewWillMoveToWindow:(NSWindow *)newWindow {
    [self deselectAnnotation:self.selectedAnnotation];
    if (!self.dormant && !newWindow) {
        self.dormant = YES;
    }

    [self.window removeObserver:self forKeyPath:@"contentLayoutRect"];
    [self.window removeObserver:self forKeyPath:@"titlebarAppearsTransparent"];
}

- (void)viewDidMoveToWindow {
    NSWindow *window = self.window;
    if (self.dormant && window) {
        self.dormant = NO;
    }

    if (window && _mbglMap->getMapOptions().constrainMode() == mbgl::ConstrainMode::None) {
        _mbglMap->setConstrainMode(mbgl::ConstrainMode::HeightOnly);
    }

    [window addObserver:self
             forKeyPath:@"contentLayoutRect"
                options:NSKeyValueObservingOptionInitial
                context:NULL];
    [window addObserver:self
             forKeyPath:@"titlebarAppearsTransparent"
                options:NSKeyValueObservingOptionInitial
                context:NULL];
}

- (BOOL)wantsLayer {
    return YES;
}

- (BOOL)wantsBestResolutionOpenGLSurface {
    // Use an OpenGL layer, except when drawing the designable, which is just
    // ordinary Cocoa.
    return !_isTargetingInterfaceBuilder;
}

- (CGLContextObj)context {
    return _mbglView->getCGLContextObj();
}

- (void)setFrame:(NSRect)frame {
    super.frame = frame;
    if (!_isTargetingInterfaceBuilder) {
        _mbglMap->setSize(self.size);
    }
}

- (void)updateConstraints {
    // Place the zoom controls at the lower-right corner of the view.
    [self addConstraint:
     [NSLayoutConstraint constraintWithItem:self
                                  attribute:NSLayoutAttributeBottom
                                  relatedBy:NSLayoutRelationEqual
                                     toItem:_zoomControls
                                  attribute:NSLayoutAttributeBottom
                                 multiplier:1
                                   constant:MLNOrnamentPadding]];
    [self addConstraint:
     [NSLayoutConstraint constraintWithItem:self
                                  attribute:NSLayoutAttributeTrailing
                                  relatedBy:NSLayoutRelationEqual
                                     toItem:_zoomControls
                                  attribute:NSLayoutAttributeTrailing
                                 multiplier:1
                                   constant:MLNOrnamentPadding]];

    // Center the compass above the zoom controls, assuming that the compass is
    // narrower than the zoom controls.
    [self addConstraint:
     [NSLayoutConstraint constraintWithItem:_compass
                                  attribute:NSLayoutAttributeCenterX
                                  relatedBy:NSLayoutRelationEqual
                                     toItem:_zoomControls
                                  attribute:NSLayoutAttributeCenterX
                                 multiplier:1
                                   constant:0]];
    [self addConstraint:
     [NSLayoutConstraint constraintWithItem:_zoomControls
                                  attribute:NSLayoutAttributeTop
                                  relatedBy:NSLayoutRelationEqual
                                     toItem:_compass
                                  attribute:NSLayoutAttributeBottom
                                 multiplier:1
                                   constant:8]];

    // Place the logo view in the lower-left corner of the view, accounting for
    // the logo’s alignment rect.
    [self addConstraint:
     [NSLayoutConstraint constraintWithItem:self
                                  attribute:NSLayoutAttributeBottom
                                  relatedBy:NSLayoutRelationEqual
                                     toItem:_logoView
                                  attribute:NSLayoutAttributeBottom
                                 multiplier:1
                                   constant:MLNOrnamentPadding - _logoView.image.alignmentRect.origin.y]];
    [self addConstraint:
     [NSLayoutConstraint constraintWithItem:_logoView
                                  attribute:NSLayoutAttributeLeading
                                  relatedBy:NSLayoutRelationEqual
                                     toItem:self
                                  attribute:NSLayoutAttributeLeading
                                 multiplier:1
                                   constant:MLNOrnamentPadding - _logoView.image.alignmentRect.origin.x]];

    // Place the attribution view to the right of the logo view and size it to
    // fit the buttons inside.
    [self addConstraint:[NSLayoutConstraint constraintWithItem:_logoView
                                                     attribute:NSLayoutAttributeBaseline
                                                     relatedBy:NSLayoutRelationEqual
                                                        toItem:_attributionView
                                                     attribute:NSLayoutAttributeBaseline
                                                    multiplier:1
                                                      constant:_logoView.image.alignmentRect.origin.y]];
    [self addConstraint:[NSLayoutConstraint constraintWithItem:_attributionView
                                                     attribute:NSLayoutAttributeLeading
                                                     relatedBy:NSLayoutRelationEqual
                                                        toItem:_logoView
                                                     attribute:NSLayoutAttributeTrailing
                                                    multiplier:1
                                                      constant:8]];

    [super updateConstraints];
}

- (void)renderSync {
    if (!self.dormant && _rendererFrontend) {
        _rendererFrontend->render();

        if (_isPrinting) {
            _isPrinting = NO;
            NSImage *image = [[NSImage alloc] initWithMLNPremultipliedImage:_mbglView->readStillImage()];
            [self performSelector:@selector(printWithImage:) withObject:image afterDelay:0];
        }

//        [self updateUserLocationAnnotationView];
    }
}

- (BOOL)isTargetingInterfaceBuilder {
    return _isTargetingInterfaceBuilder;
}

- (void)setNeedsRerender {
    MLNAssertIsMainThread();

    [self.layer setNeedsDisplay];
}

- (void)cameraWillChangeAnimated:(BOOL)animated {
    if (!_mbglMap) {
        return;
    }

    if ([self.delegate respondsToSelector:@selector(mapView:cameraWillChangeAnimated:)]) {
        [self.delegate mapView:self cameraWillChangeAnimated:animated];
    }
}

- (void)cameraIsChanging {
    if (!_mbglMap) {
        return;
    }

    // Update a minimum of UI that needs to stay attached to the map
    // while animating.
    [self updateCompass];
    [self updateAnnotationCallouts];

    if ([self.delegate respondsToSelector:@selector(mapViewCameraIsChanging:)]) {
        [self.delegate mapViewCameraIsChanging:self];
    }
}

- (void)cameraDidChangeAnimated:(BOOL)animated {
    if (!_mbglMap) {
        return;
    }

    // Update all UI at the end of an animation or atomic change to the
    // viewport. More expensive updates can happen here, but care should
    // still be taken to minimize the work done here because scroll
    // gesture recognition and momentum scrolling is performed as a
    // series of atomic changes, not an animation.
    [self updateZoomControls];
    [self updateCompass];
    [self updateAnnotationCallouts];
    [self updateAnnotationTrackingAreas];

    if ([self.delegate respondsToSelector:@selector(mapView:cameraDidChangeAnimated:)]) {
        [self.delegate mapView:self cameraDidChangeAnimated:animated];
    }
}

- (void)mapViewWillStartLoadingMap {
    if (!_mbglMap) {
        return;
    }

    if ([self.delegate respondsToSelector:@selector(mapViewWillStartLoadingMap:)]) {
        [self.delegate mapViewWillStartLoadingMap:self];
    }
}

- (void)mapViewDidFinishLoadingMap {
    if (!_mbglMap) {
        return;
    }

    [self.style willChangeValueForKey:@"sources"];
    [self.style didChangeValueForKey:@"sources"];
    [self.style willChangeValueForKey:@"layers"];
    [self.style didChangeValueForKey:@"layers"];
    if ([self.delegate respondsToSelector:@selector(mapViewDidFinishLoadingMap:)]) {
        [self.delegate mapViewDidFinishLoadingMap:self];
    }
}

- (void)mapViewDidFailLoadingMapWithError:(NSError *)error {
    if (!_mbglMap) {
        return;
    }

    if ([self.delegate respondsToSelector:@selector(mapViewDidFailLoadingMap:withError:)]) {
        [self.delegate mapViewDidFailLoadingMap:self withError:error];
    }
}

- (void)mapViewWillStartRenderingFrame {
    if (!_mbglMap) {
        return;
    }

    if ([self.delegate respondsToSelector:@selector(mapViewWillStartRenderingFrame:)]) {
        [self.delegate mapViewWillStartRenderingFrame:self];
    }
}

- (void)mapViewDidFinishRenderingFrameFullyRendered:(BOOL)fullyRendered
                                     renderingStats:(const mbgl::gfx::RenderingStats &)stats {
    if (!_mbglMap) {
        return;
    }

    if (_isChangingAnnotationLayers) {
        _isChangingAnnotationLayers = NO;
        [self.style didChangeValueForKey:@"layers"];
    }

    if ([self.delegate respondsToSelector:@selector(mapViewDidFinishRenderingFrame:fullyRendered:renderingStats:)])
    {
        if (!_renderingStats) {
            _renderingStats = [[MLNRenderingStats alloc] init];
        }

        [_renderingStats setCoreData:stats];
        [self.delegate mapViewDidFinishRenderingFrame:self fullyRendered:fullyRendered renderingStats:_renderingStats];
    }
    else if ([self.delegate respondsToSelector:@selector(mapViewDidFinishRenderingFrame:fullyRendered:frameEncodingTime:frameRenderingTime:)])
    {
        [self.delegate mapViewDidFinishRenderingFrame:self
                                        fullyRendered:fullyRendered
                                    frameEncodingTime:stats.encodingTime
                                   frameRenderingTime:stats.renderingTime];
    }
    else if ([self.delegate respondsToSelector:@selector(mapViewDidFinishRenderingFrame:fullyRendered:)])
    {
        [self.delegate mapViewDidFinishRenderingFrame:self fullyRendered:fullyRendered];
    }
}

- (void)mapViewWillStartRenderingMap {
    if (!_mbglMap) {
        return;
    }

    if ([self.delegate respondsToSelector:@selector(mapViewWillStartRenderingMap:)]) {
        [self.delegate mapViewWillStartRenderingMap:self];
    }
}

- (void)mapViewDidFinishRenderingMapFullyRendered:(BOOL)fullyRendered {
    if (!_mbglMap) {
        return;
    }

    if ([self.delegate respondsToSelector:@selector(mapViewDidFinishRenderingMap:fullyRendered:)]) {
        [self.delegate mapViewDidFinishRenderingMap:self fullyRendered:fullyRendered];
    }
}

- (void)mapViewDidBecomeIdle {
    if (!_mbglMap) {
        return;
    }

    if ([self.delegate respondsToSelector:@selector(mapViewDidBecomeIdle:)]) {
        [self.delegate mapViewDidBecomeIdle:self];
    }
}

- (void)mapViewDidFinishLoadingStyle {
    if (!_mbglMap) {
        return;
    }

    self.style = [[MLNStyle alloc] initWithRawStyle:&_mbglMap->getStyle() stylable:self];
    if ([self.delegate respondsToSelector:@selector(mapView:didFinishLoadingStyle:)])
    {
        [self.delegate mapView:self didFinishLoadingStyle:self.style];
    }
}

- (void)sourceDidChange:(MLNSource *)source {
    if (!_mbglMap) {
        return;
    }

    if ([self.delegate respondsToSelector:@selector(mapView:sourceDidChange:)]) {
        [self.delegate mapView:self sourceDidChange:source];
    }

    // Attribution only applies to tiled sources
    if ([source isKindOfClass:[MLNTileSource class]]) {
        [self installAttributionView];
    }
    self.needsUpdateConstraints = YES;
    self.needsDisplay = YES;
}

- (BOOL)shouldRemoveStyleImage:(NSString *)imageName {
    if ([self.delegate respondsToSelector:@selector(mapView:shouldRemoveStyleImage:)]) {
        return [self.delegate mapView:self shouldRemoveStyleImage:imageName];
    }

    return YES;
}

- (void)shaderWillCompile:(NSInteger)id backend:(NSInteger)backend defines:(nonnull NSString *)defines {
    if (!_mbglMap) {
        return;
    }

    if ([self.delegate respondsToSelector:@selector(mapView:shaderWillCompile:backend:defines:)]) {
        [self.delegate mapView:self shaderWillCompile:id backend:backend defines:defines];
    }
}

- (void)shaderDidCompile:(NSInteger)id backend:(NSInteger)backend defines:(nonnull NSString *)defines {
    if (!_mbglMap) {
        return;
    }

    if ([self.delegate respondsToSelector:@selector(mapView:shaderDidCompile:backend:defines:)]) {
        [self.delegate mapView:self shaderDidCompile:id backend:backend defines:defines];
    }
}

- (void)shaderDidFailCompile:(NSInteger)id backend:(NSInteger)backend defines:(nonnull NSString *)defines {
    if (!_mbglMap) {
        return;
    }

    if ([self.delegate respondsToSelector:@selector(mapView:shaderDidFailCompile:backend:defines:)]) {
        [self.delegate mapView:self shaderDidFailCompile:id backend:backend defines:defines];
    }
}

- (void)glyphsWillLoad:(nonnull NSArray<NSString*>*)fontStack range:(NSRange)range {
    if (!_mbglMap) {
        return;
    }

    if ([self.delegate respondsToSelector:@selector(mapView:glyphsWillLoad:range:)]) {
        [self.delegate mapView:self glyphsWillLoad:fontStack range:range];
    }
}

- (void)glyphsDidLoad:(nonnull NSArray<NSString*>*)fontStack range:(NSRange)range {
    if (!_mbglMap) {
        return;
    }

    if ([self.delegate respondsToSelector:@selector(mapView:glyphsDidLoad:range:)]) {
        [self.delegate mapView:self glyphsDidLoad:fontStack range:range];
    }
}

- (void)glyphsDidError:(nonnull NSArray<NSString*>*)fontStack range:(NSRange)range {
    if (!_mbglMap) {
        return;
    }

    if ([self.delegate respondsToSelector:@selector(mapView:glyphsDidError:range:)]) {
        [self.delegate mapView:self glyphsDidError:fontStack range:range];
    }
}

- (void)tileDidTriggerAction:(MLNTileOperation)operation
                           x:(NSInteger)x
                           y:(NSInteger)y
                           z:(NSInteger)z
                        wrap:(NSInteger)wrap
                 overscaledZ:(NSInteger)overscaledZ
                    sourceID:(nonnull NSString *)sourceID {
    if (!_mbglMap) {
        return;
    }

    if ([self.delegate respondsToSelector:@selector(mapView:tileDidTriggerAction:x:y:z:wrap:overscaledZ:sourceID:)]) {
        [self.delegate mapView:self tileDidTriggerAction:operation x:x y:y z:z wrap:wrap overscaledZ:overscaledZ sourceID:sourceID];
    }
}

- (void)spriteWillLoad:(nullable NSString *)id url:(nullable NSString *)url {
    if (!_mbglMap) {
        return;
    }

    if ([self.delegate respondsToSelector:@selector(mapView:spriteWillLoad:url:)]) {
        [self.delegate mapView:self spriteWillLoad:id url:url];
    }
}

- (void)spriteDidLoad:(nullable NSString *)id url:(nullable NSString *)url {
    if (!_mbglMap) {
        return;
    }

    if ([self.delegate respondsToSelector:@selector(mapView:spriteDidLoad:url:)]) {
        [self.delegate mapView:self spriteDidLoad:id url:url];
    }
}

- (void)spriteDidError:(nullable NSString *)id url:(nullable NSString *)url {
    if (!_mbglMap) {
        return;
    }

    if ([self.delegate respondsToSelector:@selector(mapView:spriteDidError:url:)]) {
        [self.delegate mapView:self spriteDidError:id url:url];
    }
}


// MARK: Printing

- (void)print:(__unused id)sender {
    _isPrinting = YES;
    [self setNeedsRerender];
}

- (void)printWithImage:(NSImage *)image {
    NSImageView *imageView = [[NSImageView alloc] initWithFrame:self.bounds];
    imageView.image = image;

    NSPrintOperation *op = [NSPrintOperation printOperationWithView:imageView];
    [op runOperation];
}

// MARK: Viewport

+ (NSSet<NSString *> *)keyPathsForValuesAffectingCenterCoordinate {
    return [NSSet setWithObjects:@"latitude", @"longitude", @"camera", nil];
}

- (CLLocationCoordinate2D)centerCoordinate {
    mbgl::EdgeInsets padding = MLNEdgeInsetsFromNSEdgeInsets(self.contentInsets);
    return MLNLocationCoordinate2DFromLatLng(*_mbglMap->getCameraOptions(padding).center);
}

- (void)setCenterCoordinate:(CLLocationCoordinate2D)centerCoordinate {
    MLNLogDebug(@"Setting centerCoordinate: %@", MLNStringFromCLLocationCoordinate2D(centerCoordinate));
    [self setCenterCoordinate:centerCoordinate animated:NO];
}

- (void)setCenterCoordinate:(CLLocationCoordinate2D)centerCoordinate animated:(BOOL)animated {
    [self setCenterCoordinate:centerCoordinate animated:animated completionHandler:nil];
}

- (void)setCenterCoordinate:(CLLocationCoordinate2D)centerCoordinate animated:(BOOL)animated completionHandler:(nullable void (^)(void))completion {
    MLNLogDebug(@"Setting centerCoordinate: %@ animated: %@", MLNStringFromCLLocationCoordinate2D(centerCoordinate), MLNStringFromBOOL(animated));
    mbgl::AnimationOptions animationOptions = MLNDurationFromTimeInterval(animated ? MLNAnimationDuration : 0);
    animationOptions.transitionFinishFn = ^() {
        [self didChangeValueForKey:@"centerCoordinate"];
        if (completion) {
            dispatch_async(dispatch_get_main_queue(), ^{
                completion();
            });
        }
    };

    [self willChangeValueForKey:@"centerCoordinate"];
    _mbglMap->easeTo(mbgl::CameraOptions()
                         .withCenter(MLNLatLngFromLocationCoordinate2D(centerCoordinate))
                         .withPadding(MLNEdgeInsetsFromNSEdgeInsets(self.contentInsets)),
                     animationOptions);
}

- (void)offsetCenterCoordinateBy:(NSPoint)delta animated:(BOOL)animated {
    [self willChangeValueForKey:@"centerCoordinate"];
    _mbglMap->cancelTransitions();
    MLNMapCamera *oldCamera = self.camera;
    _mbglMap->moveBy({ delta.x, delta.y },
                     MLNDurationFromTimeInterval(animated ? MLNAnimationDuration : 0));
    if ([self.delegate respondsToSelector:@selector(mapView:shouldChangeFromCamera:toCamera:)]
        && ![self.delegate mapView:self shouldChangeFromCamera:oldCamera toCamera:self.camera]) {
        self.camera = oldCamera;
    }
    [self didChangeValueForKey:@"centerCoordinate"];
}

- (CLLocationDegrees)pendingLatitude {
    return _pendingLatitude;
}

- (void)setPendingLatitude:(CLLocationDegrees)pendingLatitude {
    _pendingLatitude = pendingLatitude;
}

- (CLLocationDegrees)pendingLongitude {
    return _pendingLongitude;
}

- (void)setPendingLongitude:(CLLocationDegrees)pendingLongitude {
    _pendingLongitude = pendingLongitude;
}

+ (NSSet<NSString *> *)keyPathsForValuesAffectingZoomLevel {
    return [NSSet setWithObject:@"camera"];
}

- (double)zoomLevel {
    mbgl::EdgeInsets padding = MLNEdgeInsetsFromNSEdgeInsets(self.contentInsets);
    return *_mbglMap->getCameraOptions(padding).zoom;
}

- (void)setZoomLevel:(double)zoomLevel {
    MLNLogDebug(@"Setting zoomLevel: %f", zoomLevel);
    [self setZoomLevel:zoomLevel animated:NO];
}

- (void)setZoomLevel:(double)zoomLevel animated:(BOOL)animated {
    MLNLogDebug(@"Setting zoomLevel: %f animated: %@", zoomLevel, MLNStringFromBOOL(animated));
    [self willChangeValueForKey:@"zoomLevel"];
    _mbglMap->easeTo(mbgl::CameraOptions()
                         .withZoom(zoomLevel)
                         .withPadding(MLNEdgeInsetsFromNSEdgeInsets(self.contentInsets)),
                     MLNDurationFromTimeInterval(animated ? MLNAnimationDuration : 0));
    [self didChangeValueForKey:@"zoomLevel"];
}

- (void)setZoomLevel:(double)zoomLevel atPoint:(NSPoint)point animated:(BOOL)animated {
    [self willChangeValueForKey:@"centerCoordinate"];
    [self willChangeValueForKey:@"zoomLevel"];
    MLNMapCamera *oldCamera = self.camera;
    mbgl::ScreenCoordinate center(point.x, self.bounds.size.height - point.y);
    _mbglMap->easeTo(mbgl::CameraOptions()
                         .withZoom(zoomLevel)
                         .withAnchor(center),
                     MLNDurationFromTimeInterval(animated ? MLNAnimationDuration : 0));
    if ([self.delegate respondsToSelector:@selector(mapView:shouldChangeFromCamera:toCamera:)]
        && ![self.delegate mapView:self shouldChangeFromCamera:oldCamera toCamera:self.camera]) {
        self.camera = oldCamera;
    }
    [self didChangeValueForKey:@"zoomLevel"];
    [self didChangeValueForKey:@"centerCoordinate"];
}

- (void)setMinimumZoomLevel:(double)minimumZoomLevel
{
    MLNLogDebug(@"Setting minimumZoomLevel: %f", minimumZoomLevel);
    _mbglMap->setBounds(mbgl::BoundOptions().withMinZoom(minimumZoomLevel));
}

- (void)setMaximumZoomLevel:(double)maximumZoomLevel
{
    MLNLogDebug(@"Setting maximumZoomLevel: %f", maximumZoomLevel);
    _mbglMap->setBounds(mbgl::BoundOptions().withMaxZoom(maximumZoomLevel));
}

- (double)maximumZoomLevel {
    return *_mbglMap->getBounds().maxZoom;
}

- (double)minimumZoomLevel {
    return *_mbglMap->getBounds().minZoom;
}

/// Respond to a click on the zoom control.
- (IBAction)zoomInOrOut:(NSSegmentedControl *)sender {
    switch (sender.selectedSegment) {
        case 0:
            // Zoom out.
            [self moveToEndOfParagraph:sender];
            break;
        case 1:
            // Zoom in.
            [self moveToBeginningOfParagraph:sender];
            break;
        default:
            break;
    }
}

+ (NSSet<NSString *> *)keyPathsForValuesAffectingDirection {
    return [NSSet setWithObject:@"camera"];
}

- (CLLocationDirection)direction {
    return mbgl::util::wrap(*_mbglMap->getCameraOptions().bearing, 0., 360.);
}

- (void)setDirection:(CLLocationDirection)direction {
    MLNLogDebug(@"Setting direction: %f", direction);
    [self setDirection:direction animated:NO];
}

- (void)setDirection:(CLLocationDirection)direction animated:(BOOL)animated {
    MLNLogDebug(@"Setting direction: %f animated: %@", direction, MLNStringFromBOOL(animated));
    [self willChangeValueForKey:@"direction"];
    _mbglMap->easeTo(mbgl::CameraOptions()
                         .withBearing(direction)
                         .withPadding(MLNEdgeInsetsFromNSEdgeInsets(self.contentInsets)),
                     MLNDurationFromTimeInterval(animated ? MLNAnimationDuration : 0));
    [self didChangeValueForKey:@"direction"];
}

- (void)offsetDirectionBy:(CLLocationDegrees)delta animated:(BOOL)animated {
    [self setDirection:*_mbglMap->getCameraOptions().bearing + delta animated:animated];
}

- (CGFloat)minimumPitch {
    return *_mbglMap->getBounds().minPitch;
}

- (void)setMinimumPitch:(CGFloat)minimumPitch {
    MLNLogDebug(@"Setting minimumPitch: %f", minimumPitch);
    _mbglMap->setBounds(mbgl::BoundOptions().withMinPitch(minimumPitch));
}

- (CGFloat)maximumPitch {
    return *_mbglMap->getBounds().maxPitch;
}

- (void)setMaximumPitch:(CGFloat)maximumPitch {
    MLNLogDebug(@"Setting maximumPitch: %f", maximumPitch);
    _mbglMap->setBounds(mbgl::BoundOptions().withMaxPitch(maximumPitch));
}

+ (NSSet<NSString *> *)keyPathsForValuesAffectingCamera {
    return [NSSet setWithObjects:@"latitude", @"longitude", @"centerCoordinate", @"zoomLevel", @"direction", nil];
}

- (MLNMapCamera *)camera {
    mbgl::EdgeInsets padding = MLNEdgeInsetsFromNSEdgeInsets(self.contentInsets);
    return [self cameraForCameraOptions:_mbglMap->getCameraOptions(padding)];
}

- (void)setCamera:(MLNMapCamera *)camera {
    MLNLogDebug(@"Setting camera: %@", camera);
    [self setCamera:camera animated:NO];
}

- (void)setCamera:(MLNMapCamera *)camera animated:(BOOL)animated {
    MLNLogDebug(@"Setting camera: %@ animated: %@", camera, MLNStringFromBOOL(animated));
    [self setCamera:camera withDuration:animated ? MLNAnimationDuration : 0 animationTimingFunction:nil completionHandler:NULL];
}

- (void)setCamera:(MLNMapCamera *)camera withDuration:(NSTimeInterval)duration animationTimingFunction:(nullable CAMediaTimingFunction *)function completionHandler:(nullable void (^)(void))completion {
    MLNLogDebug(@"Setting camera: %@ duration: %f animationTimingFunction: %@", camera, duration, function);
    [self setCamera:camera withDuration:duration animationTimingFunction:function edgePadding:NSEdgeInsetsZero completionHandler:completion];
}

- (void)setCamera:(MLNMapCamera *)camera withDuration:(NSTimeInterval)duration animationTimingFunction:(nullable CAMediaTimingFunction *)function edgePadding:(NSEdgeInsets)edgePadding completionHandler:(nullable void (^)(void))completion {
    edgePadding = MLNEdgeInsetsInsetEdgeInset(edgePadding, self.contentInsets);
    mbgl::AnimationOptions animationOptions;
    if (duration > 0) {
        animationOptions.duration.emplace(MLNDurationFromTimeInterval(duration));
        animationOptions.easing.emplace(MLNUnitBezierForMediaTimingFunction(function));
    }
    if (completion) {
        animationOptions.transitionFinishFn = [completion]() {
            // Must run asynchronously after the transition is completely over.
            // Otherwise, a call to -setCamera: within the completion handler
            // would reenter the completion handler’s caller.
            dispatch_async(dispatch_get_main_queue(), ^{
                completion();
            });
        };
    }

    if ([self.camera isEqualToMapCamera:camera]) {
        if (completion) {
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(duration * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                completion();
            });
        }
        return;
    }

    [self willChangeValueForKey:@"camera"];
    _mbglMap->cancelTransitions();
    mbgl::CameraOptions cameraOptions = [self cameraOptionsObjectForAnimatingToCamera:camera edgePadding:edgePadding];
    _mbglMap->easeTo(cameraOptions, animationOptions);
    [self didChangeValueForKey:@"camera"];
}

- (void)flyToCamera:(MLNMapCamera *)camera completionHandler:(nullable void (^)(void))completion {
    MLNLogDebug(@"Setting flyToCamera: %@ completionHandler: %@", camera, completion);
    [self flyToCamera:camera withDuration:-1 completionHandler:completion];
}

- (void)flyToCamera:(MLNMapCamera *)camera withDuration:(NSTimeInterval)duration completionHandler:(nullable void (^)(void))completion {
    MLNLogDebug(@"Setting flyToCamera: %@ withDuration: %f completionHandler: %@", camera, duration, completion);
    [self flyToCamera:camera withDuration:duration peakAltitude:-1 completionHandler:completion];
}

- (void)flyToCamera:(MLNMapCamera *)camera withDuration:(NSTimeInterval)duration peakAltitude:(CLLocationDistance)peakAltitude completionHandler:(nullable void (^)(void))completion {
    MLNLogDebug(@"Setting flyToCamera: %@ withDuration: %f peakAltitude: %f completionHandler: %@", camera, duration, peakAltitude, completion);
    mbgl::AnimationOptions animationOptions;
    if (duration >= 0) {
        animationOptions.duration = MLNDurationFromTimeInterval(duration);
    }
    if (peakAltitude >= 0) {
        CLLocationDegrees peakLatitude = (self.centerCoordinate.latitude + camera.centerCoordinate.latitude) / 2;
        CLLocationDegrees peakPitch = (self.camera.pitch + camera.pitch) / 2;
        animationOptions.minZoom = MLNZoomLevelForAltitude(peakAltitude, peakPitch,
                                                           peakLatitude, self.frame.size);
    }
    if (completion) {
        animationOptions.transitionFinishFn = [completion]() {
            // Must run asynchronously after the transition is completely over.
            // Otherwise, a call to -setCamera: within the completion handler
            // would reenter the completion handler’s caller.
            dispatch_async(dispatch_get_main_queue(), ^{
                completion();
            });
        };
    }

    if ([self.camera isEqualToMapCamera:camera]) {
        if (completion) {
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(duration * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                completion();
            });
        }
        return;
    }

    [self willChangeValueForKey:@"camera"];
    _mbglMap->cancelTransitions();
    mbgl::CameraOptions cameraOptions = [self cameraOptionsObjectForAnimatingToCamera:camera edgePadding:self.contentInsets];
    _mbglMap->flyTo(cameraOptions, animationOptions);
    [self didChangeValueForKey:@"camera"];
}

/// Returns a CameraOptions object that specifies parameters for animating to
/// the given camera.
- (mbgl::CameraOptions)cameraOptionsObjectForAnimatingToCamera:(MLNMapCamera *)camera edgePadding:(NSEdgeInsets) edgePadding {
    mbgl::CameraOptions options;
    options.center = MLNLatLngFromLocationCoordinate2D(camera.centerCoordinate);
    options.padding = MLNEdgeInsetsFromNSEdgeInsets(edgePadding);
    options.zoom = MLNZoomLevelForAltitude(camera.altitude, camera.pitch,
                                           camera.centerCoordinate.latitude,
                                           self.frame.size);
    if (camera.heading >= 0) {
        options.bearing = camera.heading;
    }
    if (camera.pitch >= 0) {
        options.pitch = camera.pitch;
    }
    return options;
}

+ (NSSet *)keyPathsForValuesAffectingVisibleCoordinateBounds {
    return [NSSet setWithObjects:@"centerCoordinate", @"zoomLevel", @"direction", @"bounds", nil];
}

- (MLNCoordinateBounds)visibleCoordinateBounds {
    return [self convertRect:self.bounds toCoordinateBoundsFromView:self];
}

- (void)setVisibleCoordinateBounds:(MLNCoordinateBounds)bounds {
    MLNLogDebug(@"Setting visibleCoordinateBounds: %@", MLNStringFromCoordinateBounds(bounds));
    [self setVisibleCoordinateBounds:bounds animated:NO];
}

- (void)setVisibleCoordinateBounds:(MLNCoordinateBounds)bounds animated:(BOOL)animated {
    MLNLogDebug(@"Setting visibleCoordinateBounds: %@ animated: %@", MLNStringFromCoordinateBounds(bounds), MLNStringFromBOOL(animated));
    [self setVisibleCoordinateBounds:bounds edgePadding:NSEdgeInsetsZero animated:animated];
}

- (void)setVisibleCoordinateBounds:(MLNCoordinateBounds)bounds edgePadding:(NSEdgeInsets)insets animated:(BOOL)animated {
    [self setVisibleCoordinateBounds:bounds edgePadding:insets animated:animated completionHandler:nil];
}

- (void)setVisibleCoordinateBounds:(MLNCoordinateBounds)bounds edgePadding:(NSEdgeInsets)insets animated:(BOOL)animated completionHandler:(nullable void (^)(void))completion {
    _mbglMap->cancelTransitions();

    mbgl::EdgeInsets padding = MLNEdgeInsetsFromNSEdgeInsets(insets);
    padding += MLNEdgeInsetsFromNSEdgeInsets(self.contentInsets);
    mbgl::CameraOptions cameraOptions = _mbglMap->cameraForLatLngBounds(MLNLatLngBoundsFromCoordinateBounds(bounds), padding);
    mbgl::AnimationOptions animationOptions;
    if (animated) {
        animationOptions.duration = MLNDurationFromTimeInterval(MLNAnimationDuration);
    }

    MLNMapCamera *camera = [self cameraForCameraOptions:cameraOptions];
    if ([self.camera isEqualToMapCamera:camera]) {
        completion();
        return;
    }

    [self willChangeValueForKey:@"visibleCoordinateBounds"];
    animationOptions.transitionFinishFn = ^() {
        [self didChangeValueForKey:@"visibleCoordinateBounds"];
        if (completion) {
            dispatch_async(dispatch_get_main_queue(), ^{
                completion();
            });
        }
    };
    _mbglMap->easeTo(cameraOptions, animationOptions);
}

- (MLNMapCamera *)cameraThatFitsCoordinateBounds:(MLNCoordinateBounds)bounds {
    return [self cameraThatFitsCoordinateBounds:bounds edgePadding:NSEdgeInsetsZero];
}

- (MLNMapCamera *)cameraThatFitsCoordinateBounds:(MLNCoordinateBounds)bounds edgePadding:(NSEdgeInsets)insets {
    mbgl::EdgeInsets padding = MLNEdgeInsetsFromNSEdgeInsets(insets);
    padding += MLNEdgeInsetsFromNSEdgeInsets(self.contentInsets);
    mbgl::CameraOptions cameraOptions = _mbglMap->cameraForLatLngBounds(MLNLatLngBoundsFromCoordinateBounds(bounds), padding);
    return [self cameraForCameraOptions:cameraOptions];
}

- (MLNMapCamera *)camera:(MLNMapCamera *)camera fittingCoordinateBounds:(MLNCoordinateBounds)bounds edgePadding:(NSEdgeInsets)insets
{
    mbgl::EdgeInsets padding = MLNEdgeInsetsFromNSEdgeInsets(insets);
    padding += MLNEdgeInsetsFromNSEdgeInsets(self.contentInsets);

    MLNMapCamera *currentCamera = self.camera;
    CGFloat pitch = camera.pitch < 0 ? currentCamera.pitch : camera.pitch;
    CLLocationDirection direction = camera.heading < 0 ? currentCamera.heading : camera.heading;

    mbgl::CameraOptions cameraOptions = _mbglMap->cameraForLatLngBounds(MLNLatLngBoundsFromCoordinateBounds(bounds), padding, direction, pitch);
    return [self cameraForCameraOptions:cameraOptions];
}

- (MLNMapCamera *)camera:(MLNMapCamera *)camera fittingShape:(MLNShape *)shape edgePadding:(NSEdgeInsets)insets {
    mbgl::EdgeInsets padding = MLNEdgeInsetsFromNSEdgeInsets(insets);
    padding += MLNEdgeInsetsFromNSEdgeInsets(self.contentInsets);

    MLNMapCamera *currentCamera = self.camera;
    CGFloat pitch = camera.pitch < 0 ? currentCamera.pitch : camera.pitch;
    CLLocationDirection direction = camera.heading < 0 ? currentCamera.heading : camera.heading;

    mbgl::CameraOptions cameraOptions = _mbglMap->cameraForGeometry([shape geometryObject], padding, direction, pitch);

    return [self cameraForCameraOptions: cameraOptions];
}

- (MLNMapCamera *)cameraThatFitsShape:(MLNShape *)shape direction:(CLLocationDirection)direction edgePadding:(NSEdgeInsets)insets {
    mbgl::EdgeInsets padding = MLNEdgeInsetsFromNSEdgeInsets(insets);
    padding += MLNEdgeInsetsFromNSEdgeInsets(self.contentInsets);

    mbgl::CameraOptions cameraOptions = _mbglMap->cameraForGeometry([shape geometryObject], padding, direction);

    return [self cameraForCameraOptions:cameraOptions];
}

- (MLNMapCamera *)cameraForCameraOptions:(const mbgl::CameraOptions &)cameraOptions {
    mbgl::CameraOptions mapCamera = _mbglMap->getCameraOptions();
    CLLocationCoordinate2D centerCoordinate = MLNLocationCoordinate2DFromLatLng(cameraOptions.center ? *cameraOptions.center : *mapCamera.center);
    double zoomLevel = cameraOptions.zoom ? *cameraOptions.zoom : self.zoomLevel;
    CLLocationDirection direction = cameraOptions.bearing ? mbgl::util::wrap(*cameraOptions.bearing, 0., 360.) : self.direction;
    CGFloat pitch = cameraOptions.pitch ? *cameraOptions.pitch : *mapCamera.pitch;
    CLLocationDistance altitude = MLNAltitudeForZoomLevel(zoomLevel, pitch,
                                                          centerCoordinate.latitude,
                                                          self.frame.size);
    return [MLNMapCamera cameraLookingAtCenterCoordinate:centerCoordinate
                                                altitude:altitude
                                                   pitch:pitch
                                                 heading:direction];
}

- (void)setAutomaticallyAdjustsContentInsets:(BOOL)automaticallyAdjustsContentInsets {
    _automaticallyAdjustsContentInsets = automaticallyAdjustsContentInsets;
    [self adjustContentInsets];
}

/// Updates `contentInsets` to reflect the current window geometry.
- (void)adjustContentInsets {
    if (!_automaticallyAdjustsContentInsets) {
        return;
    }

    NSEdgeInsets contentInsets = self.contentInsets;
    if ((self.window.styleMask & NSWindowStyleMaskFullSizeContentView)
        && !self.window.titlebarAppearsTransparent) {
        NSRect contentLayoutRect = [self convertRect:self.window.contentLayoutRect fromView:nil];
        if (NSMaxX(contentLayoutRect) > 0 && NSMaxY(contentLayoutRect) > 0) {
            contentInsets = NSEdgeInsetsMake(NSHeight(self.bounds) - NSMaxY(contentLayoutRect),
                                             MAX(NSMinX(contentLayoutRect), 0),
                                             MAX(NSMinY(contentLayoutRect), 0),
                                             NSWidth(self.bounds) - NSMaxX(contentLayoutRect));
        }
    } else {
        contentInsets = NSEdgeInsetsZero;
    }

    self.contentInsets = contentInsets;
}

- (void)setContentInsets:(NSEdgeInsets)contentInsets {
    [self setContentInsets:contentInsets animated:NO completionHandler:nil];
}

- (void)setContentInsets:(NSEdgeInsets)contentInsets animated:(BOOL)animated {
    [self setContentInsets:contentInsets animated:animated completionHandler:nil];
}

- (void)setContentInsets:(NSEdgeInsets)contentInsets animated:(BOOL)animated completionHandler:(nullable void (^)(void))completion {
    if (NSEdgeInsetsEqual(contentInsets, self.contentInsets)) {
        if (completion) {
            completion();
        }
        return;
    }
    MLNLogDebug(@"Setting contentInset: %@ animated:", MLNStringFromNSEdgeInsets(contentInsets), MLNStringFromBOOL(animated));
    // After adjusting the content insets, move the center coordinate from the
    // old frame of reference to the new one represented by the newly set
    // content insets.
    CLLocationCoordinate2D oldCenter = self.centerCoordinate;
    _contentInsets = contentInsets;
    [self setCenterCoordinate:oldCenter animated:animated completionHandler:completion];
}

// MARK: Mouse events and gestures

- (BOOL)acceptsFirstResponder {
    return YES;
}

/// Drag to pan, plus drag to zoom, rotate, and tilt when a modifier key is held
/// down.
- (void)handlePanGesture:(NSPanGestureRecognizer *)gestureRecognizer {
    NSPoint delta = [gestureRecognizer translationInView:self];
    NSPoint endPoint = [gestureRecognizer locationInView:self];
    NSPoint startPoint = NSMakePoint(endPoint.x - delta.x, endPoint.y - delta.y);

    NSEventModifierFlags flags = [NSApp currentEvent].modifierFlags;
    if (gestureRecognizer.state == NSGestureRecognizerStateBegan) {
        [self.window invalidateCursorRectsForView:self];
        _mbglMap->setGestureInProgress(true);

        if (![self isPanningWithGesture]) {
            // Hide the cursor except when panning.
            CGDisplayHideCursor(kCGDirectMainDisplay);
            _didHideCursorDuringGesture = YES;
        }
    } else if (gestureRecognizer.state == NSGestureRecognizerStateEnded
               || gestureRecognizer.state == NSGestureRecognizerStateCancelled) {
        _mbglMap->setGestureInProgress(false);
        [self.window invalidateCursorRectsForView:self];

        if (_didHideCursorDuringGesture) {
            _didHideCursorDuringGesture = NO;
            // Move the cursor back to the start point and show it again, creating
            // the illusion that it has stayed in place during the entire gesture.
            CGPoint cursorPoint = [self convertPoint:startPoint toView:nil];
            cursorPoint = [self.window convertRectToScreen:{ cursorPoint, NSZeroSize }].origin;
            cursorPoint.y = self.window.screen.frame.size.height - cursorPoint.y;
            CGDisplayMoveCursorToPoint(kCGDirectMainDisplay, cursorPoint);
            CGDisplayShowCursor(kCGDirectMainDisplay);
        }
    }
    if (flags & NSEventModifierFlagShift) {
        // Shift-drag to zoom.
        if (!self.zoomEnabled) {
            return;
        }

        _mbglMap->cancelTransitions();

        if (gestureRecognizer.state == NSGestureRecognizerStateBegan) {
            _zoomAtBeginningOfGesture = [self zoomLevel];
        } else if (gestureRecognizer.state == NSGestureRecognizerStateChanged) {
            CGFloat newZoomLevel = _zoomAtBeginningOfGesture - delta.y / 75;
            [self setZoomLevel:newZoomLevel atPoint:startPoint animated:NO];
        }
    } else if (flags & NSEventModifierFlagOption) {
        // Option-drag to rotate and/or tilt.
        _mbglMap->cancelTransitions();

        if (gestureRecognizer.state == NSGestureRecognizerStateBegan) {
            _directionAtBeginningOfGesture = self.direction;
            _pitchAtBeginningOfGesture = *_mbglMap->getCameraOptions().pitch;
        } else if (gestureRecognizer.state == NSGestureRecognizerStateChanged) {
            MLNMapCamera *oldCamera = self.camera;
            BOOL didChangeCamera = NO;
            mbgl::ScreenCoordinate center(startPoint.x, self.bounds.size.height - startPoint.y);
            if (self.rotateEnabled) {
                CLLocationDirection newDirection = _directionAtBeginningOfGesture - delta.x / 10;
                [self willChangeValueForKey:@"direction"];
                _mbglMap->jumpTo(mbgl::CameraOptions().withBearing(newDirection).withAnchor(center));
                didChangeCamera = YES;
                [self didChangeValueForKey:@"direction"];
            }
            if (self.pitchEnabled) {
                _mbglMap->jumpTo(mbgl::CameraOptions().withPitch(_pitchAtBeginningOfGesture + delta.y / 5).withAnchor(center));
                didChangeCamera = YES;
            }

            if (didChangeCamera
                && [self.delegate respondsToSelector:@selector(mapView:shouldChangeFromCamera:toCamera:)]
                && ![self.delegate mapView:self shouldChangeFromCamera:oldCamera toCamera:self.camera]) {
                self.camera = oldCamera;
            }
        }
    } else if (self.scrollEnabled) {
        // Otherwise, drag to pan.
        _mbglMap->cancelTransitions();

        if (gestureRecognizer.state == NSGestureRecognizerStateChanged) {
            delta.y *= -1;
            [self offsetCenterCoordinateBy:delta animated:NO];
            [gestureRecognizer setTranslation:NSZeroPoint inView:nil];
        }
    }
}

/// Returns whether the user is panning using a gesture.
- (BOOL)isPanningWithGesture {
  NSGestureRecognizerState state = _panGestureRecognizer.state;
  NSEventModifierFlags flags = [NSApp currentEvent].modifierFlags;
  return ((state == NSGestureRecognizerStateBegan || state == NSGestureRecognizerStateChanged)
          && !(flags & NSEventModifierFlagShift || flags & NSEventModifierFlagOption));
}

/// Pinch to zoom.
- (void)handleMagnificationGesture:(NSMagnificationGestureRecognizer *)gestureRecognizer {
    if (!self.zoomEnabled) {
        return;
    }

    _mbglMap->cancelTransitions();

    if (gestureRecognizer.state == NSGestureRecognizerStateBegan) {
        _mbglMap->setGestureInProgress(true);
        _zoomAtBeginningOfGesture = [self zoomLevel];
    } else if (gestureRecognizer.state == NSGestureRecognizerStateChanged) {
        NSPoint zoomInPoint = [gestureRecognizer locationInView:self];
        mbgl::ScreenCoordinate center(zoomInPoint.x, self.bounds.size.height - zoomInPoint.y);
        if (gestureRecognizer.magnification > -1) {
            [self willChangeValueForKey:@"zoomLevel"];
            [self willChangeValueForKey:@"centerCoordinate"];
            MLNMapCamera *oldCamera = self.camera;
            _mbglMap->jumpTo(mbgl::CameraOptions()
                                 .withZoom(_zoomAtBeginningOfGesture + log2(1 + gestureRecognizer.magnification))
                                 .withAnchor(center));
            if ([self.delegate respondsToSelector:@selector(mapView:shouldChangeFromCamera:toCamera:)]
                && ![self.delegate mapView:self shouldChangeFromCamera:oldCamera toCamera:self.camera]) {
                self.camera = oldCamera;
            }
            [self didChangeValueForKey:@"centerCoordinate"];
            [self didChangeValueForKey:@"zoomLevel"];
        }
    } else if (gestureRecognizer.state == NSGestureRecognizerStateEnded
               || gestureRecognizer.state == NSGestureRecognizerStateCancelled) {
        _mbglMap->setGestureInProgress(false);
    }
}

/// Click or tap to select an annotation.
- (void)handleClickGesture:(NSClickGestureRecognizer *)gestureRecognizer {
    if (gestureRecognizer.state != NSGestureRecognizerStateEnded
        || [self subviewContainingGesture:gestureRecognizer]) {
        return;
    }

    NSPoint gesturePoint = [gestureRecognizer locationInView:self];
    MLNAnnotationTag hitAnnotationTag = [self annotationTagAtPoint:gesturePoint persistingResults:YES];
    if (hitAnnotationTag != MLNAnnotationTagNotFound) {
        if (hitAnnotationTag != _selectedAnnotationTag) {
            id <MLNAnnotation> annotation = [self annotationWithTag:hitAnnotationTag];
            NSAssert(annotation, @"Cannot select nonexistent annotation with tag %llu", hitAnnotationTag);
            [self selectAnnotation:annotation atPoint:gesturePoint];
        }
    } else {
        [self deselectAnnotation:self.selectedAnnotation];
    }
}

/// Right-click to show the context menu.
- (void)handleRightClickGesture:(NSClickGestureRecognizer *)gestureRecognizer {
    NSMenu *menu = [self menuForEvent:[NSApp currentEvent]];
    if (menu) {
        [NSMenu popUpContextMenu:menu withEvent:[NSApp currentEvent] forView:self];
    }
}

/// Double-click or double-tap to zoom in.
- (void)handleDoubleClickGesture:(NSClickGestureRecognizer *)gestureRecognizer {
    if (!self.zoomEnabled || gestureRecognizer.state != NSGestureRecognizerStateEnded
        || [self subviewContainingGesture:gestureRecognizer]) {
        return;
    }

    _mbglMap->cancelTransitions();

    NSPoint gesturePoint = [gestureRecognizer locationInView:self];
    [self setZoomLevel:round(self.zoomLevel) + 1 atPoint:gesturePoint animated:YES];
}

- (void)smartMagnifyWithEvent:(NSEvent *)event {
    if (!self.zoomEnabled) {
        return;
    }

    _mbglMap->cancelTransitions();

    // Tap with two fingers (“right-click”) to zoom out on mice but not trackpads.
    NSPoint gesturePoint = [self convertPoint:event.locationInWindow fromView:nil];
    [self setZoomLevel:round(self.zoomLevel) - 1 atPoint:gesturePoint animated:YES];
}

/// Rotate fingers to rotate.
- (void)handleRotationGesture:(NSRotationGestureRecognizer *)gestureRecognizer {
    if (!self.rotateEnabled) {
        return;
    }

    _mbglMap->cancelTransitions();

    if (gestureRecognizer.state == NSGestureRecognizerStateBegan) {
        _mbglMap->setGestureInProgress(true);
        _directionAtBeginningOfGesture = self.direction;
    } else if (gestureRecognizer.state == NSGestureRecognizerStateChanged) {
        MLNMapCamera *oldCamera = self.camera;

        NSPoint rotationPoint = [gestureRecognizer locationInView:self];
        mbgl::ScreenCoordinate anchor(rotationPoint.x, self.bounds.size.height - rotationPoint.y);
        _mbglMap->jumpTo(mbgl::CameraOptions()
                             .withBearing(_directionAtBeginningOfGesture + gestureRecognizer.rotationInDegrees)
                             .withAnchor(anchor));

        if ([self.delegate respondsToSelector:@selector(mapView:shouldChangeFromCamera:toCamera:)]
            && ![self.delegate mapView:self shouldChangeFromCamera:oldCamera toCamera:self.camera]) {
            self.camera = oldCamera;
        }
    } else if (gestureRecognizer.state == NSGestureRecognizerStateEnded
               || gestureRecognizer.state == NSGestureRecognizerStateCancelled) {
        _mbglMap->setGestureInProgress(false);
    }
}

- (BOOL)wantsScrollEventsForSwipeTrackingOnAxis:(__unused NSEventGestureAxis)axis {
    // Track both horizontal and vertical swipes in -scrollWheel:.
    return YES;
}

- (void)scrollWheel:(NSEvent *)event {
    // https://developer.apple.com/library/mac/releasenotes/AppKit/RN-AppKitOlderNotes/#10_7Dragging
    BOOL isScrollWheel = event.phase == NSEventPhaseNone && event.momentumPhase == NSEventPhaseNone && !event.hasPreciseScrollingDeltas;
    if (isScrollWheel || [[NSUserDefaults standardUserDefaults] boolForKey:MLNScrollWheelZoomsMapViewDefaultKey]) {
        // A traditional, vertical scroll wheel zooms instead of panning.
        if (self.zoomEnabled) {
            const double delta =
                event.scrollingDeltaY / ([event hasPreciseScrollingDeltas] ? 100 : 10);
            if (delta != 0) {
                double scale = 2.0 / (1.0 + std::exp(-std::abs(delta)));

                // Zooming out.
                if (delta < 0) {
                    scale = 1.0 / scale;
                }

                NSPoint gesturePoint = [self convertPoint:event.locationInWindow fromView:nil];
                [self setZoomLevel:self.zoomLevel + log2(scale) atPoint:gesturePoint animated:NO];
            }
        }
    } else if (self.scrollEnabled
               && _magnificationGestureRecognizer.state == NSGestureRecognizerStatePossible
               && _rotationGestureRecognizer.state == NSGestureRecognizerStatePossible) {
        // Scroll to pan.
        _mbglMap->cancelTransitions();

        CGFloat x = event.scrollingDeltaX;
        CGFloat y = event.scrollingDeltaY;
        if (x || y) {
            [self offsetCenterCoordinateBy:NSMakePoint(x, y) animated:NO];
        }

        // Drift pan.
        if (event.momentumPhase != NSEventPhaseNone) {
            [self offsetCenterCoordinateBy:NSMakePoint(x, y) animated:NO];
        }
    }
}

/// Returns the subview that the gesture is located in.
- (NSView *)subviewContainingGesture:(NSGestureRecognizer *)gestureRecognizer {
    if (NSPointInRect([gestureRecognizer locationInView:self.compass], self.compass.bounds)) {
        return self.compass;
    }
    if (NSPointInRect([gestureRecognizer locationInView:self.zoomControls], self.zoomControls.bounds)) {
        return self.zoomControls;
    }
    if (NSPointInRect([gestureRecognizer locationInView:self.attributionView], self.attributionView.bounds)) {
        return self.attributionView;
    }
    return nil;
}

// MARK: NSGestureRecognizerDelegate methods
- (BOOL)gestureRecognizer:(NSGestureRecognizer *)gestureRecognizer shouldAttemptToRecognizeWithEvent:(NSEvent *)event {
    if (gestureRecognizer == _singleClickRecognizer) {
        if (!self.selectedAnnotation) {
            NSPoint gesturePoint = [self convertPoint:[event locationInWindow] fromView:nil];
            MLNAnnotationTag hitAnnotationTag = [self annotationTagAtPoint:gesturePoint persistingResults:NO];
            if (hitAnnotationTag == MLNAnnotationTagNotFound) {
                return NO;
            }
        }
    }
    return YES;
}

// MARK: Keyboard events

- (void)keyDown:(NSEvent *)event {
    // This is the recommended way to handle arrow key presses, causing
    // methods like -moveUp: and -moveToBeginningOfParagraph: to be called
    // for various standard keybindings.
    [self interpretKeyEvents:@[event]];
}

// The following action methods are declared in NSResponder.h.

- (void)insertTab:(id)sender {
    if (self.window.firstResponder == self) {
        [self.window selectNextKeyView:self];
    }
}

- (void)insertBacktab:(id)sender {
    if (self.window.firstResponder == self) {
        [self.window selectPreviousKeyView:self];
    }
}

- (void)insertText:(NSString *)insertString {
    switch (insertString.length == 1 ? [insertString characterAtIndex:0] : 0) {
        case '-':
            [self moveToEndOfParagraph:nil];
            break;

        case '+':
        case '=':
            [self moveToBeginningOfParagraph:nil];
            break;

        default:
            [super insertText:insertString];
            break;
    }
}

- (IBAction)moveUp:(__unused id)sender {
    [self offsetCenterCoordinateBy:NSMakePoint(0, MLNKeyPanningIncrement) animated:YES];
}

- (IBAction)moveDown:(__unused id)sender {
    [self offsetCenterCoordinateBy:NSMakePoint(0, -MLNKeyPanningIncrement) animated:YES];
}

- (IBAction)moveLeft:(__unused id)sender {
    [self offsetCenterCoordinateBy:NSMakePoint(MLNKeyPanningIncrement, 0) animated:YES];
}

- (IBAction)moveRight:(__unused id)sender {
    [self offsetCenterCoordinateBy:NSMakePoint(-MLNKeyPanningIncrement, 0) animated:YES];
}

- (IBAction)moveToBeginningOfParagraph:(__unused id)sender {
    if (self.zoomEnabled) {
        [self setZoomLevel:round(self.zoomLevel) + 1 animated:YES];
    }
}

- (IBAction)moveToEndOfParagraph:(__unused id)sender {
    if (self.zoomEnabled) {
        [self setZoomLevel:round(self.zoomLevel) - 1 animated:YES];
    }
}

- (IBAction)moveWordLeft:(__unused id)sender {
    if (self.rotateEnabled) {
        [self offsetDirectionBy:MLNKeyRotationIncrement animated:YES];
    }
}

- (IBAction)moveWordRight:(__unused id)sender {
    if (self.rotateEnabled) {
        [self offsetDirectionBy:-MLNKeyRotationIncrement animated:YES];
    }
}

- (void)setZoomEnabled:(BOOL)zoomEnabled {
    _zoomEnabled = zoomEnabled;
    _zoomControls.enabled = zoomEnabled;
    _zoomControls.hidden = !zoomEnabled;
}

- (void)setRotateEnabled:(BOOL)rotateEnabled {
    _rotateEnabled = rotateEnabled;
    _compass.enabled = rotateEnabled;
    _compass.hidden = !rotateEnabled;
}

// MARK: Ornaments

/// Updates the zoom controls’ enabled state based on the current zoom level.
- (void)updateZoomControls {
    [_zoomControls setEnabled:self.zoomLevel > self.minimumZoomLevel forSegment:0];
    [_zoomControls setEnabled:self.zoomLevel < self.maximumZoomLevel forSegment:1];
}

/// Updates the compass to point in the same direction as the map.
- (void)updateCompass {
    // The circular slider control goes counterclockwise, whereas our map
    // measures its direction clockwise.
    _compass.doubleValue = -self.direction;
}

- (IBAction)rotate:(NSSlider *)sender {
    [self setDirection:-sender.doubleValue animated:YES];
}

// MARK: Annotations

- (nullable NSArray<id <MLNAnnotation>> *)annotations {
    if (_annotationContextsByAnnotationTag.empty()) {
        return nil;
    }

    // Map all the annotation tags to the annotations themselves.
    std::vector<id <MLNAnnotation>> annotations;
    std::transform(_annotationContextsByAnnotationTag.begin(),
                   _annotationContextsByAnnotationTag.end(),
                   std::back_inserter(annotations),
                   ^ id <MLNAnnotation> (const std::pair<MLNAnnotationTag, MLNAnnotationContext> &pair) {
                       return pair.second.annotation;
                   });
    return [NSArray arrayWithObjects:&annotations[0] count:annotations.size()];
}

- (nullable NSArray<id <MLNAnnotation>> *)visibleAnnotations
{
    return [self visibleAnnotationsInRect:self.bounds];
}

- (nullable NSArray<id <MLNAnnotation>> *)visibleAnnotationsInRect:(CGRect)rect
{
    if (_annotationContextsByAnnotationTag.empty())
    {
        return nil;
    }

    std::vector<MLNAnnotationTag> annotationTags = [self annotationTagsInRect:rect];
    std::vector<MLNAnnotationTag> shapeAnnotationTags = [self shapeAnnotationTagsInRect:rect];

    if (shapeAnnotationTags.size()) {
        annotationTags.insert(annotationTags.end(), shapeAnnotationTags.begin(), shapeAnnotationTags.end());
    }

    if (annotationTags.size())
    {
        NSMutableArray *annotations = [NSMutableArray arrayWithCapacity:annotationTags.size()];

        for (auto const& annotationTag: annotationTags)
        {
            if (!_annotationContextsByAnnotationTag.count(annotationTag) ||
                annotationTag == MLNAnnotationTagNotFound)
            {
                continue;
            }

            MLNAnnotationContext annotationContext = _annotationContextsByAnnotationTag.at(annotationTag);
            NSAssert(annotationContext.annotation, @"Missing annotation for tag %llu.", annotationTag);
            if (annotationContext.annotation)
            {
                [annotations addObject:annotationContext.annotation];
            }
        }

        return [annotations copy];
    }

    return nil;
}

/// Returns the annotation assigned the given tag. Cheap.
- (id <MLNAnnotation>)annotationWithTag:(MLNAnnotationTag)tag {
    if ( ! _annotationContextsByAnnotationTag.count(tag) ||
        tag == MLNAnnotationTagNotFound) {
        return nil;
    }

    MLNAnnotationContext &annotationContext = _annotationContextsByAnnotationTag.at(tag);
    return annotationContext.annotation;
}

/// Returns the annotation tag assigned to the given annotation.
- (MLNAnnotationTag)annotationTagForAnnotation:(id <MLNAnnotation>)annotation {
    if (!annotation || _annotationTagsByAnnotation.count(annotation) == 0) {
        return MLNAnnotationTagNotFound;
    }

    return  _annotationTagsByAnnotation.at(annotation);
}

- (void)addAnnotation:(id <MLNAnnotation>)annotation {
    if (annotation) {
        [self addAnnotations:@[annotation]];
    }
}

- (void)addAnnotations:(NSArray<id <MLNAnnotation>> *)annotations {
    if (!annotations) {
        return;
    }

    [self willChangeValueForKey:@"annotations"];

    BOOL delegateHasImagesForAnnotations = [self.delegate respondsToSelector:@selector(mapView:imageForAnnotation:)];

    for (id <MLNAnnotation> annotation in annotations) {
        NSAssert([annotation conformsToProtocol:@protocol(MLNAnnotation)], @"Annotation does not conform to MLNAnnotation");

        // adding the same annotation object twice is a no-op
        if (_annotationTagsByAnnotation.count(annotation) != 0) {
            continue;
        }

        if ([annotation isKindOfClass:[MLNMultiPoint class]]) {
            // The multipoint knows how to style itself (with the map view’s help).
            MLNMultiPoint *multiPoint = (MLNMultiPoint *)annotation;
            if (!multiPoint.pointCount) {
                continue;
            }

            _isChangingAnnotationLayers = YES;
            MLNAnnotationTag annotationTag = _mbglMap->addAnnotation([multiPoint annotationObjectWithDelegate:self]);
            MLNAnnotationContext context;
            context.annotation = annotation;
            _annotationContextsByAnnotationTag[annotationTag] = context;
            _annotationTagsByAnnotation[annotation] = annotationTag;

            [(NSObject *)annotation addObserver:self forKeyPath:@"coordinates" options:0 context:(void *)(NSUInteger)annotationTag];
        } else if (![annotation isKindOfClass:[MLNMultiPolyline class]]
                   && ![annotation isKindOfClass:[MLNMultiPolygon class]]
                   && ![annotation isKindOfClass:[MLNShapeCollection class]]
                   && ![annotation isKindOfClass:[MLNPointCollection class]]) {
            MLNAnnotationImage *annotationImage = nil;
            if (delegateHasImagesForAnnotations) {
                annotationImage = [self.delegate mapView:self imageForAnnotation:annotation];
            }
            if (!annotationImage) {
                annotationImage = [self dequeueReusableAnnotationImageWithIdentifier:MLNDefaultStyleMarkerSymbolName];
            }
            if (!annotationImage) {
                annotationImage = self.defaultAnnotationImage;
            }

            NSString *symbolName = annotationImage.styleIconIdentifier;
            if (!symbolName) {
                symbolName = [MLNAnnotationSpritePrefix stringByAppendingString:annotationImage.reuseIdentifier];
                annotationImage.styleIconIdentifier = symbolName;
            }

            if (!self.annotationImagesByIdentifier[annotationImage.reuseIdentifier]) {
                self.annotationImagesByIdentifier[annotationImage.reuseIdentifier] = annotationImage;
                [self installAnnotationImage:annotationImage];
            }

            MLNAnnotationTag annotationTag = _mbglMap->addAnnotation(mbgl::SymbolAnnotation {
                MLNPointFromLocationCoordinate2D(annotation.coordinate),
                symbolName.UTF8String ?: ""
            });

            MLNAnnotationContext context;
            context.annotation = annotation;
            context.imageReuseIdentifier = annotationImage.reuseIdentifier;
            _annotationContextsByAnnotationTag[annotationTag] = context;
            _annotationTagsByAnnotation[annotation] = annotationTag;

            if ([annotation isKindOfClass:[NSObject class]]) {
                NSAssert(![annotation isKindOfClass:[MLNMultiPoint class]], @"Point annotation should not be MLNMultiPoint.");
                [(NSObject *)annotation addObserver:self forKeyPath:@"coordinate" options:0 context:(void *)(NSUInteger)annotationTag];
            }

            // Opt into potentially expensive tooltip tracking areas.
            if ([annotation respondsToSelector:@selector(toolTip)] && annotation.toolTip.length) {
                _wantsToolTipRects = YES;
            }
        }
    }

    [self didChangeValueForKey:@"annotations"];
    if (_isChangingAnnotationLayers) {
        [self.style willChangeValueForKey:@"layers"];
    }

    [self updateAnnotationTrackingAreas];
}

/// Initializes and returns a default annotation image that depicts a round pin
/// rising from the center, with a shadow slightly below center. The alignment
/// rect therefore excludes the bottom half.
- (MLNAnnotationImage *)defaultAnnotationImage {
    NSImage *image = MLNDefaultMarkerImage();
    NSRect alignmentRect = image.alignmentRect;
    alignmentRect.origin.y = NSMidY(alignmentRect);
    alignmentRect.size.height /= 2;
    image.alignmentRect = alignmentRect;
    return [MLNAnnotationImage annotationImageWithImage:image
                                        reuseIdentifier:MLNDefaultStyleMarkerSymbolName];
}

/// Sends the raw pixel data of the annotation image’s image to mbgl and
/// calculates state needed for hit testing later.
- (void)installAnnotationImage:(MLNAnnotationImage *)annotationImage {
    NSString *iconIdentifier = annotationImage.styleIconIdentifier;
    self.annotationImagesByIdentifier[annotationImage.reuseIdentifier] = annotationImage;

    NSImage *image = annotationImage.image;
    NSSize size = image.size;
    if (size.width == 0 || size.height == 0 || !image.valid) {
        // Can’t create an empty sprite. An image that hasn’t loaded is also useless.
        return;
    }

    _mbglMap->addAnnotationImage([annotationImage.image mgl_styleImageWithIdentifier:iconIdentifier]);

    // Create a slop area with a “radius” equal to the annotation image’s entire
    // size, allowing the eventual click to be on any point within this image.
    // Union this slop area with any existing slop areas.
    _unionedAnnotationImageSize = NSMakeSize(MAX(_unionedAnnotationImageSize.width, size.width),
                                             MAX(_unionedAnnotationImageSize.height, size.height));

    // Opt into potentially expensive cursor tracking areas.
    if (annotationImage.cursor) {
        _wantsCursorRects = YES;
    }
}

- (void)removeAnnotation:(id <MLNAnnotation>)annotation {
    if (annotation) {
        [self removeAnnotations:@[annotation]];
    }
}

- (void)removeAnnotations:(NSArray<id <MLNAnnotation>> *)annotations {
    if (!annotations) {
        return;
    }

    [self willChangeValueForKey:@"annotations"];

    for (id <MLNAnnotation> annotation in annotations) {
        NSAssert([annotation conformsToProtocol:@protocol(MLNAnnotation)], @"Annotation does not conform to MLNAnnotation");

        MLNAnnotationTag annotationTag = [self annotationTagForAnnotation:annotation];
        NSAssert(annotationTag != MLNAnnotationTagNotFound, @"No ID for annotation %@", annotation);

        if (annotationTag == _selectedAnnotationTag) {
            [self deselectAnnotation:annotation];
        }
        if (annotationTag == _lastSelectedAnnotationTag) {
            _lastSelectedAnnotationTag = MLNAnnotationTagNotFound;
        }

        _annotationContextsByAnnotationTag.erase(annotationTag);
        _annotationTagsByAnnotation.erase(annotation);

        if ([annotation isKindOfClass:[NSObject class]] &&
            ![annotation isKindOfClass:[MLNMultiPoint class]]) {
            [(NSObject *)annotation removeObserver:self forKeyPath:@"coordinate" context:(void *)(NSUInteger)annotationTag];
        } else if ([annotation isKindOfClass:[MLNMultiPoint class]]) {
            [(NSObject *)annotation removeObserver:self forKeyPath:@"coordinates" context:(void *)(NSUInteger)annotationTag];
        }

        _isChangingAnnotationLayers = YES;
        _mbglMap->removeAnnotation(annotationTag);
    }

    [self didChangeValueForKey:@"annotations"];
    if (_isChangingAnnotationLayers) {
        [self.style willChangeValueForKey:@"layers"];
    }

    [self updateAnnotationTrackingAreas];
}

- (nullable MLNAnnotationImage *)dequeueReusableAnnotationImageWithIdentifier:(NSString *)identifier {
    return self.annotationImagesByIdentifier[identifier];
}

- (id <MLNAnnotation>)annotationAtPoint:(NSPoint)point {
    return [self annotationWithTag:[self annotationTagAtPoint:point persistingResults:NO]];
}

/**
    Returns the tag of the annotation at the given point in the view.

    This is more involved than it sounds: if multiple point annotations overlap
    near the point, this method cycles through them so that each of them is
    accessible to the user at some point.

    @param persist True to remember the cycleable set of annotations, so that a
        different annotation is returned the next time this method is called
        with the same point. Setting this parameter to false is useful for
        asking “what if?”
 */
- (MLNAnnotationTag)annotationTagAtPoint:(NSPoint)point persistingResults:(BOOL)persist {
    // Look for any annotation near the click. An annotation is “near” if the
    // distance between its center and the click is less than the maximum height
    // or width of an installed annotation image.
    NSRect queryRect = NSInsetRect({ point, NSZeroSize },
                                   -_unionedAnnotationImageSize.width / 2,
                                   -_unionedAnnotationImageSize.height / 2);
    queryRect = NSInsetRect(queryRect, -MLNAnnotationImagePaddingForHitTest,
                            -MLNAnnotationImagePaddingForHitTest);
    std::vector<MLNAnnotationTag> nearbyAnnotations = [self annotationTagsInRect:queryRect];
    std::vector<MLNAnnotationTag> nearbyShapeAnnotations = [self shapeAnnotationTagsInRect:queryRect];

    if (nearbyShapeAnnotations.size()) {
        nearbyAnnotations.insert(nearbyAnnotations.end(), nearbyShapeAnnotations.begin(), nearbyShapeAnnotations.end());
    }

    if (nearbyAnnotations.size()) {
        // Assume that the user is fat-fingering an annotation.
        NSRect hitRect = NSInsetRect({ point, NSZeroSize },
                                     -MLNAnnotationImagePaddingForHitTest,
                                     -MLNAnnotationImagePaddingForHitTest);

        // Filter out any annotation whose image is unselectable or for which
        // hit testing fails.
        auto end = std::remove_if(nearbyAnnotations.begin(), nearbyAnnotations.end(), [&](const MLNAnnotationTag annotationTag) {
            id <MLNAnnotation> annotation = [self annotationWithTag:annotationTag];
            NSAssert(annotation, @"Unknown annotation found nearby click");
            if (!annotation) {
                return true;
            }

            if ([annotation isKindOfClass:[MLNMultiPoint class]])
            {
                if ([self.delegate respondsToSelector:@selector(mapView:shapeAnnotationIsEnabled:)]) {
                    return !!(![self.delegate mapView:self shapeAnnotationIsEnabled:(MLNMultiPoint *)annotation]);
                } else {
                    return false;
                }
            }

            MLNAnnotationImage *annotationImage = [self imageOfAnnotationWithTag:annotationTag];
            if (!annotationImage.selectable) {
                return true;
            }

            // Filter out the annotation if the fattened finger didn’t land on a
            // translucent or opaque pixel in the image.
            NSRect annotationRect = [self frameOfImage:annotationImage.image
                                  centeredAtCoordinate:annotation.coordinate];
            return !!![annotationImage.image hitTestRect:hitRect withImageDestinationRect:annotationRect
                                                 context:nil hints:nil flipped:NO];
        });
        nearbyAnnotations.resize(std::distance(nearbyAnnotations.begin(), end));
    }

    MLNAnnotationTag hitAnnotationTag = MLNAnnotationTagNotFound;
    if (nearbyAnnotations.size()) {
        // The first selection in the cycle should be the one nearest to the
        // tap. Also the annotation tags need to be stable in order to compare them with
        // the remembered tags _annotationsNearbyLastClick.
        CLLocationCoordinate2D currentCoordinate = [self convertPoint:point toCoordinateFromView:self];
        std::sort(nearbyAnnotations.begin(), nearbyAnnotations.end(), [&](const MLNAnnotationTag tagA, const MLNAnnotationTag tagB) {
            CLLocationCoordinate2D coordinateA = [[self annotationWithTag:tagA] coordinate];
            CLLocationCoordinate2D coordinateB = [[self annotationWithTag:tagB] coordinate];
            CLLocationDegrees deltaA = hypot(coordinateA.latitude - currentCoordinate.latitude,
                                             coordinateA.longitude - currentCoordinate.longitude);
            CLLocationDegrees deltaB = hypot(coordinateB.latitude - currentCoordinate.latitude,
                                             coordinateB.longitude - currentCoordinate.longitude);
            return deltaA < deltaB;
        });

        if (nearbyAnnotations == _annotationsNearbyLastClick) {
            // The last time we persisted a set of annotations, we had the same
            // set of annotations as we do now. Cycle through them.
            if (_lastSelectedAnnotationTag == MLNAnnotationTagNotFound
                || _lastSelectedAnnotationTag == nearbyAnnotations.back()) {
                // Either no annotation is selected or the last annotation in
                // the set was selected. Wrap around to the first annotation in
                // the set.
                hitAnnotationTag = nearbyAnnotations.front();
            } else {
                auto result = std::find(nearbyAnnotations.begin(),
                                        nearbyAnnotations.end(),
                                        _lastSelectedAnnotationTag);
                if (result == nearbyAnnotations.end()) {
                    // An annotation from this set hasn’t been selected before.
                    // Select the first (nearest) one.
                    hitAnnotationTag = nearbyAnnotations.front();
                } else {
                    // Step to the next annotation in the set.
                    auto distance = std::distance(nearbyAnnotations.begin(), result);
                    hitAnnotationTag = nearbyAnnotations[distance + 1];
                }
            }
        } else {
            // Remember the nearby annotations for the next time this method is
            // called.
            if (persist) {
                _annotationsNearbyLastClick = nearbyAnnotations;
            }

            // Choose the first nearby annotation.
            if (nearbyAnnotations.size()) {
                hitAnnotationTag = nearbyAnnotations.front();
            }
        }
    }

    return hitAnnotationTag;
}

/// Returns the tags of the annotations coincident with the given rectangle.
- (std::vector<MLNAnnotationTag>)annotationTagsInRect:(NSRect)rect {
    // Cocoa origin is at the lower-left corner.
    return self.renderer->queryPointAnnotations({
        { NSMinX(rect), NSHeight(self.bounds) - NSMaxY(rect) },
        { NSMaxX(rect), NSHeight(self.bounds) - NSMinY(rect) },
    });
}

- (std::vector<MLNAnnotationTag>)shapeAnnotationTagsInRect:(NSRect)rect {
    // Cocoa origin is at the lower-left corner.
    return _rendererFrontend->getRenderer()->queryShapeAnnotations({
        { NSMinX(rect), NSHeight(self.bounds) - NSMaxY(rect) },
        { NSMaxX(rect), NSHeight(self.bounds) - NSMinY(rect) },
    });
}

- (id <MLNAnnotation>)selectedAnnotation {
    if ( ! _annotationContextsByAnnotationTag.count(_selectedAnnotationTag) ||
        _selectedAnnotationTag == MLNAnnotationTagNotFound) {
        return nil;
    }

    MLNAnnotationContext &annotationContext = _annotationContextsByAnnotationTag.at(_selectedAnnotationTag);
    return annotationContext.annotation;
}

- (void)setSelectedAnnotation:(id <MLNAnnotation>)annotation {
    MLNLogDebug(@"Selecting annotation: %@", annotation);
    [self willChangeValueForKey:@"selectedAnnotations"];
    _selectedAnnotationTag = [self annotationTagForAnnotation:annotation];
    if (_selectedAnnotationTag != MLNAnnotationTagNotFound) {
        _lastSelectedAnnotationTag = _selectedAnnotationTag;
    }
    [self didChangeValueForKey:@"selectedAnnotations"];
}

- (NSArray<id <MLNAnnotation>> *)selectedAnnotations {
    id <MLNAnnotation> selectedAnnotation = self.selectedAnnotation;
    return selectedAnnotation ? @[selectedAnnotation] : @[];
}

- (void)setSelectedAnnotations:(NSArray<id <MLNAnnotation>> *)selectedAnnotations {
    MLNLogDebug(@"Selecting: %lu annotations", selectedAnnotations.count);
    if (!selectedAnnotations.count) {
        return;
    }

    id <MLNAnnotation> firstAnnotation = selectedAnnotations[0];
    NSAssert([firstAnnotation conformsToProtocol:@protocol(MLNAnnotation)], @"Annotation does not conform to MLNAnnotation");
    if ([firstAnnotation isKindOfClass:[MLNMultiPoint class]]) {
        return;
    }

    [self selectAnnotation:firstAnnotation];
}

- (BOOL)isMovingAnnotationIntoViewSupportedForAnnotation:(id<MLNAnnotation>)annotation animated:(BOOL)animated {
    // Consider delegating
    return [annotation isKindOfClass:[MLNPointAnnotation class]];
}

- (void)selectAnnotation:(id <MLNAnnotation>)annotation
{
    MLNLogDebug(@"Selecting annotation: %@", annotation);
    [self selectAnnotation:annotation atPoint:NSZeroPoint];
}

- (void)selectAnnotation:(id <MLNAnnotation>)annotation atPoint:(NSPoint)gesturePoint
{
    MLNLogDebug(@"Selecting annotation: %@ atPoint: %@", annotation, NSStringFromPoint(gesturePoint));
    [self selectAnnotation:annotation atPoint:gesturePoint moveIntoView:YES animateSelection:YES];
}

- (void)selectAnnotation:(id <MLNAnnotation>)annotation atPoint:(NSPoint)gesturePoint moveIntoView:(BOOL)moveIntoView animateSelection:(BOOL)animateSelection
{
    MLNLogDebug(@"Selecting annotation: %@ atPoint: %@ moveIntoView: %@ animateSelection: %@", annotation, NSStringFromPoint(gesturePoint), MLNStringFromBOOL(moveIntoView), MLNStringFromBOOL(animateSelection));
    id <MLNAnnotation> selectedAnnotation = self.selectedAnnotation;
    if (annotation == selectedAnnotation) {
        return;
    }

    // Deselect the annotation before reselecting it.
    [self deselectAnnotation:selectedAnnotation];

    // Add the annotation to the map if it hasn’t been added yet.
    MLNAnnotationTag annotationTag = [self annotationTagForAnnotation:annotation];
    if (annotationTag == MLNAnnotationTagNotFound) {
        [self addAnnotation:annotation];
    }

    if (moveIntoView) {
        moveIntoView = [self isMovingAnnotationIntoViewSupportedForAnnotation:annotation animated:animateSelection];
    }

    // The annotation's anchor will bounce to the current click.
    NSRect positioningRect = [self positioningRectForCalloutForAnnotationWithTag:annotationTag];

    // Check for invalid (zero) positioning rect
    if (NSEqualRects(positioningRect, NSZeroRect)) {
        CLLocationCoordinate2D origin = annotation.coordinate;
        positioningRect.origin = [self convertCoordinate:origin toPointToView:self];
    }

    BOOL shouldShowCallout = ([annotation respondsToSelector:@selector(title)]
                              && annotation.title
                              && !self.calloutForSelectedAnnotation.shown
                              && [self.delegate respondsToSelector:@selector(mapView:annotationCanShowCallout:)]
                              && [self.delegate mapView:self annotationCanShowCallout:annotation]);

    if (NSIsEmptyRect(NSIntersectionRect(positioningRect, self.bounds))) {
        if (!moveIntoView && !NSEqualPoints(gesturePoint, NSZeroPoint)) {
            positioningRect = CGRectMake(gesturePoint.x, gesturePoint.y, positioningRect.size.width, positioningRect.size.height);
        }
    }
    // Onscreen or partially on-screen
    else if (!shouldShowCallout) {
        moveIntoView = NO;
    }

    self.selectedAnnotation = annotation;

    // For the callout to be shown, the annotation must have a title, its
    // callout must not already be shown, and the annotation must be able to
    // show a callout according to the delegate.
    if (shouldShowCallout) {
        NSPopover *callout = [self calloutForAnnotation:annotation];

        // Hang the callout off the right edge of the annotation image’s
        // alignment rect, or off the left edge in a right-to-left UI.
        callout.delegate = self;
        self.calloutForSelectedAnnotation = callout;

        NSRectEdge edge = (self.userInterfaceLayoutDirection == NSUserInterfaceLayoutDirectionRightToLeft
                           ? NSMinXEdge
                           : NSMaxXEdge);

        // The following will do nothing if the positioning rect is not on-screen. See
        // ``MLNMapView/updateAnnotationCallouts`` for presenting the callout when the selected
        // annotation comes back on-screen.
        [callout showRelativeToRect:positioningRect ofView:self preferredEdge:edge];
    }

    if (moveIntoView)
    {
        moveIntoView = NO;

        NSRect (^edgeInsetsInsetRect)(NSRect, NSEdgeInsets) = ^(NSRect rect, NSEdgeInsets insets) {
            return NSMakeRect(rect.origin.x + insets.left,
                              rect.origin.y + insets.bottom,
                              rect.size.width - insets.left - insets.right,
                              rect.size.height - insets.top - insets.bottom);
        };

        // Add padding around the positioning rect (in essence an inset from the edge of the viewport
        NSRect expandedPositioningRect = positioningRect;

        if (shouldShowCallout) {
            // If we have a callout, expand this rect to include a buffer
            expandedPositioningRect = edgeInsetsInsetRect(positioningRect, MLNMapViewOffscreenAnnotationPadding);
        }

        // Used for callout positioning, and moving offscreen annotations onscreen.
        CGRect constrainedRect = edgeInsetsInsetRect(self.bounds, self.contentInsets);
        CGRect bounds          = constrainedRect;

        // Any one of these cases should trigger a move onscreen
        CGFloat minX = CGRectGetMinX(expandedPositioningRect);

        if (minX < CGRectGetMinX(bounds)) {
            constrainedRect.origin.x = minX;
            moveIntoView = YES;
        }
        else {
            CGFloat maxX = CGRectGetMaxX(expandedPositioningRect);

            if (maxX > CGRectGetMaxX(bounds)) {
                constrainedRect.origin.x = maxX - CGRectGetWidth(constrainedRect);
                moveIntoView = YES;
            }
        }

        CGFloat minY = CGRectGetMinY(expandedPositioningRect);

        if (minY < CGRectGetMinY(bounds)) {
            constrainedRect.origin.y = minY;
            moveIntoView = YES;
        }
        else {
            CGFloat maxY = CGRectGetMaxY(expandedPositioningRect);

            if (maxY > CGRectGetMaxY(bounds)) {
                constrainedRect.origin.y = maxY - CGRectGetHeight(constrainedRect);
                moveIntoView = YES;
            }
        }

        if (moveIntoView)
        {
            CGPoint center = CGPointMake(CGRectGetMidX(constrainedRect), CGRectGetMidY(constrainedRect));
            CLLocationCoordinate2D centerCoord = [self convertPoint:center toCoordinateFromView:self];
            [self setCenterCoordinate:centerCoord animated:animateSelection];
        }
    }
}

- (void)showAnnotations:(NSArray<id <MLNAnnotation>> *)annotations animated:(BOOL)animated {
    MLNLogDebug(@"Showing: %lu annotations animated: %@", annotations.count, MLNStringFromBOOL(animated));
    CGFloat maximumPadding = 100;
    CGFloat yPadding = (NSHeight(self.bounds) / 5 <= maximumPadding) ? (NSHeight(self.bounds) / 5) : maximumPadding;
    CGFloat xPadding = (NSWidth(self.bounds) / 5 <= maximumPadding) ? (NSWidth(self.bounds) / 5) : maximumPadding;

    NSEdgeInsets edgeInsets = NSEdgeInsetsMake(yPadding, xPadding, yPadding, xPadding);

    [self showAnnotations:annotations edgePadding:edgeInsets animated:animated];
}

- (void)showAnnotations:(NSArray<id <MLNAnnotation>> *)annotations edgePadding:(NSEdgeInsets)insets animated:(BOOL)animated {
    [self showAnnotations:annotations edgePadding:insets animated:animated completionHandler:nil];
}

- (void)showAnnotations:(NSArray<id <MLNAnnotation>> *)annotations edgePadding:(NSEdgeInsets)insets animated:(BOOL)animated completionHandler:(nullable void (^)(void))completion {
    if (!annotations.count) {
        if (completion) {
            completion();
        }
        return;
    }

    mbgl::LatLngBounds bounds = mbgl::LatLngBounds::empty();

    for (id <MLNAnnotation> annotation in annotations) {
        if ([annotation conformsToProtocol:@protocol(MLNOverlay)]) {
            bounds.extend(MLNLatLngBoundsFromCoordinateBounds(((id <MLNOverlay>)annotation).overlayBounds));
        } else {
            bounds.extend(MLNLatLngFromLocationCoordinate2D(annotation.coordinate));
        }
    }

    [self setVisibleCoordinateBounds:MLNCoordinateBoundsFromLatLngBounds(bounds)
                         edgePadding:insets
                            animated:animated
                   completionHandler:completion];
}

/// Returns a popover detailing the annotation.
- (NSPopover *)calloutForAnnotation:(id <MLNAnnotation>)annotation {
    NSPopover *callout = [[NSPopover alloc] init];
    callout.behavior = NSPopoverBehaviorTransient;

    NSViewController *viewController;
    if ([self.delegate respondsToSelector:@selector(mapView:calloutViewControllerForAnnotation:)]) {
        NSViewController *viewControllerFromDelegate = [self.delegate mapView:self
                                           calloutViewControllerForAnnotation:annotation];
        if (viewControllerFromDelegate) {
            viewController = viewControllerFromDelegate;
        }
    }
    if (!viewController) {
        viewController = self.calloutViewController;
    }
    NSAssert(viewController, @"Unable to load MLNAnnotationCallout view controller");
    // The popover’s view controller can bind to KVO-compliant key paths of the
    // annotation.
    viewController.representedObject = annotation;
    callout.contentViewController = viewController;

    return callout;
}

- (NSViewController *)calloutViewController {
    // Lazily load a default view controller.
    if (!_calloutViewController) {
        _calloutViewController = [[NSViewController alloc] initWithNibName:@"MLNAnnotationCallout"
                                                                    bundle:[NSBundle mgl_frameworkBundle]];
    }
    return _calloutViewController;
}

/// Returns the rectangle that represents the annotation image of the annotation
/// with the given tag. This rectangle is fitted to the image’s alignment rect
/// and is appropriate for positioning a popover.
- (NSRect)positioningRectForCalloutForAnnotationWithTag:(MLNAnnotationTag)annotationTag {
    id <MLNAnnotation> annotation = [self annotationWithTag:annotationTag];
    if (!annotation) {
        return NSZeroRect;
    }
    if ([annotation isKindOfClass:[MLNMultiPoint class]]) {
        CLLocationCoordinate2D origin = annotation.coordinate;
        CGPoint originPoint = [self convertCoordinate:origin toPointToView:self];
        return CGRectMake(originPoint.x, originPoint.y, MLNAnnotationImagePaddingForHitTest, MLNAnnotationImagePaddingForHitTest);

    }

    NSImage *image = [self imageOfAnnotationWithTag:annotationTag].image;
    if (!image) {
        image = [self dequeueReusableAnnotationImageWithIdentifier:MLNDefaultStyleMarkerSymbolName].image;
    }
    if (!image) {
        return NSZeroRect;
    }

    NSRect positioningRect = [self frameOfImage:image centeredAtCoordinate:annotation.coordinate];
    positioningRect = NSOffsetRect(image.alignmentRect, positioningRect.origin.x, positioningRect.origin.y);
    return NSInsetRect(positioningRect, -MLNAnnotationImagePaddingForCallout,
                       -MLNAnnotationImagePaddingForCallout);
}

/// Returns the rectangle relative to the viewport that represents the given
/// image centered at the given coordinate.
- (NSRect)frameOfImage:(NSImage *)image centeredAtCoordinate:(CLLocationCoordinate2D)coordinate {
    NSPoint calloutAnchorPoint = [self convertCoordinate:coordinate toPointToView:self];
    return NSInsetRect({ calloutAnchorPoint, NSZeroSize }, -image.size.width / 2, -image.size.height / 2);
}

/// Returns the annotation image assigned to the annotation with the given tag.
- (MLNAnnotationImage *)imageOfAnnotationWithTag:(MLNAnnotationTag)annotationTag {
    if (annotationTag == MLNAnnotationTagNotFound
        || _annotationContextsByAnnotationTag.count(annotationTag) == 0) {
        return nil;
    }

    NSString *customSymbol = _annotationContextsByAnnotationTag.at(annotationTag).imageReuseIdentifier;
    NSString *symbolName = customSymbol.length ? customSymbol : MLNDefaultStyleMarkerSymbolName;

    return [self dequeueReusableAnnotationImageWithIdentifier:symbolName];
}

- (void)deselectAnnotation:(id <MLNAnnotation>)annotation {
    if (!annotation || self.selectedAnnotation != annotation) {
        return;
    }

    // Close the callout popover gracefully.
    NSPopover *callout = self.calloutForSelectedAnnotation;
    [callout performClose:self];

    self.selectedAnnotation = nil;
}

/// Move the annotation callout to point to the selected annotation at its
/// current position.
- (void)updateAnnotationCallouts {
    NSPopover *callout = self.calloutForSelectedAnnotation;
    if (callout) {
        NSRect rect = [self positioningRectForCalloutForAnnotationWithTag:_selectedAnnotationTag];

        NSAssert(!NSEqualRects(rect, NSZeroRect), @"Positioning rect should be non-zero");

        if (!NSIsEmptyRect(NSIntersectionRect(rect, self.bounds))) {

            // It's possible that the current callout hasn't been presented (since the original
            // positioningRect was offscreen). We can check that the callout has a valid window
            // This results in the callout being presented just as the annotation comes on screen
            // which matches MapKit, but (currently) not iOS.
            if (!callout.contentViewController.view.window) {
                NSRectEdge edge = (self.userInterfaceLayoutDirection == NSUserInterfaceLayoutDirectionRightToLeft
                                   ? NSMinXEdge
                                   : NSMaxXEdge);
                // Re-present the callout
                [callout showRelativeToRect:rect ofView:self preferredEdge:edge];
            }
            else {
                callout.positioningRect = rect;
            }
        }
    }
}

// MARK: MLNMultiPointDelegate methods

- (double)alphaForShapeAnnotation:(MLNShape *)annotation {
    if (_delegateHasAlphasForShapeAnnotations) {
        return [self.delegate mapView:self alphaForShapeAnnotation:annotation];
    }
    return 1.0;
}

- (mbgl::Color)strokeColorForShapeAnnotation:(MLNShape *)annotation {
    NSColor *color = (_delegateHasStrokeColorsForShapeAnnotations
                      ? [self.delegate mapView:self strokeColorForShapeAnnotation:annotation]
                      : [NSColor selectedMenuItemColor]);
    return color.mgl_color;
}

- (mbgl::Color)fillColorForPolygonAnnotation:(MLNPolygon *)annotation {
    NSColor *color = (_delegateHasFillColorsForShapeAnnotations
                      ? [self.delegate mapView:self fillColorForPolygonAnnotation:annotation]
                      : [NSColor selectedMenuItemColor]);
    return color.mgl_color;
}

- (CGFloat)lineWidthForPolylineAnnotation:(MLNPolyline *)annotation {
    if (_delegateHasLineWidthsForShapeAnnotations) {
        return [self.delegate mapView:self lineWidthForPolylineAnnotation:(MLNPolyline *)annotation];
    }
    return 3.0;
}

// MARK: MLNPopoverDelegate methods

- (void)popoverDidShow:(__unused NSNotification *)notification {
    id <MLNAnnotation> annotation = self.selectedAnnotation;
    if (annotation && [self.delegate respondsToSelector:@selector(mapView:didSelectAnnotation:)]) {
        [self.delegate mapView:self didSelectAnnotation:annotation];
    }
}

- (void)popoverDidClose:(__unused NSNotification *)notification {
    // Deselect the closed popover, in case the popover was closed due to user
    // action.
    id <MLNAnnotation> annotation = self.calloutForSelectedAnnotation.contentViewController.representedObject;
    self.calloutForSelectedAnnotation = nil;
    self.selectedAnnotation = nil;

    if ([self.delegate respondsToSelector:@selector(mapView:didDeselectAnnotation:)]) {
        [self.delegate mapView:self didDeselectAnnotation:annotation];
    }
}

// MARK: Overlays

- (nonnull NSArray<id <MLNOverlay>> *)overlays
{
    if (self.annotations == nil) { return @[]; }

    NSMutableArray<id <MLNOverlay>> *mutableOverlays = [NSMutableArray array];

    [self.annotations enumerateObjectsUsingBlock:^(id<MLNAnnotation>  _Nonnull annotation, NSUInteger idx, BOOL * _Nonnull stop) {
        if ([annotation conformsToProtocol:@protocol(MLNOverlay)])
        {
            [mutableOverlays addObject:(id<MLNOverlay>)annotation];
        }
    }];

    return [NSArray arrayWithArray:mutableOverlays];
}

- (void)addOverlay:(id <MLNOverlay>)overlay {
    MLNLogDebug(@"Adding overlay: %@", overlay);
    [self addOverlays:@[overlay]];
}

- (void)addOverlays:(NSArray<id <MLNOverlay>> *)overlays
{
    MLNLogDebug(@"Adding: %lu overlays", overlays.count);
#if DEBUG
    for (id <MLNOverlay> overlay in overlays) {
        NSAssert([overlay conformsToProtocol:@protocol(MLNOverlay)], @"Overlay does not conform to MLNOverlay");
    }
#endif
    [self addAnnotations:overlays];
}

- (void)removeOverlay:(id <MLNOverlay>)overlay {
    MLNLogDebug(@"Removing overlay: %@", overlay);
    [self removeOverlays:@[overlay]];
}

- (void)removeOverlays:(NSArray<id <MLNOverlay>> *)overlays {
    MLNLogDebug(@"Removing: %lu overlays", overlays.count);
#if DEBUG
    for (id <MLNOverlay> overlay in overlays) {
        NSAssert([overlay conformsToProtocol:@protocol(MLNOverlay)], @"Overlay does not conform to MLNOverlay");
    }
#endif
    [self removeAnnotations:overlays];
}

// MARK: Tooltips and cursors

- (void)updateAnnotationTrackingAreas {
    if (_wantsToolTipRects) {
        [self removeAllToolTips];
        std::vector<MLNAnnotationTag> annotationTags = [self annotationTagsInRect:self.bounds];
        for (MLNAnnotationTag annotationTag : annotationTags) {
            MLNAnnotationImage *annotationImage = [self imageOfAnnotationWithTag:annotationTag];
            id <MLNAnnotation> annotation = [self annotationWithTag:annotationTag];
            if ([annotation respondsToSelector:@selector(toolTip)] && annotation.toolTip.length) {
                // Add a tooltip tracking area over the annotation image’s
                // frame, accounting for the image’s alignment rect.
                NSImage *image = annotationImage.image;
                NSRect annotationRect = [self frameOfImage:image
                                      centeredAtCoordinate:annotation.coordinate];
                annotationRect = NSOffsetRect(image.alignmentRect, annotationRect.origin.x, annotationRect.origin.y);
                if (!NSIsEmptyRect(annotationRect)) {
                    [self addToolTipRect:annotationRect owner:self userData:(void *)(NSUInteger)annotationTag];
                }
            }
            // Opt into potentially expensive cursor tracking areas.
            if (annotationImage.cursor) {
                _wantsCursorRects = YES;
            }
        }
    }

    // Blow away any cursor tracking areas and rebuild them. That’s the
    // potentially expensive part.
    if (_wantsCursorRects) {
        [self.window invalidateCursorRectsForView:self];
    }
}

- (NSString *)view:(__unused NSView *)view stringForToolTip:(__unused NSToolTipTag)tag point:(__unused NSPoint)point userData:(void *)data {
    NSAssert((NSUInteger)data < MLNAnnotationTagNotFound, @"Invalid annotation tag in tooltip rect user data.");
    MLNAnnotationTag annotationTag = (MLNAnnotationTag)MIN((NSUInteger)data, MLNAnnotationTagNotFound);
    id <MLNAnnotation> annotation = [self annotationWithTag:annotationTag];
    return annotation.toolTip;
}

- (void)resetCursorRects {
    // Drag to pan has a grabbing hand cursor.
    if ([self isPanningWithGesture]) {
        [self addCursorRect:self.bounds cursor:[NSCursor closedHandCursor]];
        return;
    }

    // The rest of this method can be expensive, so bail if no annotations have
    // ever had custom cursors.
    if (!_wantsCursorRects) {
        return;
    }

    std::vector<MLNAnnotationTag> annotationTags = [self annotationTagsInRect:self.bounds];
    for (MLNAnnotationTag annotationTag : annotationTags) {
        id <MLNAnnotation> annotation = [self annotationWithTag:annotationTag];
        MLNAnnotationImage *annotationImage = [self imageOfAnnotationWithTag:annotationTag];
        if (annotationImage.cursor) {
            // Add a cursor tracking area over the annotation image, respecting
            // the image’s alignment rect.
            NSImage *image = annotationImage.image;
            NSRect annotationRect = [self frameOfImage:image
                                  centeredAtCoordinate:annotation.coordinate];
            annotationRect = NSOffsetRect(image.alignmentRect, annotationRect.origin.x, annotationRect.origin.y);
            [self addCursorRect:annotationRect cursor:annotationImage.cursor];
        }
    }
}

// MARK: Data

- (NSArray<id <MLNFeature>> *)visibleFeaturesAtPoint:(NSPoint)point {
    MLNLogDebug(@"Querying visibleFeaturesAtPoint: %@", NSStringFromPoint(point));
    return [self visibleFeaturesAtPoint:point inStyleLayersWithIdentifiers:nil];
}

- (NSArray<id <MLNFeature>> *)visibleFeaturesAtPoint:(CGPoint)point inStyleLayersWithIdentifiers:(NSSet<NSString *> *)styleLayerIdentifiers {
    MLNLogDebug(@"Querying visibleFeaturesAtPoint: %@ inStyleLayersWithIdentifiers: %@", NSStringFromPoint(point), styleLayerIdentifiers);
    return [self visibleFeaturesAtPoint:point inStyleLayersWithIdentifiers:styleLayerIdentifiers predicate:nil];
}

- (NSArray<id <MLNFeature>> *)visibleFeaturesAtPoint:(NSPoint)point inStyleLayersWithIdentifiers:(NSSet<NSString *> *)styleLayerIdentifiers predicate:(NSPredicate *)predicate {
    MLNLogDebug(@"Querying visibleFeaturesAtPoint: %@ inStyleLayersWithIdentifiers: %@ predicate: %@", NSStringFromPoint(point), styleLayerIdentifiers, predicate);
    // Cocoa origin is at the lower-left corner.
    mbgl::ScreenCoordinate screenCoordinate = { point.x, NSHeight(self.bounds) - point.y };

    std::optional<std::vector<std::string>> optionalLayerIDs;
    if (styleLayerIdentifiers) {
        __block std::vector<std::string> layerIDs;
        layerIDs.reserve(styleLayerIdentifiers.count);
        [styleLayerIdentifiers enumerateObjectsUsingBlock:^(NSString * _Nonnull identifier, BOOL * _Nonnull stop) {
            layerIDs.push_back(identifier.UTF8String);
        }];
        optionalLayerIDs = layerIDs;
    }

    std::optional<mbgl::style::Filter> optionalFilter;
    if (predicate) {
        optionalFilter = predicate.mgl_filter;
    }

    std::vector<mbgl::Feature> features = _rendererFrontend->getRenderer()->queryRenderedFeatures(screenCoordinate, { optionalLayerIDs, optionalFilter });
    return MLNFeaturesFromMBGLFeatures(features);
}

- (NSArray<id <MLNFeature>> *)visibleFeaturesInRect:(NSRect)rect {
    MLNLogDebug(@"Querying visibleFeaturesInRect: %@", NSStringFromRect(rect));
    return [self visibleFeaturesInRect:rect inStyleLayersWithIdentifiers:nil];
}

- (NSArray<id <MLNFeature>> *)visibleFeaturesInRect:(CGRect)rect inStyleLayersWithIdentifiers:(NSSet<NSString *> *)styleLayerIdentifiers {
    MLNLogDebug(@"Querying visibleFeaturesInRect: %@ inStyleLayersWithIdentifiers: %@", NSStringFromRect(rect), styleLayerIdentifiers);
    return [self visibleFeaturesInRect:rect inStyleLayersWithIdentifiers:styleLayerIdentifiers predicate:nil];
}

- (NSArray<id <MLNFeature>> *)visibleFeaturesInRect:(NSRect)rect inStyleLayersWithIdentifiers:(NSSet<NSString *> *)styleLayerIdentifiers predicate:(NSPredicate *)predicate {
    MLNLogDebug(@"Querying visibleFeaturesInRect: %@ inStyleLayersWithIdentifiers: %@ predicate: %@", NSStringFromRect(rect), styleLayerIdentifiers, predicate);
    // Cocoa origin is at the lower-left corner.
    mbgl::ScreenBox screenBox = {
        { NSMinX(rect), NSHeight(self.bounds) - NSMaxY(rect) },
        { NSMaxX(rect), NSHeight(self.bounds) - NSMinY(rect) },
    };

    std::optional<std::vector<std::string>> optionalLayerIDs;
    if (styleLayerIdentifiers) {
        __block std::vector<std::string> layerIDs;
        layerIDs.reserve(styleLayerIdentifiers.count);
        [styleLayerIdentifiers enumerateObjectsUsingBlock:^(NSString * _Nonnull identifier, BOOL * _Nonnull stop) {
            layerIDs.push_back(identifier.UTF8String);
        }];
        optionalLayerIDs = layerIDs;
    }

    std::optional<mbgl::style::Filter> optionalFilter;
    if (predicate) {
        optionalFilter = predicate.mgl_filter;
    }

    std::vector<mbgl::Feature> features = _rendererFrontend->getRenderer()->queryRenderedFeatures(screenBox, { optionalLayerIDs, optionalFilter });
    return MLNFeaturesFromMBGLFeatures(features);
}

// MARK: User interface validation

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem {
    return NO;
}

// MARK: Interface Builder methods

- (void)prepareForInterfaceBuilder {
    [super prepareForInterfaceBuilder];

    // Color the background a glorious Mapbox teal.
    self.layer.borderColor = [NSColor colorWithRed:59/255.
                                             green:178/255.
                                              blue:208/255.
                                             alpha:0.8].CGColor;
    self.layer.borderWidth = 2;
    self.layer.backgroundColor = [NSColor colorWithRed:59/255.
                                                 green:178/255.
                                                  blue:208/255.
                                                 alpha:0.6].CGColor;

    // Place a playful marker right smack dab in the middle.
    self.layer.contents = MLNDefaultMarkerImage();
    self.layer.contentsGravity = kCAGravityCenter;
    self.layer.contentsScale = [NSScreen mainScreen].backingScaleFactor;
}

// MARK: Geometric methods

- (NSPoint)convertCoordinate:(CLLocationCoordinate2D)coordinate toPointToView:(nullable NSView *)view {
    if (!CLLocationCoordinate2DIsValid(coordinate)) {
        return NSMakePoint(NAN, NAN);
    }
    return [self convertLatLng:MLNLatLngFromLocationCoordinate2D(coordinate) toPointToView:view];
}

/// Converts a geographic coordinate to a point in the view’s coordinate system.
- (NSPoint)convertLatLng:(mbgl::LatLng)latLng toPointToView:(nullable NSView *)view {
    mbgl::ScreenCoordinate pixel = _mbglMap->pixelForLatLng(latLng);
    // Cocoa origin is at the lower-left corner.
    pixel.y = NSHeight(self.bounds) - pixel.y;
    return [self convertPoint:NSMakePoint(pixel.x, pixel.y) toView:view];
}

- (CLLocationCoordinate2D)convertPoint:(NSPoint)point toCoordinateFromView:(nullable NSView *)view {
    return MLNLocationCoordinate2DFromLatLng([self convertPoint:point toLatLngFromView:view]);
}

/// Converts a point in the view’s coordinate system to a geographic coordinate.
- (mbgl::LatLng)convertPoint:(NSPoint)point toLatLngFromView:(nullable NSView *)view {
    NSPoint convertedPoint = [self convertPoint:point fromView:view];
    return _mbglMap->latLngForPixel({
        convertedPoint.x,
        // mbgl origin is at the top-left corner.
        NSHeight(self.bounds) - convertedPoint.y,
    }).wrapped();
}

- (NSRect)convertCoordinateBounds:(MLNCoordinateBounds)bounds toRectToView:(nullable NSView *)view {
    return [self convertLatLngBounds:MLNLatLngBoundsFromCoordinateBounds(bounds) toRectToView:view];
}

/// Converts a geographic bounding box to a rectangle in the view’s coordinate
/// system.
- (NSRect)convertLatLngBounds:(mbgl::LatLngBounds)bounds toRectToView:(nullable NSView *)view {
    auto northwest = bounds.northwest();
    auto northeast = bounds.northeast();
    auto southwest = bounds.southwest();
    auto southeast = bounds.southeast();

    auto center = [self convertPoint:{ NSMidX(view.bounds), NSMidY(view.bounds) } toLatLngFromView:view];

    // Extend bounds to account for the antimeridian
    northwest.unwrapForShortestPath(center);
    northeast.unwrapForShortestPath(center);
    southwest.unwrapForShortestPath(center);
    southeast.unwrapForShortestPath(center);

    auto correctedLatLngBounds = mbgl::LatLngBounds::empty();
    correctedLatLngBounds.extend(northwest);
    correctedLatLngBounds.extend(northeast);
    correctedLatLngBounds.extend(southwest);
    correctedLatLngBounds.extend(southeast);

    NSRect rect = { [self convertLatLng:correctedLatLngBounds.southwest() toPointToView:view], CGSizeZero };
    rect = MLNExtendRect(rect, [self convertLatLng:correctedLatLngBounds.northeast() toPointToView:view]);
    return rect;
}

- (MLNCoordinateBounds)convertRect:(NSRect)rect toCoordinateBoundsFromView:(nullable NSView *)view {
    return MLNCoordinateBoundsFromLatLngBounds([self convertRect:rect toLatLngBoundsFromView:view]);
}

/// Converts a rectangle in the given view’s coordinate system to a geographic
/// bounding box.
- (mbgl::LatLngBounds)convertRect:(NSRect)rect toLatLngBoundsFromView:(nullable NSView *)view {
    auto bounds = mbgl::LatLngBounds::empty();
    auto bottomLeft = [self convertPoint:{ NSMinX(rect), NSMinY(rect) } toLatLngFromView:view];
    auto bottomRight = [self convertPoint:{ NSMaxX(rect), NSMinY(rect) } toLatLngFromView:view];
    auto topRight = [self convertPoint:{ NSMaxX(rect), NSMaxY(rect) } toLatLngFromView:view];
    auto topLeft = [self convertPoint:{ NSMinX(rect), NSMaxY(rect) } toLatLngFromView:view];

    // If the bounds straddles the antimeridian, unwrap it so that one side
    // extends beyond ±180° longitude.
    auto center = [self convertPoint:{ NSMidX(rect), NSMidY(rect) } toLatLngFromView:view];
    bottomLeft.unwrapForShortestPath(center);
    bottomRight.unwrapForShortestPath(center);
    topRight.unwrapForShortestPath(center);
    topLeft.unwrapForShortestPath(center);

    bounds.extend(bottomLeft);
    bounds.extend(bottomRight);
    bounds.extend(topRight);
    bounds.extend(topLeft);

    return bounds;
}

- (CLLocationDistance)metersPerPointAtLatitude:(CLLocationDegrees)latitude {
    return mbgl::Projection::getMetersPerPixelAtLatitude(latitude, self.zoomLevel);
}

// MARK: Debugging

- (MLNMapDebugMaskOptions)debugMask {
    mbgl::MapDebugOptions options = _mbglMap->getDebug();
    MLNMapDebugMaskOptions mask = 0;
    if (options & mbgl::MapDebugOptions::TileBorders) {
        mask |= MLNMapDebugTileBoundariesMask;
    }
    if (options & mbgl::MapDebugOptions::ParseStatus) {
        mask |= MLNMapDebugTileInfoMask;
    }
    if (options & mbgl::MapDebugOptions::Timestamps) {
        mask |= MLNMapDebugTimestampsMask;
    }
    if (options & mbgl::MapDebugOptions::Collision) {
        mask |= MLNMapDebugCollisionBoxesMask;
    }
    if (options & mbgl::MapDebugOptions::Overdraw) {
        mask |= MLNMapDebugOverdrawVisualizationMask;
    }
    if (options & mbgl::MapDebugOptions::StencilClip) {
        mask |= MLNMapDebugStencilBufferMask;
    }
    if (options & mbgl::MapDebugOptions::DepthBuffer) {
        mask |= MLNMapDebugDepthBufferMask;
    }
    return mask;
}

- (void)setDebugMask:(MLNMapDebugMaskOptions)debugMask {
    mbgl::MapDebugOptions options = mbgl::MapDebugOptions::NoDebug;
    if (debugMask & MLNMapDebugTileBoundariesMask) {
        options |= mbgl::MapDebugOptions::TileBorders;
    }
    if (debugMask & MLNMapDebugTileInfoMask) {
        options |= mbgl::MapDebugOptions::ParseStatus;
    }
    if (debugMask & MLNMapDebugTimestampsMask) {
        options |= mbgl::MapDebugOptions::Timestamps;
    }
    if (debugMask & MLNMapDebugCollisionBoxesMask) {
        options |= mbgl::MapDebugOptions::Collision;
    }
    if (debugMask & MLNMapDebugOverdrawVisualizationMask) {
        options |= mbgl::MapDebugOptions::Overdraw;
    }
    if (debugMask & MLNMapDebugStencilBufferMask) {
        options |= mbgl::MapDebugOptions::StencilClip;
    }
    if (debugMask & MLNMapDebugDepthBufferMask) {
        options |= mbgl::MapDebugOptions::DepthBuffer;
    }
    _mbglMap->setDebug(options);
}

- (NSArray<NSString*>*)getActionJournalLog
{
    const auto& actionJournal = _mbglMap->getActionJournal();
    if (!actionJournal) {
        return nil;
    }

    const auto& log = actionJournal->getLog();
    NSMutableArray<NSString*>* objcLog = [NSMutableArray new];

    for (const auto& event : log) {
        [objcLog addObject:[NSString stringWithUTF8String:event.c_str()]];
    }

    return objcLog;
}

- (void)clearActionJournalLog
{
    const auto& actionJournal = _mbglMap->getActionJournal();
    if (!actionJournal) {
        return;
    }

    actionJournal->clearLog();
}

- (MLNBackendResource *)backendResource {
    return _mbglView->getObject();
}

@end
