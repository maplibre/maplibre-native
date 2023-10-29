#import "MLNVectorStyleLayer.h"
#import "MLNStyleLayer_Private.h"

@implementation MLNVectorStyleLayer

- (void)setPredicate:(NSPredicate *)predicate {
    [NSException raise:MLNAbstractClassException
                format:@"MLNVectorStyleLayer is an abstract class"];
}

- (NSPredicate *)predicate {
    [NSException raise:MLNAbstractClassException
                format:@"MLNVectorStyleLayer is an abstract class"];
    return nil;
}

- (NSString *)description {    
    if (self.rawLayer) {
        return [NSString stringWithFormat:
                @"<%@: %p; identifier = %@; sourceIdentifier = %@; "
                @"sourceLayerIdentifier = %@; predicate = %@; visible = %@>",
                NSStringFromClass([self class]), (void *)self, self.identifier,
                self.sourceIdentifier, self.sourceLayerIdentifier, self.predicate,
                self.visible ? @"YES" : @"NO"];
    }
    else {
        return [NSString stringWithFormat:
                @"<%@: %p; identifier = %@; sourceIdentifier = <unknown>; "
                @"sourceLayerIdentifier = <unknown>; predicate = <unknown>; visible = <unknown>>",
                NSStringFromClass([self class]), (void *)self, self.identifier];
    }
}

@end
