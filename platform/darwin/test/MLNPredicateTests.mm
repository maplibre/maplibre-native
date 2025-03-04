#import <XCTest/XCTest.h>
#import <Mapbox.h>

#import "NSPredicate+MLNPrivateAdditions.h"
#import "MLNValueEvaluator.h"

@implementation NSString (MLNAdditions)

- (NSString *)stringByRemovingPointerAddresses {
    return [self stringByReplacingOccurrencesOfString:@"\\b0x[0-9a-f]+\\b"
                                           withString:@"0xdeadbeef"
                                              options:NSRegularExpressionSearch
                                                range:NSMakeRange(0, self.length)];
}

@end

@interface MLNPredicateTests : XCTestCase
@end

@implementation MLNPredicateTests

- (void)testUnsupportedFilterPredicates {
    XCTAssertThrowsSpecificNamed([NSPredicate predicateWithFormat:@"a BEGINSWITH 'L'"].mgl_filter, NSException, NSInvalidArgumentException);
    XCTAssertThrowsSpecificNamed([NSPredicate predicateWithFormat:@"a ENDSWITH 'itude'"].mgl_filter, NSException, NSInvalidArgumentException);
    XCTAssertThrowsSpecificNamed([NSPredicate predicateWithFormat:@"a LIKE 'glob?trotter'"].mgl_filter, NSException, NSInvalidArgumentException);
    XCTAssertThrowsSpecificNamed([NSPredicate predicateWithFormat:@"a MATCHES 'i\\w{18}n'"].mgl_filter, NSException, NSInvalidArgumentException);
    NSPredicate *selectorPredicate = [NSPredicate predicateWithFormat:@"(SELF isKindOfClass: %@)", [MLNPolyline class]];
    XCTAssertThrowsSpecificNamed(selectorPredicate.mgl_filter, NSException, NSInvalidArgumentException);

    XCTAssertThrowsSpecificNamed([NSPredicate predicateWithBlock:^BOOL(id _Nullable evaluatedObject, NSDictionary<NSString *, id> * _Nullable bindings) {
        XCTAssertTrue(NO, @"Predicate block should not be evaluated.");
        return NO;
    }].mgl_filter, NSException, NSInvalidArgumentException);
}

- (void)testComparisonPredicates {
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"x == YES"];
        NSArray *jsonExpression = @[@"==", @[@"get", @"x"],  @YES];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:jsonExpression], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:jsonExpression]
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"CAST(x, 'NSNumber') < 5"];
        NSArray *jsonExpression = @[@"<", @[@"to-number", @[@"get", @"x"]], @5];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:jsonExpression], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:jsonExpression]
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"CAST(x, 'NSNumber') > 5"];
        NSArray *jsonExpression = @[@">", @[@"to-number", @[@"get", @"x"]], @5];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:jsonExpression], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:jsonExpression]
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"CAST(x, 'NSNumber') <= 5"];
        NSArray *jsonExpression = @[@"<=", @[@"to-number", @[@"get", @"x"]], @5];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:jsonExpression], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:jsonExpression]
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"CAST(x, 'NSNumber') >= 5"];
        NSArray *jsonExpression = @[@">=", @[@"to-number", @[@"get", @"x"]], @5];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:jsonExpression], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:jsonExpression]
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"CAST(x, 'NSString') > 'value'"];
        NSArray *jsonExpression = @[@">",  @[@"to-string", @[@"get", @"x"]], @"value"];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:jsonExpression], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:jsonExpression]
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"a = 'b'"];
        NSArray *jsonExpression = @[@"==", @[@"get", @"a"], @"b"];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        [self testSymmetryWithPredicate:predicate
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"$geometryType = 'Point'"];
        NSArray *jsonExpression = @[@"==", @[@"geometry-type"], @"Point"];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        [self testSymmetryWithPredicate:predicate
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"$featureIdentifier = 67086180"];
        NSArray *jsonExpression = @[@"==", @[@"id"], @67086180];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        [self testSymmetryWithPredicate:predicate
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"$featureIdentifier = nil"];
        NSArray *jsonExpression = @[@"==", @[@"id"], [NSNull null]];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        [self testSymmetryWithPredicate:predicate
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"a = nil"];
        NSArray *jsonExpression = @[@"==", @[@"get", @"a"], [NSNull null]];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        [self testSymmetryWithPredicate:predicate
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"$geometryType != 'Point'"];
        NSArray *jsonExpression = @[@"!=", @[@"geometry-type"], @"Point"];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        [self testSymmetryWithPredicate:predicate
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"$featureIdentifier != 67086180"];
        NSArray *jsonExpression = @[@"!=", @[@"id"], @67086180];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        [self testSymmetryWithPredicate:predicate
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"$featureIdentifier != nil"];
        NSArray *jsonExpression = @[@"!=", @[@"id"],  [NSNull null]];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        [self testSymmetryWithPredicate:predicate
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"a != 'b'"];
        NSArray *jsonExpression = @[@"!=", @[@"get", @"a"], @"b"];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        [self testSymmetryWithPredicate:predicate
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"a != nil"];
        NSArray *jsonExpression = @[@"!=", @[@"get", @"a"], [NSNull null]];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        [self testSymmetryWithPredicate:predicate
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"CAST(a, 'NSString') < 'b'"];
        NSArray *jsonExpression = @[@"<", @[@"to-string", @[@"get", @"a"]], @"b"];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:jsonExpression], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:jsonExpression]
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"CAST(a, 'NSString') <= 'b'"];
        NSArray *jsonExpression = @[@"<=", @[@"to-string", @[@"get", @"a"]], @"b"];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:jsonExpression], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:jsonExpression]
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"CAST(a, 'NSString') > 'b'"];
        NSArray *jsonExpression = @[@">", @[@"to-string", @[@"get", @"a"]], @"b"];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:jsonExpression], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:jsonExpression]
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"CAST(a, 'NSString') >= 'b'"];
        NSArray *jsonExpression = @[@">=", @[@"to-string", @[@"get", @"a"]], @"b"];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:jsonExpression], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:jsonExpression]
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"CAST(a, 'NSString') BETWEEN {'b', 'z'}"];
        NSArray *jsonExpression =@[@"all", @[@"<=", @"b", @[@"to-string", @[@"get", @"a"]]], @[@"<=", @[@"to-string", @[@"get", @"a"]], @"z"]];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:jsonExpression], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:jsonExpression]
                          mustRoundTrip:NO];
    }
    {
        NSExpression *limits = [NSExpression expressionForAggregate:@[[NSExpression expressionForConstantValue:@10], [NSExpression expressionForConstantValue:@100]]];
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"CAST(x, 'NSNumber') BETWEEN %@", limits];
        NSArray *jsonExpression = @[@"all", @[@">=", @[@"to-number", @[@"get", @"x"]], @10], @[@"<=", @[@"to-number", @[@"get", @"x"]], @100]];
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:jsonExpression], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:jsonExpression]
                          mustRoundTrip:NO];
    }
    {
        NSArray *expected = @[@"all", @[@"<=", @10, @[@"to-number", @[@"get", @"x"]]], @[@"<=", @[@"to-number", @[@"get", @"x"]], @100]];
        NSExpression *limits = [NSExpression expressionForAggregate:@[[NSExpression expressionForConstantValue:@10], [NSExpression expressionForConstantValue:@100]]];
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"CAST(x, 'NSNumber') BETWEEN %@", limits];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, expected);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:expected], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:expected]
                          mustRoundTrip:NO];
    }
    {
        NSArray *expected = @[@"all", @[@"<=", @10, @[@"to-number", @[@"get", @"x"]]], @[@">=", @100, @[@"to-number", @[@"get", @"x"]]]];
        NSExpression *limits = [NSExpression expressionForAggregate:@[[NSExpression expressionForConstantValue:@10], [NSExpression expressionForConstantValue:@100]]];
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"CAST(x, 'NSNumber') BETWEEN %@", limits];
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:expected], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:expected]
                          mustRoundTrip:NO];
    }
    {
        NSArray *expected = @[@"all", @[@">=", @[@"to-number", @[@"get", @"x"]], @10], @[@">=", @100, @[@"to-number", @[@"get", @"x"]]]];
        NSExpression *limits = [NSExpression expressionForAggregate:@[[NSExpression expressionForConstantValue:@10], [NSExpression expressionForConstantValue:@100]]];
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"CAST(x, 'NSNumber') BETWEEN %@", limits];
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:expected], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:expected]
                          mustRoundTrip:NO];
    }
    {
        NSArray *expected = @[@"in", @[@"id"], @[@"literal", @[@6, @5, @4, @3]]];
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"$featureIdentifier IN { 6, 5, 4, 3}"];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, expected);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:expected], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:expected]
                          mustRoundTrip:YES];
    }
    {
        NSArray *expected = @[@"in", @[@"to-number", @[@"id"]], @[@"literal", @[@3002970001, @3004140052, @3002950027, @3002970033]]];
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"CAST($featureIdentifier, 'NSNumber') IN { 3002970001, 3004140052, 3002950027, 3002970033 }"];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, expected);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:expected], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:expected]
                          mustRoundTrip:YES];
    }
    {
        NSArray *expected = @[@"!", @[@"in", @[@"get", @"x"], @[@"literal", @[@6, @5, @4, @3]]]];
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"NOT x IN { 6, 5, 4, 3}"];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, expected);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:expected], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:expected]
                          mustRoundTrip:YES];
    }
    {
        NSArray *expected = @[@"in", @[@"get", @"a"], @[@"literal", @[@"b", @"c"]]];
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"a IN { 'b', 'c' }"];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, expected);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:expected], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:expected]
                          mustRoundTrip:YES];
    }
    {
        NSArray *expected = @[@"in", @[@"geometry-type"], @[@"literal", @[@"LineString", @"Polygon"]]];
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"%@ IN %@", [NSExpression expressionForVariable:@"geometryType"], @[@"LineString", @"Polygon"]];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, expected);
        NSPredicate *predicateAfter = [NSPredicate predicateWithFormat:@"%@ IN {'LineString', 'Polygon'}", [NSExpression expressionForVariable:@"geometryType"]];
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:expected], predicateAfter);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:expected]
                          mustRoundTrip:YES];
    }
    {
        NSArray *expected = @[@"in", @[@"get", @"x"], @[@"literal", @[@6, @5, @4, @3]]];
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"{ 6, 5, 4, 3 } CONTAINS x"];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, expected);
        NSPredicate *predicateAfter = [NSPredicate predicateWithFormat:@"x IN { 6, 5, 4, 3 }"];
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:expected], predicateAfter);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:expected]
                          mustRoundTrip:YES];
    }
    {
        NSArray *expected = @[@"in", @[@"geometry-type"], @[@"literal", @[@"LineString", @"Polygon"]]];
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"%@ CONTAINS %@", @[@"LineString", @"Polygon"], [NSExpression expressionForVariable:@"geometryType"]];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, expected);
        NSPredicate *predicateAfter = [NSPredicate predicateWithFormat:@"%@ IN {'LineString', 'Polygon'}", [NSExpression expressionForVariable:@"geometryType"]];
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:expected], predicateAfter);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:expected]
                          mustRoundTrip:YES];
    }
    {
        NSArray *expected = @[@"in", @[@"id"], @[@"literal", @[@6, @5, @4, @3]]];
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"{ 6, 5, 4, 3} CONTAINS $featureIdentifier"];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, expected);
        NSPredicate *predicateAfter = [NSPredicate predicateWithFormat:@"$featureIdentifier IN {6, 5, 4, 3}"];
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:expected], predicateAfter);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:expected]
                          mustRoundTrip:YES];
    }
    {
        NSArray *expected = @[@"in", @"📍", @[@"get", @"name"]];
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"'📍' IN name"];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, expected);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:expected], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:expected]
                          mustRoundTrip:YES];
    }
    {
        NSArray *expected = @[@"in", @"📍", @[@"get", @"name"]];
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"name CONTAINS '📍'"];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, expected);
        NSPredicate *predicateAfter = [NSPredicate predicateWithFormat:@"'📍' IN name"];
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:expected], predicateAfter);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:expected]
                          mustRoundTrip:YES];
    }
    {
        NSArray *expected = @[@"in", @"📍", @[@"literal", @"Pinole"]];
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"'📍' IN 'Pinole'"];
#if TARGET_OS_IPHONE
        XCTExpectFailure(@"Awaiting unit test refactoring for https://github.com/maplibre/maplibre-native/issues/331");
#endif
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, expected);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:expected], predicate);
        XCTAssertEqualObjects([NSPredicate mgl_predicateWithFilter:predicate.mgl_filter], [NSPredicate predicateWithValue:NO]);
    }
    {
        NSArray *expected = @[@"in", @"📍", @[@"literal", @"Pinole"]];
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"'Pinole' CONTAINS '📍'"];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, expected);
        NSPredicate *predicateAfter = [NSPredicate predicateWithFormat:@"'📍' IN 'Pinole'"];
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:expected], predicateAfter);
        XCTAssertEqualObjects([NSPredicate mgl_predicateWithFilter:predicate.mgl_filter], [NSPredicate predicateWithValue:NO]);
    }
    {
        NSArray *expected = @[@"in", @[@"id"], @[@"literal", @[@6, @5, @4, @3]]];
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"ANY {6, 5, 4, 3} = $featureIdentifier"];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, expected);
        NSPredicate *predicateAfter = [NSPredicate predicateWithFormat:@"$featureIdentifier IN {6, 5, 4, 3}"];
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:expected], predicateAfter);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:expected]
                          mustRoundTrip:YES];
    }
    {
        NSArray *expected = @[@"!", @[@"in", @[@"id"], @[@"literal", @[@6, @5, @4, @3]]]];
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"NONE {6, 5, 4, 3} = $featureIdentifier"];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, expected);
        NSPredicate *predicateAfter = [NSPredicate predicateWithFormat:@"NOT $featureIdentifier IN {6, 5, 4, 3}"];
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:expected], predicateAfter);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:expected]
                          mustRoundTrip:YES];
    }
    {
        NSArray *expected = @[@"!", @[@"in", @[@"id"], @[@"literal", @[@6, @5, @4, @3]]]];
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"ALL {6, 5, 4, 3} != $featureIdentifier"];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, expected);
        NSPredicate *predicateAfter = [NSPredicate predicateWithFormat:@"NOT $featureIdentifier IN {6, 5, 4, 3}"];
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:expected], predicateAfter);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:expected]
                          mustRoundTrip:YES];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"ANY {6, 5, 4, 3} != $featureIdentifier"];
        XCTAssertThrowsSpecificNamed(predicate.mgl_jsonExpressionObject, NSException, NSInvalidArgumentException);
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"ANY {6, 5, 4, 3} > $featureIdentifier"];
        XCTAssertThrowsSpecificNamed(predicate.mgl_jsonExpressionObject, NSException, NSInvalidArgumentException);
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"NONE {6, 5, 4, 3} != $featureIdentifier"];
        XCTAssertThrowsSpecificNamed(predicate.mgl_jsonExpressionObject, NSException, NSInvalidArgumentException);
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"ALL {6, 5, 4, 3} = $featureIdentifier"];
        XCTAssertThrowsSpecificNamed(predicate.mgl_jsonExpressionObject, NSException, NSInvalidArgumentException);
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"ALL {6, 5, 4, 3} < $featureIdentifier"];
        XCTAssertThrowsSpecificNamed(predicate.mgl_jsonExpressionObject, NSException, NSInvalidArgumentException);
    }
    {
        NSArray *expected = @[
            @"within",
            @{
                @"type": @"Polygon",
                @"coordinates": @[
                    @[
                        @[@0, @0],
                        @[@0, @1],
                        @[@1, @1],
                        @[@1, @0],
                    ],
                ],
            },
        ];
        CLLocationCoordinate2D coordinates[] = {
            { .latitude = 0, .longitude = 0 },
            { .latitude = 1, .longitude = 0 },
            { .latitude = 1, .longitude = 1 },
            { .latitude = 0, .longitude = 1 },
        };
        MLNPolygon *shape = [MLNPolygon polygonWithCoordinates:coordinates count:sizeof(coordinates) / sizeof(coordinates[0])];
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"SELF IN %@", shape];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, expected);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:expected], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:expected]
                          mustRoundTrip:YES];
    }
    {
        NSArray *expected = @[
            @"within",
            @{
                @"type": @"Feature",
                @"id": @"unit",
                @"properties": @{},
                @"geometry": @{
                    @"type": @"Polygon",
                    @"coordinates": @[
                        @[
                            @[@0, @0],
                            @[@0, @1],
                            @[@1, @1],
                            @[@1, @0],
                        ],
                    ],
                },
            },
        ];
        CLLocationCoordinate2D coordinates[] = {
            { .latitude = 0, .longitude = 0 },
            { .latitude = 1, .longitude = 0 },
            { .latitude = 1, .longitude = 1 },
            { .latitude = 0, .longitude = 1 },
        };
        MLNPolygonFeature *feature = [MLNPolygonFeature polygonWithCoordinates:coordinates count:sizeof(coordinates) / sizeof(coordinates[0])];
        feature.identifier = @"unit";
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"%@ CONTAINS SELF", feature];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, expected);
        NSPredicate *predicateAfter = [NSPredicate predicateWithFormat:@"SELF IN %@", feature];
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:expected], predicateAfter);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:expected]
                          mustRoundTrip:YES];
    }
}

- (void)testComparisonPredicatesWithOptions {
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"a =[c] 'b'"];
        NSArray *jsonExpression = @[@"==", @[@"get", @"a"], @"b", @[@"collator", @{@"case-sensitive": @NO, @"diacritic-sensitive": @YES}]];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
#if TARGET_OS_IPHONE
        XCTExpectFailure(@"Awaiting unit test refactoring for https://github.com/maplibre/maplibre-native/issues/331");
#endif
        [self testSymmetryWithPredicate:predicate
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"a =[d] 'b'"];
        NSArray *jsonExpression = @[@"==", @[@"get", @"a"], @"b", @[@"collator", @{@"case-sensitive": @YES, @"diacritic-sensitive": @NO}]];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        [self testSymmetryWithPredicate:predicate
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"a =[cd] 'b'"];
        NSArray *jsonExpression = @[@"==", @[@"get", @"a"], @"b", @[@"collator", @{@"case-sensitive": @NO, @"diacritic-sensitive": @NO}]];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        [self testSymmetryWithPredicate:predicate
                          mustRoundTrip:NO];
    }

    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"a !=[cd] 'b'"];
        NSArray *jsonExpression = @[@"!=", @[@"get", @"a"], @"b", @[@"collator", @{@"case-sensitive": @NO, @"diacritic-sensitive": @NO}]];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        [self testSymmetryWithPredicate:predicate
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"CAST(a, 'NSString') <[cd] 'b'"];
        NSArray *jsonExpression = @[@"<", @[@"to-string", @[@"get", @"a"]], @"b", @[@"collator", @{@"case-sensitive": @NO, @"diacritic-sensitive": @NO}]];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        [self testSymmetryWithPredicate:predicate
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"CAST(a, 'NSString') <=[cd] 'b'"];
        NSArray *jsonExpression = @[@"<=", @[@"to-string", @[@"get", @"a"]], @"b", @[@"collator", @{@"case-sensitive": @NO, @"diacritic-sensitive": @NO}]];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        [self testSymmetryWithPredicate:predicate
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"CAST(a, 'NSString') >[cd] 'b'"];
        NSArray *jsonExpression = @[@">", @[@"to-string", @[@"get", @"a"]], @"b", @[@"collator", @{@"case-sensitive": @NO, @"diacritic-sensitive": @NO}]];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        [self testSymmetryWithPredicate:predicate
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"CAST(a, 'NSString') >=[cd] 'b'"];
        NSArray *jsonExpression = @[@">=", @[@"to-string", @[@"get", @"a"]], @"b", @[@"collator", @{@"case-sensitive": @NO, @"diacritic-sensitive": @NO}]];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        [self testSymmetryWithPredicate:predicate
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"TRUE = MLN_FUNCTION('==', a, 'b', MLN_FUNCTION('collator', %@))", @{
            @"case-sensitive": @NO,
            @"diacritic-sensitive": @NO,
            @"locale": @"tlh",
        }];
        NSArray *jsonExpression = @[@"==", @[@"get", @"a"], @"b",
                                    @[@"collator",
                                      @{@"case-sensitive": @NO,
                                        @"diacritic-sensitive": @NO,
                                        @"locale": @"tlh"}]];
        XCTAssertEqualObjects([predicate.mgl_jsonExpressionObject lastObject], jsonExpression);
    }

    // https://github.com/mapbox/mapbox-gl-js/issues/9339
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"ANY {6, 5, 4, 3} =[c] $featureIdentifier"];
        XCTAssertThrowsSpecificNamed(predicate.mgl_jsonExpressionObject, NSException, NSInvalidArgumentException);
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"NONE {6, 5, 4, 3} =[d] $featureIdentifier"];
        XCTAssertThrowsSpecificNamed(predicate.mgl_jsonExpressionObject, NSException, NSInvalidArgumentException);
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"ALL {6, 5, 4, 3} !=[cd] $featureIdentifier"];
        XCTAssertThrowsSpecificNamed(predicate.mgl_jsonExpressionObject, NSException, NSInvalidArgumentException);
    }
}

- (void)testCompoundPredicates {
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"a == 'b' AND c == 'd'"];
        NSArray *jsonExpression = @[@"all", @[@"==", @[@"get", @"a"], @"b"], @[@"==",  @[@"get", @"c"], @"d"]];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:jsonExpression], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:jsonExpression]
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"a == 'b' OR c == 'd'"];
        NSArray *jsonExpression = @[@"any", @[@"==", @[@"get", @"a"], @"b"], @[@"==",  @[@"get", @"c"], @"d"]];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:jsonExpression], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:jsonExpression]
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"NOT(a == 'b' AND c == 'd')"];
        NSArray *jsonExpression = @[@"!", @[@"all", @[@"==", @[@"get", @"a"], @"b"], @[@"==",  @[@"get", @"c"], @"d"]]];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:jsonExpression], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:jsonExpression]
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"NOT(a == 'b' OR c == 'd')"];
        NSArray *jsonExpression = @[@"!", @[@"any", @[@"==", @[@"get", @"a"], @"b"], @[@"==",  @[@"get", @"c"], @"d"]]];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        XCTAssertEqualObjects([NSPredicate predicateWithMLNJSONObject:jsonExpression], predicate);
        [self testSymmetryWithPredicate:[NSPredicate predicateWithMLNJSONObject:jsonExpression]
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"NOT a == nil"];
        NSArray *jsonExpression = @[@"!", @[@"==", @[@"get", @"a"], [NSNull null]]];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        [self testSymmetryWithPredicate:predicate
                          mustRoundTrip:NO];
    }
    {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"NOT a != nil"];
        NSArray *jsonExpression = @[@"!", @[@"!=", @[@"get", @"a"], [NSNull null]]];
        XCTAssertEqualObjects(predicate.mgl_jsonExpressionObject, jsonExpression);
        [self testSymmetryWithPredicate:predicate
                          mustRoundTrip:NO];
    }
}

- (void)testSymmetryWithPredicate:(NSPredicate *)forwardPredicate mustRoundTrip:(BOOL)mustRoundTrip {
    auto forwardFilter = forwardPredicate.mgl_filter;
    NSPredicate *forwardPredicateAfter = [NSPredicate mgl_predicateWithFilter:forwardFilter];
    if (mustRoundTrip) {
        // A collection of ints may turn into an aggregate of longs, for
        // example, so compare formats instead of the predicates themselves.
        XCTAssertEqualObjects(forwardPredicate.predicateFormat.stringByRemovingPointerAddresses, forwardPredicateAfter.predicateFormat.stringByRemovingPointerAddresses);
    } else {
        XCTAssertEqualObjects(forwardPredicate, forwardPredicateAfter);
    }
}

@end
