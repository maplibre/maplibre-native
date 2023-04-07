#import "MLNMapViewIntegrationTest.h"
#import "MLNMapSnapshotter_Private.h"

@interface MLNMapSnapshotter ()
@property (nonatomic) BOOL cancelled;
@end


@interface MLNMapSnapshotterTest : MLNMapViewIntegrationTest
@end

// Convenience func to create snapshotter
MLNMapSnapshotter* snapshotterWithCoordinates(CLLocationCoordinate2D coordinates, CGSize size) {
    // Create snapshot options
    MLNMapCamera* mapCamera    = [[MLNMapCamera alloc] init];
    mapCamera.pitch            = 20;
    mapCamera.centerCoordinate = coordinates;
    MLNMapSnapshotOptions* options = [[MLNMapSnapshotOptions alloc] initWithStyleURL:[[MLNStyle predefinedStyle:@"Hybrid"] url]
                                                                              camera:mapCamera
                                                                                size:size];
    options.zoomLevel = 10;

    // Create and start the snapshotter
    MLNMapSnapshotter* snapshotter = [[MLNMapSnapshotter alloc] initWithOptions:options];
    return snapshotter;
}

MLNMapSnapshotter* snapshotterWithBounds(MLNCoordinateBounds bounds, CGSize size) {

    MLNMapCamera* mapCamera = [[MLNMapCamera alloc] init];
    MLNMapSnapshotOptions* options = [[MLNMapSnapshotOptions alloc] initWithStyleURL:[[MLNStyle predefinedStyle:@"Hybrid"] url]
                                                                              camera:mapCamera
                                                                                size:size];
    options.coordinateBounds = bounds;

    // Create and start the snapshotter
    MLNMapSnapshotter* snapshotter = [[MLNMapSnapshotter alloc] initWithOptions:options];
    return snapshotter;
}



@implementation MLNMapSnapshotterTest

- (void)setUp {
    [super setUp];
    [MLNSettings useWellKnownTileServer:MLNMapTiler];
}

- (void)testMultipleSnapshotsWithASingleSnapshotterLOCKED {
    CGSize size = self.mapView.bounds.size;

    XCTestExpectation *expectation = [self expectationWithDescription:@"snapshots"];
    expectation.expectedFulfillmentCount = 2;
    expectation.assertForOverFulfill = YES;

    CLLocationCoordinate2D coord = CLLocationCoordinate2DMake(30.0, 30.0);

    MLNMapSnapshotter *snapshotter = snapshotterWithCoordinates(coord, size);
    XCTAssertNotNil(snapshotter);

    [snapshotter startWithCompletionHandler:^(MLNMapSnapshot * _Nullable snapshot, NSError * _Nullable error) {
        [expectation fulfill];
    }];

    @try {
        [snapshotter startWithCompletionHandler:^(MLNMapSnapshot * _Nullable snapshot, NSError * _Nullable error) {
            XCTFail(@"Should not be called - but should it?");
        }];
        XCTFail(@"Should not be called");
    }
    @catch (NSException *exception) {
        XCTAssert(exception.name == NSInternalInconsistencyException);
        [expectation fulfill];
    }

    [self waitForExpectations:@[expectation] timeout:10.0];
}

- (void)testSnapshotterWithoutStrongReferenceLOCKED {
    XCTestExpectation *expectation = [self expectationWithDescription:@"Completion handler should be called even if there’s no strong reference to the snapshotter."];
    
    CGSize size = self.mapView.bounds.size;
    CLLocationCoordinate2D coordinates = CLLocationCoordinate2DMake(30.0, 30.0);
    __weak MLNMapSnapshotter *weakSnapshotter;
    @autoreleasepool {
        MLNMapSnapshotter *snapshotter = snapshotterWithCoordinates(coordinates, size);
        weakSnapshotter = snapshotter;
        __weak __typeof__(self) weakSelf = self;
        [snapshotter startWithCompletionHandler:^(MLNMapSnapshot * _Nullable snapshot, NSError * _Nullable error) {
            MLNTestAssertNotNil(weakSelf, weakSnapshotter, @"Snapshotter should not go away until the completion handler finishes executing.");
            // This completion block should be called.
            [expectation fulfill];
        }];
        MLNTestAssertNotNil(self, weakSnapshotter, @"Locally scoped snapshotter should not go away while in scope.");
    }
    MLNTestAssertNotNil(self, weakSnapshotter, @"Snapshotter should remain until it finishes even there’s no strong reference to it.");
    
    [self waitForExpectations:@[expectation] timeout:10.0];
    MLNTestAssertNil(self, weakSnapshotter, @"Completion handler should not leak snapshotter.");
}

- (void)testSnapshotterInBackgroundWithoutStrongReferenceLOCKED {
    // See also https://github.com/mapbox/mapbox-gl-native/issues/12336

    NSTimeInterval timeout         = 10.0;
    XCTestExpectation *expectation = [self expectationWithDescription:@"snapshot"];
    CGSize size                    = self.mapView.bounds.size;
    CLLocationCoordinate2D coord   = CLLocationCoordinate2DMake(30.0, 30.0);

    // Test triggering to main queue
    dispatch_queue_t backgroundQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

    __weak __typeof__(self) weakself = self;
    __block __weak MLNMapSnapshotter *weakSnapshotter;

    dispatch_async(backgroundQueue, ^{

        dispatch_group_t dg = dispatch_group_create();
        dispatch_group_enter(dg);

        dispatch_async(dispatch_get_main_queue(), ^{
            @autoreleasepool {
                MLNMapSnapshotter *snapshotter = snapshotterWithCoordinates(coord, size);
                weakSnapshotter = snapshotter;
                
                [snapshotter startWithCompletionHandler:^(MLNMapSnapshot * _Nullable snapshot, NSError * _Nullable error) {
                    // We expect this completion block to be called with an error
                    __typeof__(self) strongself = weakself;

                    MLNTestAssertNotNil(strongself, snapshot);
                    MLNTestAssertNil(strongself, error, @"Snapshotter should have completed");
                    MLNTestAssertNotNil(strongself, weakSnapshotter, @"Snapshotter should not have been deallocated yet.");

                    dispatch_group_leave(dg);
                }];
            }
        });

        dispatch_group_notify(dg, dispatch_get_main_queue(), ^{
            [expectation fulfill];
        });
    });

    [self waitForExpectations:@[expectation] timeout:timeout];
    MLNTestAssertNil(self, weakSnapshotter, @"Snapshotter should not leak.");
}

- (void)testSnapshotterUsingNestedDispatchQueuesLOCKED {
    // This is the opposite pair to the above test `testDeallocatingSnapshotterDuringSnapshot`
    // The only significant difference is that the snapshotter is a `__block` variable, so
    // its lifetime should continue until it's set to nil in the completion block.
    // See also https://github.com/mapbox/mapbox-gl-native/issues/12336

    NSTimeInterval timeout         = 10.0;
    XCTestExpectation *expectation = [self expectationWithDescription:@"snapshot"];
    CGSize size                    = self.mapView.bounds.size;
    CLLocationCoordinate2D coord   = CLLocationCoordinate2DMake(30.0, 30.0);

    // Test triggering to main queue
    dispatch_queue_t backgroundQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

    __weak __typeof__(self) weakself = self;

    dispatch_async(backgroundQueue, ^{

        dispatch_group_t dg = dispatch_group_create();
        dispatch_group_enter(dg);

        dispatch_async(dispatch_get_main_queue(), ^{

            __block MLNMapSnapshotter *snapshotter = snapshotterWithCoordinates(coord, size);

            [snapshotter startWithCompletionHandler:^(MLNMapSnapshot * _Nullable snapshot, NSError * _Nullable error) {
                // We expect this completion block to be called with no error
                __typeof__(self) strongself = weakself;
                MLNTestAssertNotNil(strongself, snapshot);
                MLNTestAssertNil(strongself, error, @"Snapshotter should have completed");
                dispatch_group_leave(dg);
                snapshotter = nil;
            }];
        });

        dispatch_group_notify(dg, dispatch_get_main_queue(), ^{
            [expectation fulfill];
        });
    });

    [self waitForExpectations:@[expectation] timeout:timeout];
}

- (void)testCancellingSnapshotLOCKED {
    CGSize size                    = self.mapView.bounds.size;
    CLLocationCoordinate2D coord   = CLLocationCoordinate2DMake(30.0, 30.0);

    __block __weak MLNMapSnapshotter *weakSnapshotter;
    @autoreleasepool {
        MLNMapSnapshotter *snapshotter = snapshotterWithCoordinates(coord, size);
        weakSnapshotter = snapshotter;
        __weak __typeof__(self) weakself = self;
        [snapshotter startWithCompletionHandler:^(MLNMapSnapshot * _Nullable snapshot, NSError * _Nullable error) {
            __typeof__(self) strongself = weakself;
            MLNTestFail(strongself, @"Completion handler should not be called when canceling.");
        }];
    }

    MLNTestAssertNotNil(self, weakSnapshotter);
    [weakSnapshotter cancel];
    MLNTestAssertNil(self, weakSnapshotter, @"Snapshotter should not leak after being canceled.");
}

- (void)testAllocatingSnapshotOnBackgroundQueueLOCKED {
    XCTestExpectation *expectation = [self expectationWithDescription:@"snapshots"];

    CGSize size                  = self.mapView.bounds.size;
    CLLocationCoordinate2D coord = CLLocationCoordinate2DMake(30.0, 30.0);

    dispatch_queue_attr_t attr = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, QOS_CLASS_USER_INITIATED, QOS_MIN_RELATIVE_PRIORITY);
    dispatch_queue_t backgroundQueue = dispatch_queue_create(__PRETTY_FUNCTION__, attr);

    dispatch_async(backgroundQueue, ^{

        // Create the snapshotter - DO NOT START.
        MLNMapSnapshotter* snapshotter = snapshotterWithCoordinates(coord, size);

        dispatch_group_t group = dispatch_group_create();
        dispatch_group_enter(group);

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            dispatch_group_leave(group);
        });

        dispatch_group_wait(group, DISPATCH_TIME_FOREVER);

        snapshotter = nil;

        dispatch_sync(dispatch_get_main_queue(), ^{
            [expectation fulfill];
        });
    });

    [self waitForExpectations:@[expectation] timeout:2.0];
}

- (void)testSnapshotterFromBackgroundQueueShouldFailLOCKED {
    CGSize size = self.mapView.bounds.size;
    CLLocationCoordinate2D coord = CLLocationCoordinate2DMake(30.0, 30.0);

    XCTestExpectation *expectation = [self expectationWithDescription:@"snapshots"];
    expectation.expectedFulfillmentCount = 1;
    expectation.assertForOverFulfill = YES;

    __weak __typeof__(self) weakself = self;

    dispatch_queue_attr_t attr = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, QOS_CLASS_USER_INITIATED, QOS_MIN_RELATIVE_PRIORITY); // also for concurrent
    dispatch_queue_t backgroundQueue = dispatch_queue_create(__PRETTY_FUNCTION__, attr);

    // Use dispatch_group to keep the backgroundQueue block around (and
    // so also the MLNMapSnapshotter
    dispatch_group_t group = dispatch_group_create();
    dispatch_group_enter(group);

    dispatch_async(backgroundQueue, ^{

        MLNMapSnapshotter *snapshotter = snapshotterWithCoordinates(coord, size);
        XCTAssertNotNil(snapshotter);

        MLNMapSnapshotCompletionHandler completion = ^(MLNMapSnapshot * _Nullable snapshot, NSError * _Nullable error) {
            // The completion block should not be called
            MLNTestFail(weakself);
            dispatch_group_leave(group);
        };

        @try {
            [snapshotter startWithCompletionHandler:completion];
            MLNTestFail(weakself, @"startWithCompletionHandler: should raise an exception");
        }
        @catch (NSException *exception) {
            MLNTestAssert(weakself, exception.name == NSInvalidArgumentException);
            dispatch_group_leave(group);
        }

        // Wait for the snapshot to complete
        dispatch_group_wait(group, DISPATCH_TIME_FOREVER);

        snapshotter = nil;

        dispatch_sync(dispatch_get_main_queue(), ^{
            [expectation fulfill];
        });
    });

    [self waitForExpectations:@[expectation] timeout:60.0];
}

- (void)testMultipleSnapshottersLOCKEDandPENDING {
    NSUInteger numSnapshots = 8;
    CGSize size = self.mapView.bounds.size;

    XCTestExpectation *expectation = [self expectationWithDescription:@"snapshots"];
    expectation.expectedFulfillmentCount = numSnapshots;
    expectation.assertForOverFulfill = YES;

    __weak __typeof__(self) weakself = self;

    for (size_t run = 0; run < numSnapshots; run++) {

        float ratio = (float)run/(float)numSnapshots;
        float lon = (ratio*120.0) + ((1.0-ratio)*54.0);
        CLLocationCoordinate2D coord = CLLocationCoordinate2DMake(57.0, lon);

        @autoreleasepool {
            MLNMapSnapshotter *snapshotter = snapshotterWithCoordinates(coord, size);
            [snapshotter startWithCompletionHandler:^(MLNMapSnapshot * _Nullable snapshot, NSError * _Nullable error) {
                
                // This should be the main queue
                __typeof__(self) strongself = weakself;
                
                MLNTestAssertNotNil(strongself, strongself);
                
                MLNTestAssertNotNil(strongself, snapshot);
                MLNTestAssertNotNil(strongself, snapshot.image);
                MLNTestAssertNil(strongself, error, @"Snapshot should not error with: %@", error);
                
                // Change this to XCTAttachmentLifetimeKeepAlways to be able to look at the snapshots after running
                XCTAttachment *attachment = [XCTAttachment attachmentWithImage:snapshot.image];
                attachment.lifetime = XCTAttachmentLifetimeDeleteOnSuccess;
                [strongself addAttachment:attachment];
                
                [expectation fulfill];
            }];
        }
    } // end for loop

    [self waitForExpectations:@[expectation] timeout:60.0];
}

- (void)testSnapshotPointConversionLOCKED {
    CGSize size = self.mapView.bounds.size;

    XCTestExpectation *expectation = [self expectationWithDescription:@"snapshot"];
    expectation.expectedFulfillmentCount = 1;
    expectation.assertForOverFulfill = YES;

    CLLocationCoordinate2D coord = CLLocationCoordinate2DMake(30.0, 30.0);

    MLNMapSnapshotter *snapshotter = snapshotterWithCoordinates(coord, size);
    XCTAssertNotNil(snapshotter);

    __weak __typeof__(self) weakself = self;

    [snapshotter startWithCompletionHandler:^(MLNMapSnapshot * _Nullable snapshot, NSError * _Nullable error) {

        __typeof__(self) myself = weakself;

        MLNTestAssertNotNil(myself, snapshot);

        CGPoint point = [snapshot pointForCoordinate:coord];

        CGFloat epsilon = 0.000001;

        MLNTestAssertEqualWithAccuracy(myself, point.x, size.width/2.0, epsilon);
        MLNTestAssertEqualWithAccuracy(myself, point.y, size.height/2.0, epsilon);

        CLLocationCoordinate2D coord2 = [snapshot coordinateForPoint:point];

        MLNTestAssertEqualWithAccuracy(myself, coord.latitude, coord2.latitude, epsilon);
        MLNTestAssertEqualWithAccuracy(myself, coord.longitude, coord2.longitude, epsilon);

        [expectation fulfill];
    }];

    [self waitForExpectations:@[expectation] timeout:10.0];
}

- (void)testSnapshotPointConversionCoordinateOrderingLOCKED {
    CGSize size = self.mapView.bounds.size;

    XCTestExpectation *expectation = [self expectationWithDescription:@"snapshot"];
    expectation.expectedFulfillmentCount = 1;
    expectation.assertForOverFulfill = YES;

    CLLocationCoordinate2D coord = CLLocationCoordinate2DMake(30.0, 30.0);

    MLNMapSnapshotter *snapshotter = snapshotterWithCoordinates(coord, size);
    XCTAssertNotNil(snapshotter);

    __weak __typeof__(self) weakself = self;

    [snapshotter startWithCompletionHandler:^(MLNMapSnapshot * _Nullable snapshot, NSError * _Nullable error) {

        __typeof__(self) myself = weakself;

        CGFloat epsilon = 0.000001;

        MLNTestAssertNotNil(myself, snapshot);

        CLLocationCoordinate2D coordTL = [snapshot coordinateForPoint:CGPointZero];

        MLNTestAssert(myself, coordTL.longitude < coord.longitude);
        MLNTestAssert(myself, coordTL.latitude > coord.latitude);

        // And check point
        CGPoint tl = [snapshot pointForCoordinate:coordTL];
        MLNTestAssertEqualWithAccuracy(myself, tl.x, 0.0, epsilon);
        MLNTestAssertEqualWithAccuracy(myself, tl.y, 0.0, epsilon);

        CLLocationCoordinate2D coordBR = [snapshot coordinateForPoint:CGPointMake(size.width, size.height)];

        MLNTestAssert(myself, coordBR.longitude > coord.longitude);
        MLNTestAssert(myself, coordBR.latitude < coord.latitude);

        [expectation fulfill];
    }];

    [self waitForExpectations:@[expectation] timeout:10.0];
}

- (void)testSnapshotWithOverlayHandlerFailureLOCKED {
    CGSize size = self.mapView.bounds.size;

    XCTestExpectation *expectation = [self expectationWithDescription:@"snapshot with overlay fails"];
    expectation.expectedFulfillmentCount = 2;

    CLLocationCoordinate2D coord = CLLocationCoordinate2DMake(30.0, 30.0);

    MLNMapSnapshotter *snapshotter = snapshotterWithCoordinates(coord, size);
    XCTAssertNotNil(snapshotter);

    [snapshotter startWithOverlayHandler:^(MLNMapSnapshotOverlay *snapshotOverlay) {
        XCTAssertNotNil(snapshotOverlay);
        
        UIGraphicsEndImageContext();
        [expectation fulfill];
    } completionHandler:^(MLNMapSnapshot * _Nullable snapshot, NSError * _Nullable error) {
        XCTAssertNil(snapshot);
        XCTAssertNotNil(error);
        XCTAssertEqualObjects(error.domain, MLNErrorDomain);
        XCTAssertEqual(error.code, MLNErrorCodeSnapshotFailed);
        XCTAssertEqualObjects(error.localizedDescription, @"Failed to generate composited snapshot.");
                
        [expectation fulfill];
    }];

    [self waitForExpectations:@[expectation] timeout:10.0];
}

- (void)testSnapshotWithOverlayHandlerSuccessLOCKED {
    CGSize size = self.mapView.bounds.size;
    CGRect snapshotRect = CGRectMake(0, 0, size.width, size.height);

    XCTestExpectation *expectation = [self expectationWithDescription:@"snapshot with overlay succeeds"];
    expectation.expectedFulfillmentCount = 2;

    CLLocationCoordinate2D coord = CLLocationCoordinate2DMake(30.0, 30.0);

    MLNMapSnapshotter *snapshotter = snapshotterWithCoordinates(coord, size);
    XCTAssertNotNil(snapshotter);
    
    CGFloat scale = snapshotter.options.scale;
    
    [snapshotter startWithOverlayHandler:^(MLNMapSnapshotOverlay *snapshotOverlay) {
        XCTAssertNotNil(snapshotOverlay);
        
        CGFloat width = CGBitmapContextGetWidth(snapshotOverlay.context);
        CGFloat height = CGBitmapContextGetHeight(snapshotOverlay.context);

        CGRect contextRect = CGContextConvertRectToDeviceSpace(snapshotOverlay.context, CGRectMake(0, 0, 1, 0));
        CGFloat scaleFromContext = contextRect.size.width;
        XCTAssertEqual(scale, scaleFromContext);
        XCTAssertEqual(width, size.width*scale);
        XCTAssertEqual(height, size.height*scale);
        
        CGContextSetFillColorWithColor(snapshotOverlay.context, [UIColor.greenColor CGColor]);
        CGContextSetAlpha(snapshotOverlay.context, 0.2);
        CGContextAddRect(snapshotOverlay.context, snapshotRect);
        CGContextFillRect(snapshotOverlay.context, snapshotRect);
        [expectation fulfill];
    } completionHandler:^(MLNMapSnapshot * _Nullable snapshot, NSError * _Nullable error) {
        XCTAssertNil(error);
        XCTAssertNotNil(snapshot);
        [expectation fulfill];
    }];

    [self waitForExpectations:@[expectation] timeout:10.0];
}

- (void)testSnapshotCoordinatesWithOverlayHandlerLOCKED {
    CGSize size = self.mapView.bounds.size;

    XCTestExpectation *expectation = [self expectationWithDescription:@"snapshot with overlay succeeds"];
    expectation.expectedFulfillmentCount = 2;

    CLLocationCoordinate2D london = { .latitude = 51.5074, .longitude = -0.1278 };
    CLLocationCoordinate2D paris  = { .latitude = 48.8566, .longitude = 2.3522 };

    MLNCoordinateBounds bounds = {
        .ne = london,
        .sw = paris
    };

    MLNMapSnapshotter *snapshotter = snapshotterWithBounds(bounds, size);
    XCTAssertNotNil(snapshotter);

    void (^testCoordinates)(id<MLNMapSnapshotProtocol>) = ^(id<MLNMapSnapshotProtocol> snapshot){
        XCTAssertNotNil(snapshot);

        CGPoint londonPoint = [snapshot pointForCoordinate:london];
        CGPoint parisPoint  = [snapshot pointForCoordinate:paris];

        XCTAssertEqualWithAccuracy(londonPoint.x, 0, 0.1);
        XCTAssertEqualWithAccuracy(parisPoint.x, size.width, 0.1);

        // Vertically, London and Paris are inset (due to the size vs coordinate bounds)
        XCTAssert(parisPoint.y > londonPoint.y);
        XCTAssert(londonPoint.y > 0.0);
        XCTAssert(parisPoint.y < size.height);

        CLLocationCoordinate2D london2 = [snapshot coordinateForPoint:londonPoint];
        CLLocationCoordinate2D paris2  = [snapshot coordinateForPoint:parisPoint];

        XCTAssertEqualWithAccuracy(london.latitude,  london2.latitude,  0.0000001);
        XCTAssertEqualWithAccuracy(london.longitude, london2.longitude, 0.0000001);
        XCTAssertEqualWithAccuracy(paris.latitude,   paris2.latitude,   0.0000001);
        XCTAssertEqualWithAccuracy(paris.longitude,  paris2.longitude,  0.0000001);
    };

    [snapshotter startWithOverlayHandler:^(MLNMapSnapshotOverlay *snapshotOverlay) {
        XCTAssert([snapshotOverlay conformsToProtocol:@protocol(MLNMapSnapshotProtocol)]);
        testCoordinates((id<MLNMapSnapshotProtocol>)snapshotOverlay);

        [expectation fulfill];
    } completionHandler:^(MLNMapSnapshot * _Nullable snapshot, NSError * _Nullable error) {
        XCTAssert([snapshot conformsToProtocol:@protocol(MLNMapSnapshotProtocol)]);
        testCoordinates((id<MLNMapSnapshotProtocol>)snapshot);

        [expectation fulfill];
    }];

    [self waitForExpectations:@[expectation] timeout:10.0];
}


@end
