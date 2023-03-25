#import "MLNFoundation.h"

#include <mbgl/util/run_loop.hpp>

void MLNInitializeRunLoop();

/* Using a compound statement (GNU Extension, supported by clang) */
#define MLN_OBJC_DYNAMIC_CAST(object, type) \
    ({ \
        __typeof__( object ) temp##__LINE__ = (object); \
        (type *)([temp##__LINE__ isKindOfClass:[type class]] ? temp##__LINE__ : nil); \
    })

#define MLN_OBJC_DYNAMIC_CAST_AS_PROTOCOL(object, proto) \
    ({ \
        __typeof__( object ) temp##__LINE__ = (object); \
        (id< proto >)([temp##__LINE__ conformsToProtocol:@protocol( proto )] ? temp##__LINE__ : nil); \
    })

