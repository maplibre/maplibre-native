#import "MLNFoundation.h"
#import "MLNTypes.h"
#import "MLNComputedShapeSource.h"

#include <mbgl/style/sources/custom_geometry_source.hpp>

NS_ASSUME_NONNULL_BEGIN

MLN_EXPORT
mbgl::style::CustomGeometrySource::Options MBGLCustomGeometrySourceOptionsFromDictionary(NSDictionary<MLNShapeSourceOption, id> *options);

NS_ASSUME_NONNULL_END
