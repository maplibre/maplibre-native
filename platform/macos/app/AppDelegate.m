#import "AppDelegate.h"

#import "MapDocument.h"

NSString * const MGLApiKeyDefaultsKey = @"MGLApiKey";
NSString * const MGLLastMapCameraDefaultsKey = @"MGLLastMapCamera";
NSString * const MGLLastMapStyleURLDefaultsKey = @"MGLLastMapStyleURL";
NSString * const MGLLastMapDebugMaskDefaultsKey = @"MGLLastMapDebugMask";

/**
 Some convenience methods to make offline pack properties easier to bind to.
 */
@implementation MGLOfflinePack (Additions)

+ (NSSet *)keyPathsForValuesAffectingStateImage {
    return [NSSet setWithObjects:@"state", nil];
}

- (NSImage *)stateImage {
    switch (self.state) {
        case MGLOfflinePackStateComplete:
            return [NSImage imageNamed:@"NSMenuOnStateTemplate"];

        case MGLOfflinePackStateActive:
            return [NSImage imageNamed:@"NSFollowLinkFreestandingTemplate"];

        default:
            return nil;
    }
}

+ (NSSet<NSString *> *)keyPathsForValuesAffectingCountOfResourcesCompleted {
    return [NSSet setWithObjects:@"progress", nil];
}

- (uint64_t)countOfResourcesCompleted {
    return self.progress.countOfResourcesCompleted;
}

+ (NSSet<NSString *> *)keyPathsForValuesAffectingCountOfResourcesExpected {
    return [NSSet setWithObjects:@"progress", nil];
}

- (uint64_t)countOfResourcesExpected {
    return self.progress.countOfResourcesExpected;
}

+ (NSSet<NSString *> *)keyPathsForValuesAffectingCountOfBytesCompleted {
    return [NSSet setWithObjects:@"progress", nil];
}

- (uint64_t)countOfBytesCompleted {
    return self.progress.countOfBytesCompleted;
}

+ (NSSet<NSString *> *)keyPathsForValuesAffectingCountOfTilesCompleted {
    return [NSSet setWithObjects:@"progress", nil];
}

- (uint64_t)countOfTilesCompleted {
    return self.progress.countOfTilesCompleted;
}

+ (NSSet<NSString *> *)keyPathsForValuesAffectingCountOfTileBytesCompleted {
    return [NSSet setWithObjects:@"progress", nil];
}

- (uint64_t)countOfTileBytesCompleted {
    return self.progress.countOfTileBytesCompleted;
}

@end

@interface AppDelegate () <NSWindowDelegate>

@property (weak) IBOutlet NSArrayController *offlinePacksArrayController;
@property (weak) IBOutlet NSPanel *offlinePacksPanel;

@end

@implementation AppDelegate

#pragma mark Lifecycle

+ (void)load {
    [MGLSettings useWellKnownTileServer:MGLMapTiler];
    // Set access token, unless MGLSettings already read it in from Info.plist.
    if (![MGLSettings apiKey]) {
        NSString *apiKey = [NSProcessInfo processInfo].environment[@"MGL_API_KEY"];
        if (apiKey) {
            // Store to preferences so that we can launch the app later on without having to specify
            // token.
            [[NSUserDefaults standardUserDefaults] setObject:apiKey forKey:MGLApiKeyDefaultsKey];
        } else {
            // Try to retrieve from preferences, maybe we've stored them there previously and can reuse
            // the token.
            apiKey = [[NSUserDefaults standardUserDefaults] stringForKey:MGLApiKeyDefaultsKey];
        }
        [MGLSettings setApiKey:apiKey];
    }
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification {
    [[NSAppleEventManager sharedAppleEventManager] setEventHandler:self
                                                       andSelector:@selector(handleGetURLEvent:withReplyEvent:)
                                                     forEventClass:kInternetEventClass
                                                        andEventID:kAEGetURL];

    if (![[NSUserDefaults standardUserDefaults] boolForKey:@"NSQuitAlwaysKeepsWindows"]) {
        NSData *cameraData = [[NSUserDefaults standardUserDefaults] objectForKey:MGLLastMapCameraDefaultsKey];
        if (cameraData) {
            NSKeyedUnarchiver *coder = [[NSKeyedUnarchiver alloc] initForReadingWithData:cameraData];
            self.pendingZoomLevel = -1;
            self.pendingCamera = [[MGLMapCamera alloc] initWithCoder:coder];
        }
        NSString *styleURLString = [[NSUserDefaults standardUserDefaults] objectForKey:MGLLastMapStyleURLDefaultsKey];
        if (styleURLString) {
            self.pendingStyleURL = [NSURL URLWithString:styleURLString];
        }
        self.pendingDebugMask = [[NSUserDefaults standardUserDefaults] integerForKey:MGLLastMapDebugMaskDefaultsKey];
    }
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Set access token, unless MGLSettings already read it in from Info.plist.
    if (![MGLSettings apiKey]) {
        NSAlert *alert = [[NSAlert alloc] init];
        alert.messageText = @"API key required";
        alert.informativeText = @"To load tiles and styles, enter your API key in Preferences.";
        [alert addButtonWithTitle:@"Open Preferences"];
        [alert runModal];
        [self showPreferences:nil];
    }

    [self.offlinePacksArrayController bind:@"content" toObject:[MGLOfflineStorage sharedOfflineStorage] withKeyPath:@"packs" options:nil];
}

- (void)applicationWillTerminate:(NSNotification *)notification {
    [[NSNotificationCenter defaultCenter] removeObserver:self name:nil object:nil];

    if (![[NSUserDefaults standardUserDefaults] boolForKey:@"NSQuitAlwaysKeepsWindows"]) {
        NSDocument *currentDocument = [NSDocumentController sharedDocumentController].currentDocument;
        if ([currentDocument isKindOfClass:[MapDocument class]]) {
            MGLMapView *mapView = [(MapDocument *)currentDocument mapView];
            NSMutableData *cameraData = [NSMutableData data];
            NSKeyedArchiver *coder = [[NSKeyedArchiver alloc] initForWritingWithMutableData:cameraData];
            [mapView.camera encodeWithCoder:coder];
            [coder finishEncoding];
            [[NSUserDefaults standardUserDefaults] setObject:cameraData forKey:MGLLastMapCameraDefaultsKey];
            [[NSUserDefaults standardUserDefaults] setObject:mapView.styleURL.absoluteString forKey:MGLLastMapStyleURLDefaultsKey];
            [[NSUserDefaults standardUserDefaults] setInteger:mapView.debugMask forKey:MGLLastMapDebugMaskDefaultsKey];
        }
    }

    [self.offlinePacksArrayController unbind:@"content"];
}

#pragma mark Services

- (void)handleGetURLEvent:(NSAppleEventDescriptor *)event withReplyEvent:(NSAppleEventDescriptor *)replyEvent {
    // geo:29.95,-90.066667,3000
    // geo:29.95,-90.066667?z=14    
    // mapboxgl://?center=29.95,-90.066667&zoom=14&bearing=45&pitch=30
    
    NSURLComponents *components = [NSURLComponents componentsWithString:[event paramDescriptorForKeyword:keyDirectObject].stringValue];
    BOOL isGLURL = [components.scheme isEqualToString:@"mapboxgl"];
    BOOL isGeoURL = [components.scheme isEqualToString:@"geo"];

    NSString *centerString;

    NSMutableDictionary<NSString *, NSString *> *params = [[NSMutableDictionary alloc] init];
    for (NSURLQueryItem *queryItem in components.queryItems) {
        params[queryItem.name] = queryItem.value;
    }

    if (isGLURL) {
        centerString = params[@"center"];
    } else if (isGeoURL) {
        NSArray<NSString *> *parsedPath = [components.path componentsSeparatedByString:@";"];
        centerString = parsedPath.firstObject;
        for (NSString *param in [parsedPath subarrayWithRange:NSMakeRange(1, parsedPath.count - 1)]) {
            NSArray *parts = [param componentsSeparatedByString:@"="];
            if (parts.count >= 2) {
                params[parts[0]] = parts[1];
            }
        }
    }    

    MGLMapCamera *camera = [MGLMapCamera camera];
    NSString *zoomLevelString = params[@"zoom"] ?: params[@"z"];
    self.pendingZoomLevel = zoomLevelString.length ? zoomLevelString.doubleValue : -1;

    NSString *directionString = params[@"bearing"];
    if (directionString.length) {
        camera.heading = directionString.doubleValue;
    }

    if (centerString) {
        NSArray *coordinateValues = [centerString componentsSeparatedByString:@","];
        if (coordinateValues.count >= 2) {
            camera.centerCoordinate = CLLocationCoordinate2DMake([coordinateValues[0] doubleValue],
                                                                 [coordinateValues[1] doubleValue]);
        }
        if (coordinateValues.count == 3) {
            camera.altitude = [coordinateValues[2] doubleValue];
        }
    }

    NSString *pitchString = params[@"pitch"];
    if (pitchString.length) {
        camera.pitch = pitchString.doubleValue;
    }

    self.pendingCamera = camera;
    [[NSDocumentController sharedDocumentController] openUntitledDocumentAndDisplay:YES error:NULL];
}

#pragma mark Offline pack management

- (IBAction)showOfflinePacksPanel:(id)sender {
    [self.offlinePacksPanel makeKeyAndOrderFront:sender];

    for (MGLOfflinePack *pack in self.offlinePacksArrayController.arrangedObjects) {
        [pack requestProgress];
    }
}

- (IBAction)delete:(id)sender {
    for (MGLOfflinePack *pack in self.offlinePacksArrayController.selectedObjects) {
        [self unwatchOfflinePack:pack];
        [[MGLOfflineStorage sharedOfflineStorage] removePack:pack withCompletionHandler:^(NSError * _Nullable error) {
            if (error) {
                [[NSAlert alertWithError:error] runModal];
            }
        }];
    }
}

- (IBAction)chooseOfflinePack:(id)sender {
    for (MGLOfflinePack *pack in self.offlinePacksArrayController.selectedObjects) {
        switch (pack.state) {
            case MGLOfflinePackStateComplete:
            {
                if ([pack.region isKindOfClass:[MGLTilePyramidOfflineRegion class]]) {
                    MGLTilePyramidOfflineRegion *region = (MGLTilePyramidOfflineRegion *)pack.region;
                    self.pendingVisibleCoordinateBounds = region.bounds;
                    self.pendingMinimumZoomLevel = region.minimumZoomLevel;
                    self.pendingMaximumZoomLevel = region.maximumZoomLevel;
                    [[NSDocumentController sharedDocumentController] openUntitledDocumentAndDisplay:YES error:NULL];
                }
                break;
            }

            case MGLOfflinePackStateInactive:
                [self watchOfflinePack:pack];
                [pack resume];
                break;

            case MGLOfflinePackStateActive:
                [pack suspend];
                [self unwatchOfflinePack:pack];
                break;

            default:
                break;
        }
    }
}

- (void)watchOfflinePack:(MGLOfflinePack *)pack {
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(offlinePackDidChangeProgress:) name:MGLOfflinePackProgressChangedNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(offlinePackDidReceiveError:) name:MGLOfflinePackErrorNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(offlinePackDidReceiveError:) name:MGLOfflinePackMaximumMapboxTilesReachedNotification object:nil];
}

- (void)unwatchOfflinePack:(MGLOfflinePack *)pack {
    [[NSNotificationCenter defaultCenter] removeObserver:self name:nil object:pack];
}

- (void)offlinePackDidChangeProgress:(NSNotification *)notification {
    MGLOfflinePack *pack = notification.object;
    if (pack.state == MGLOfflinePackStateComplete) {
        [[NSSound soundNamed:@"Glass"] play];
    }
}

- (void)offlinePackDidReceiveError:(NSNotification *)notification {
    [[NSSound soundNamed:@"Basso"] play];
}

#pragma mark Help methods

- (IBAction)showShortcuts:(id)sender {
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = @"MapLibre GL Help";
    alert.informativeText = @"\
• To scroll, swipe with two fingers on a trackpad, or drag the cursor, or press the arrow keys.\n\
• To zoom in, pinch two fingers apart on a trackpad, or double-click, or hold down Shift while dragging the cursor down, or hold down Option while pressing the up key.\n\
• To zoom out, pinch two fingers together on a trackpad, or double-tap with two fingers on a trackpad, or double-tap on a mouse, or hold down Shift while dragging the cursor up, or hold down Option while pressing the down key.\n\
• To rotate, move two fingers opposite each other in a circle on a trackpad, or hold down Option while dragging the cursor left and right, or hold down Option while pressing the left and right arrow keys.\n\
• To tilt, hold down Option while dragging the cursor up and down.\n\
• To drop a pin, click and hold.\
";
    [alert runModal];
}

- (IBAction)showPreferences:(id)sender {
    [self.preferencesWindow makeKeyAndOrderFront:sender];
}

- (IBAction)print:(id)sender {
    NSDocument *currentDocument = [NSDocumentController sharedDocumentController].currentDocument;
    if ([currentDocument isKindOfClass:[MapDocument class]]) {
        MGLMapView *mapView = [(MapDocument *)currentDocument mapView];
        [mapView print:sender];
    }
}

#pragma mark User interface validation

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem {
    if (menuItem.action == @selector(showShortcuts:)) {
        return YES;
    }
    if (menuItem.action == @selector(showPreferences:)) {
        return YES;
    }
    if (menuItem.action == @selector(showOfflinePacksPanel:)) {
        return YES;
    }
    if (menuItem.action == @selector(print:)) {
        return YES;
    }
    if (menuItem.action == @selector(delete:)) {
        return self.offlinePacksArrayController.selectedObjects.count;
    }
    return NO;
}

#pragma mark NSWindowDelegate methods

- (void)windowWillClose:(NSNotification *)notification {
    NSWindow *window = notification.object;
    if (window == self.preferencesWindow) {
        [window makeFirstResponder:nil];
    }
}

@end
