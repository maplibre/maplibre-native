#import <Foundation/Foundation.h>

#import "NSPredicate+MLNAdditions.h"

#include <mln/style/filter.hpp>

NS_ASSUME_NONNULL_BEGIN

@interface NSPredicate (MLNPrivateAdditions)

- (mln::style::Filter)mgl_filter;

+ (nullable instancetype)mgl_predicateWithFilter:(mln::style::Filter)filter;

@end

@interface NSPredicate (MLNExpressionAdditions)

- (nullable id)mgl_if:(id)firstValue, ...;

- (nullable id)mgl_match:(NSExpression *)firstCase, ...;

@end

NS_ASSUME_NONNULL_END
