#import <Foundation/Foundation.h>
#import "MGLEvent.h"
#include <mbgl/util/observable.hpp>

NS_ASSUME_NONNULL_BEGIN

@interface MGLEvent ()
- (instancetype)initWithEvent:(const mbgl::ObservableEvent&)event;
@end

NS_ASSUME_NONNULL_END