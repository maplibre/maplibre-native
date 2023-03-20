#import "NSCompoundPredicate+MLNAdditions.h"

#import "MLNStyleValue_Private.h"

#import "NSPredicate+MLNPrivateAdditions.h"
#import "NSExpression+MLNPrivateAdditions.h"
#import "MLNLoggingConfiguration_Private.h"

#include <mbgl/style/conversion/property_value.hpp>

@implementation NSCompoundPredicate (MLNAdditions)

- (std::vector<mbgl::style::Filter>)mgl_subfilters
{
    std::vector<mbgl::style::Filter>filters;
    for (NSPredicate *predicate in self.subpredicates) {
        filters.push_back(predicate.mgl_filter);
    }
    return filters;
}

@end

@implementation NSCompoundPredicate (MLNExpressionAdditions)

- (id)mgl_jsonExpressionObject {
    switch (self.compoundPredicateType) {
        case NSNotPredicateType: {
            MLNAssert(self.subpredicates.count <= 1, @"NOT predicate cannot have multiple subpredicates.");
            NSPredicate *subpredicate = self.subpredicates.firstObject;
            return @[@"!", subpredicate.mgl_jsonExpressionObject];
        }
            
        case NSAndPredicateType: {
            NSArray *subarrays = [self.subpredicates valueForKeyPath:@"mgl_jsonExpressionObject"];
            return [@[@"all"] arrayByAddingObjectsFromArray:subarrays];
        }
            
        case NSOrPredicateType: {
            NSArray *subarrays = [self.subpredicates valueForKeyPath:@"mgl_jsonExpressionObject"];
            return [@[@"any"] arrayByAddingObjectsFromArray:subarrays];
        }
    }
    
    [NSException raise:@"Compound predicate type not handled"
                format:@""];
    return nil;
}

@end
