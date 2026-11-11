#import <Foundation/Foundation.h>

#import "MLNSky.h"

namespace mln {
namespace style {
class Sky;
}
}  // namespace mln

@interface MLNSky (Private)

/** Initializes a public sky configuration from a core sky snapshot. */
- (instancetype)initWithMBGLSky:(const mln::style::Sky *)mbglSky;

/** Returns the core representation of this sky configuration. */
- (mln::style::Sky)mbglSky;

@end
