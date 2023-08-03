#import <Foundation/Foundation.h>

#import "NSPredicate+MLNAdditions.h"

#include <mbgl/style/filter.hpp>

NS_ASSUME_NONNULL_BEGIN

@interface NSPredicate (MLNPrivateAdditions)

- (mbgl::style::Filter)mgl_filter;

+ (nullable instancetype)mgl_predicateWithFilter:(mbgl::style::Filter)filter;

@end

@interface NSPredicate (MLNExpressionAdditions)

- (nullable id)mgl_if:(id)firstValue, ...;

- (nullable id)mgl_match:(NSExpression *)firstCase, ...;

@end

NS_ASSUME_NONNULL_END
