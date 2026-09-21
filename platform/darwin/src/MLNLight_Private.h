#import <Foundation/Foundation.h>

#import "MLNLight.h"

namespace mln {
namespace style {
class Light;
}
}  // namespace mln

@interface MLNLight (Private)

/**
 Initializes and returns a ``MLNLight`` associated with a style's light.
 */
- (instancetype)initWithMBGLLight:(const mln::style::Light *)mbglLight;

/**
 Returns an `mln::style::Light` representation of the ``MLNLight``.
 */
- (mln::style::Light)mbglLight;

@end
