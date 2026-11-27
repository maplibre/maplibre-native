#import "MBXNationalGridProjection.h"

#include <mln/test/national_grid.hpp>

#include <array>
#include <memory>

MLNCoordinateReferenceSystem *MBXNationalGridProjection(void) {
  auto crs = std::make_shared<const mln::CRS>(mln::test::nationalGrid("EPSG:27700"));
  MLNTileMatrixSet tileMatrixSet = {{crs->tileMatrixSet.origin[0], crs->tileMatrixSet.origin[1]},
                                    crs->tileMatrixSet.extentAtZoom0};
  return [[MLNCoordinateReferenceSystem alloc] initWithName:@(crs->name.c_str())
      tileMatrixSet:tileMatrixSet
      forward:^MLNCRSPosition(CLLocationCoordinate2D coordinate) {
        std::array<double, 2> lngLat{coordinate.longitude, coordinate.latitude};
        std::array<double, 2> position{};
        crs->forward({&lngLat, 1}, {&position, 1});
        return {position[0], position[1]};
      }
      inverse:^CLLocationCoordinate2D(MLNCRSPosition crsPosition) {
        std::array<double, 2> position{crsPosition.x, crsPosition.y};
        std::array<double, 2> lngLat{};
        crs->inverse({&position, 1}, {&lngLat, 1});
        return CLLocationCoordinate2DMake(lngLat[1], lngLat[0]);
      }];
}

NSURL *MBXNationalGridCrosshairsStyleURL(NSBundle *bundle) {
  NSURL *fixtureURL = [bundle URLForResource:@"national_grid_crosshairs" withExtension:@"json"];
  NSData *data = fixtureURL ? [NSData dataWithContentsOfURL:fixtureURL] : nil;
  NSMutableDictionary *style =
      data ? [NSJSONSerialization JSONObjectWithData:data
                                             options:NSJSONReadingMutableContainers
                                               error:nil]
           : nil;
  if (![style isKindOfClass:[NSMutableDictionary class]]) {
    return nil;
  }

  // The render-test runner serves `local://tiles/` from metrics/integration/tiles, which the bundle
  // carries at its root.
  style[@"name"] = @"National Grid crosshairs (EPSG:27700)";
  for (NSMutableDictionary *source in [style[@"sources"] allValues]) {
    NSMutableArray *tiles = source[@"tiles"];
    for (NSUInteger i = 0; i < tiles.count; i++) {
      tiles[i] = [tiles[i] stringByReplacingOccurrencesOfString:@"local://tiles/"
                                                     withString:bundle.resourceURL.absoluteString];
    }
  }

  NSURL *styleURL =
      [NSURL fileURLWithPath:[NSTemporaryDirectory()
                                 stringByAppendingPathComponent:@"national_grid.json"]];
  NSData *json = [NSJSONSerialization dataWithJSONObject:style options:0 error:nil];
  return [json writeToURL:styleURL atomically:YES] ? styleURL : nil;
}
