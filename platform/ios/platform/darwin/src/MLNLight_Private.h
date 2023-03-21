#import <Foundation/Foundation.h>

#import "MLNLight.h"

namespace mbgl {
    namespace style {
        class Light;
    }
}

@interface MLNLight (Private)

/**
 Initializes and returns a `MLNLight` associated with a style's light.
 */
- (instancetype)initWithMBGLLight:(const mbgl::style::Light *)mbglLight;

/**
 Returns an `mbgl::style::Light` representation of the `MLNLight`.
 */
- (mbgl::style::Light)mbglLight;

@end
