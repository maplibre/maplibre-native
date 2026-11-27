#import "MLNCoordinateReferenceSystem.h"

#include <mln/util/crs.hpp>

NS_ASSUME_NONNULL_BEGIN

@interface MLNCoordinateReferenceSystem (Private)

- (mln::CRS)mbglCRS;

@end

NS_ASSUME_NONNULL_END
