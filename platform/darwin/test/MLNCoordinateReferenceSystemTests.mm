#import <MapLibre.h>
#import <XCTest/XCTest.h>

#import "MLNCoordinateReferenceSystem_Private.h"

#include <array>
#include <cmath>
#include <vector>

@interface MLNCoordinateReferenceSystemTests : XCTestCase
@end

@implementation MLNCoordinateReferenceSystemTests

- (MLNCoordinateReferenceSystem *)identityNamed:(NSString *)name extent:(double)extent {
  return [[MLNCoordinateReferenceSystem alloc] initWithName:name
      tileMatrixSet:(MLNTileMatrixSet){{-extent / 2, extent / 2}, extent}
      forward:^MLNCRSPosition(CLLocationCoordinate2D coordinate) {
        return {coordinate.longitude, coordinate.latitude};
      }
      inverse:^CLLocationCoordinate2D(MLNCRSPosition position) {
        return CLLocationCoordinate2DMake(position.y, position.x);
      }];
}

- (void)testAddingRemovingAndAddingAgain {
  NSError *error;
  XCTAssertTrue([MLNStyle addProjection:[self identityNamed:@"test:registry" extent:360]
                                  error:&error]);
  XCTAssertNil(error);

  XCTAssertFalse([MLNStyle addProjection:[self identityNamed:@"test:registry" extent:360]
                                   error:&error]);
  XCTAssertEqualObjects(error.domain, MLNErrorDomain);
  XCTAssertEqual(error.code, MLNErrorCodeInvalidProjection);
  XCTAssertEqualObjects(error.localizedDescription,
                        @"A projection called \"test:registry\" already exists.");

  error = nil;
  XCTAssertTrue([MLNStyle removeProjectionNamed:@"test:registry" error:&error]);
  XCTAssertNil(error);
  XCTAssertTrue([MLNStyle addProjection:[self identityNamed:@"test:registry" extent:360]
                                  error:&error]);
  XCTAssertNil(error);
  XCTAssertTrue([MLNStyle removeProjectionNamed:@"test:registry" error:&error]);
}

- (void)testProjectionsTheRegistryRefuses {
  NSError *error;
  XCTAssertFalse([MLNStyle addProjection:[self identityNamed:@"no-colon" extent:360] error:&error]);
  XCTAssertEqual(error.code, MLNErrorCodeInvalidProjection);

  error = nil;
  XCTAssertFalse([MLNStyle addProjection:[self identityNamed:@"test:flat" extent:0] error:&error]);
  XCTAssertEqual(error.code, MLNErrorCodeInvalidProjection);

  error = nil;
  XCTAssertFalse([MLNStyle removeProjectionNamed:@"identity" error:&error]);
  XCTAssertEqual(error.code, MLNErrorCodeInvalidProjection);

  error = nil;
  XCTAssertTrue([MLNStyle removeProjectionNamed:@"test:never-added" error:&error]);
  XCTAssertNil(error);
}

- (void)testPositionsWithoutAnAnswerBecomeNaN {
  MLNCoordinateReferenceSystem *crs = [[MLNCoordinateReferenceSystem alloc]
      initWithName:@"test:holes"
      tileMatrixSet:(MLNTileMatrixSet){{-180, 180}, 360}
      forward:^MLNCRSPosition(CLLocationCoordinate2D coordinate) {
        if (coordinate.latitude > 80) {
          [NSException raise:NSInvalidArgumentException format:@"No position north of 80 degrees"];
        }
        if (coordinate.latitude < -80) {
          return {NAN, NAN};
        }
        return {coordinate.longitude, coordinate.latitude};
      }
      inverse:^CLLocationCoordinate2D(MLNCRSPosition position) {
        if (position.x > 170) {
          return kCLLocationCoordinate2DInvalid;
        }
        if (position.x < -170) {
          [NSException raise:NSInvalidArgumentException format:@"No coordinate west of -170"];
        }
        return CLLocationCoordinate2DMake(position.y, position.x);
      }];
  mln::CRS core = crs.mbglCRS;

  std::vector<std::array<double, 2>> lngLats = {{10, 20}, {10, 85}, {10, -85}};
  std::vector<std::array<double, 2>> positions(lngLats.size());
  core.forward(lngLats, positions);
  XCTAssertEqual(positions[0][0], 10);
  XCTAssertEqual(positions[0][1], 20);
  XCTAssertTrue(std::isnan(positions[1][0]) && std::isnan(positions[1][1]));
  XCTAssertTrue(std::isnan(positions[2][0]) && std::isnan(positions[2][1]));

  std::vector<std::array<double, 2>> world = {{30, 40}, {175, 0}, {-175, 0}};
  std::vector<std::array<double, 2>> back(world.size());
  core.inverse(world, back);
  XCTAssertEqual(back[0][0], 30);
  XCTAssertEqual(back[0][1], 40);
  XCTAssertTrue(std::isnan(back[1][0]) && std::isnan(back[1][1]));
  XCTAssertTrue(std::isnan(back[2][0]) && std::isnan(back[2][1]));
}

- (void)testAnInverseMayAnswerALongitudePastTheAntimeridian {
  MLNCoordinateReferenceSystem *crs =
      [[MLNCoordinateReferenceSystem alloc] initWithName:@"test:far-west"
          tileMatrixSet:(MLNTileMatrixSet){{-180, 180}, 360}
          forward:^MLNCRSPosition(CLLocationCoordinate2D coordinate) {
            return {coordinate.longitude, coordinate.latitude};
          }
          inverse:^CLLocationCoordinate2D(__unused MLNCRSPosition position) {
            return CLLocationCoordinate2DMake(80, -200);
          }];

  std::vector<std::array<double, 2>> world = {{0, 0}};
  std::vector<std::array<double, 2>> back(1);
  crs.mbglCRS.inverse(world, back);
  XCTAssertEqual(back[0][0], -200);
  XCTAssertEqual(back[0][1], 80);
}

@end
