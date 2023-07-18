#import "MLNFoundation.h"
#import "MLNShapeSource.h"
#include <mbgl/util/immutable.hpp>

NS_ASSUME_NONNULL_BEGIN

namespace mbgl {
    namespace style {
        struct GeoJSONOptions;
    }
}

MLN_EXPORT
mbgl::Immutable<mbgl::style::GeoJSONOptions> MLNGeoJSONOptionsFromDictionary(NSDictionary<MLNShapeSourceOption, id> *options);

@interface MLNShapeSource (Private)

/**
 :nodoc:
 Debug log showing structure of an `MLNFeature`. This method recurses in the case
 that the feature conforms to `MLNCluster`. This method is used for testing and
 should be considered experimental, likely to be removed or changed in future
 releases.
 
 @param feature An object that conforms to the `MLNFeature` protocol.
 @param indent Used during recursion. Specify 0.
 */

- (void)debugRecursiveLogForFeature:(id<MLNFeature>)feature indent:(NSUInteger)indent;
@end

NS_ASSUME_NONNULL_END
