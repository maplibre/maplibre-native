#import <Mapbox.h>
#import <XCTest/XCTest.h>

@interface MLNSourceQueryTests : XCTestCase <MLNMapViewDelegate>

@end

@implementation MLNSourceQueryTests

- (void) testQueryVectorTileSource {
    MLNVectorTileSource *source = [[MLNVectorTileSource alloc] initWithIdentifier:@"vector" tileURLTemplates:@[@"fake"] options:nil];
    NSSet *sourceLayers = [NSSet setWithObjects:@"buildings", @"water", nil];
    NSArray* features = [source featuresInSourceLayersWithIdentifiers:sourceLayers predicate:nil];
    // Source not added yet, so features is 0
    XCTAssertEqual([features count], 0);
}

- (void) testQueryShapeSource {
    MLNShapeSource *source = [[MLNShapeSource alloc] initWithIdentifier:@"shape" shape:[MLNShapeCollection shapeCollectionWithShapes:@[]] options:nil];
    NSArray* features = [source featuresMatchingPredicate:nil];
    // Source not added yet, so features is 0
    XCTAssertEqual([features count], 0);
}

@end
