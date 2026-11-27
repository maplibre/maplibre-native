#import "MLNCoordinateReferenceSystem_Private.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <span>

@implementation MLNCoordinateReferenceSystem

- (instancetype)initWithName:(NSString *)name
               tileMatrixSet:(MLNTileMatrixSet)tileMatrixSet
                     forward:(MLNCRSPosition (^)(CLLocationCoordinate2D coordinate))forward
                     inverse:(CLLocationCoordinate2D (^)(MLNCRSPosition position))inverse {
  if (self = [super init]) {
    _name = [name copy];
    _tileMatrixSet = tileMatrixSet;
    _forward = [forward copy];
    _inverse = [inverse copy];
  }
  return self;
}

- (mln::CRS)mbglCRS {
  MLNCRSPosition (^forward)(CLLocationCoordinate2D) = self.forward;
  CLLocationCoordinate2D (^inverse)(MLNCRSPosition) = self.inverse;
  return {
      .name = self.name.UTF8String,
      .forward =
          [forward](std::span<const std::array<double, 2>> lngLats,
                    std::span<std::array<double, 2>> positions) {
            @autoreleasepool {
              for (std::size_t i = 0; i < lngLats.size(); ++i) {
                MLNCRSPosition position = {NAN, NAN};
                @try {
                  position = forward(CLLocationCoordinate2DMake(lngLats[i][1], lngLats[i][0]));
                } @catch (NSException *) {
                }
                positions[i] = {position.x, position.y};
              }
            }
          },
      .inverse =
          [inverse](std::span<const std::array<double, 2>> positions,
                    std::span<std::array<double, 2>> lngLats) {
            @autoreleasepool {
              for (std::size_t i = 0; i < positions.size(); ++i) {
                CLLocationCoordinate2D coordinate = kCLLocationCoordinate2DInvalid;
                @try {
                  coordinate = inverse({positions[i][0], positions[i][1]});
                } @catch (NSException *) {
                }
                if (std::abs(coordinate.latitude) <= 90 && std::isfinite(coordinate.longitude)) {
                  lngLats[i] = {coordinate.longitude, coordinate.latitude};
                } else {
                  lngLats[i] = {NAN, NAN};
                }
              }
            }
          },
      .tileMatrixSet = {.origin = {self.tileMatrixSet.origin.x, self.tileMatrixSet.origin.y},
                        .extentAtZoom0 = self.tileMatrixSet.extentAtZoom0},
  };
}

@end
