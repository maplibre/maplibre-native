#import "NSComparisonPredicate+MGLAdditions.h"

#import "MGLStyleValue_Private.h"

#import "NSPredicate+MGLAdditions.h"
#import "NSExpression+MGLPrivateAdditions.h"

@implementation NSComparisonPredicate (MGLAdditions)

- (NSString *)mgl_keyPath {
    NSExpression *leftExpression = self.leftExpression;
    NSExpression *rightExpression = self.rightExpression;
    NSExpressionType leftType = leftExpression.expressionType;
    NSExpressionType rightType = rightExpression.expressionType;
    if (leftType == NSKeyPathExpressionType && rightType == NSConstantValueExpressionType) {
        return leftExpression.keyPath;
    } else if (leftType == NSConstantValueExpressionType && rightType == NSKeyPathExpressionType) {
        return rightExpression.keyPath;
    }

    [NSException raise:NSInvalidArgumentException
                format:@"Comparison predicate must compare an attribute (as a key path) to a constant or vice versa."];
    return nil;
}

- (mbgl::Value)mgl_constantValue {
    NSExpression *leftExpression = self.leftExpression;
    NSExpression *rightExpression = self.rightExpression;
    NSExpressionType leftType = leftExpression.expressionType;
    NSExpressionType rightType = rightExpression.expressionType;
    mbgl::Value value;
    if (leftType == NSKeyPathExpressionType && rightType == NSConstantValueExpressionType) {
        value = rightExpression.mgl_constantMBGLValue;
    } else if (leftType == NSConstantValueExpressionType && rightType == NSKeyPathExpressionType) {
        value = leftExpression.mgl_constantMBGLValue;
    } else {
        [NSException raise:NSInvalidArgumentException
                    format:@"Comparison predicate must compare an attribute (as a key path) to a constant or vice versa."];
    }
    return value;
}

- (mbgl::FeatureType)mgl_featureType {
    NSExpression *leftExpression = self.leftExpression;
    NSExpression *rightExpression = self.rightExpression;
    NSExpressionType leftType = leftExpression.expressionType;
    NSExpressionType rightType = rightExpression.expressionType;
    mbgl::FeatureType type;
    if (leftType == NSKeyPathExpressionType && rightType == NSConstantValueExpressionType) {
        type = rightExpression.mgl_featureType;
    } else if (leftType == NSConstantValueExpressionType && rightType == NSKeyPathExpressionType) {
        type = leftExpression.mgl_featureType;
    } else {
        [NSException raise:NSInvalidArgumentException
                    format:@"Comparison predicate must compare an attribute (as a key path) to a constant or vice versa."];
    }
    return type;
}

- (mbgl::FeatureIdentifier)mgl_featureIdentifier {
    NSExpression *leftExpression = self.leftExpression;
    NSExpression *rightExpression = self.rightExpression;
    NSExpressionType leftType = leftExpression.expressionType;
    NSExpressionType rightType = rightExpression.expressionType;
    mbgl::FeatureIdentifier identifier;
    if (leftType == NSKeyPathExpressionType && rightType == NSConstantValueExpressionType) {
        identifier = rightExpression.mgl_featureIdentifier;
    } else if (leftType == NSConstantValueExpressionType && rightType == NSKeyPathExpressionType) {
        identifier = leftExpression.mgl_featureIdentifier;
    } else {
        [NSException raise:NSInvalidArgumentException
                    format:@"Comparison predicate must compare an attribute (as a key path) to a constant or vice versa."];
    }
    return identifier;
}

@end

@implementation NSComparisonPredicate (MGLExpressionAdditions)

- (id)mgl_jsonExpressionObject {
    NSString *op;
    switch (self.predicateOperatorType) {
        case NSLessThanPredicateOperatorType:
            op = @"<";
            break;
        case NSLessThanOrEqualToPredicateOperatorType:
            op = @"<=";
            break;
        case NSGreaterThanPredicateOperatorType:
            op = @">";
            break;
        case NSGreaterThanOrEqualToPredicateOperatorType:
            op = @">=";
            break;
        case NSEqualToPredicateOperatorType:
            op = @"==";
            break;
        case NSNotEqualToPredicateOperatorType:
            op = @"!=";
            break;
        case NSBetweenPredicateOperatorType: {
            op = @"all";
            NSArray *limits = self.rightExpression.constantValue;
            NSPredicate *leftHandPredicate = [NSComparisonPredicate predicateWithLeftExpression:limits[0]
                                                                                rightExpression:self.leftExpression
                                                                                       modifier:NSDirectPredicateModifier
                                                                                           type:NSLessThanOrEqualToPredicateOperatorType
                                                                                        options:0];
            NSPredicate *rightHandPredicate = [NSComparisonPredicate predicateWithLeftExpression:self.leftExpression
                                                                                 rightExpression:limits[1]
                                                                                        modifier:NSDirectPredicateModifier
                                                                                            type:NSLessThanOrEqualToPredicateOperatorType
                                                                                         options:0];
            return @[op, leftHandPredicate.mgl_jsonExpressionObject, rightHandPredicate.mgl_jsonExpressionObject];
        }
        case NSInPredicateOperatorType: {
            // An “in” expression comparing two string literals is unfortunately
            // misinterpreted as a legacy “in” filter due to ambiguity. Wrap one
            // argument in a “literal” expression to force an expression.
            // https://github.com/mapbox/mapbox-gl-js/issues/9373#issuecomment-594537077
            if (self.leftExpression.expressionType == NSConstantValueExpressionType &&
                self.rightExpression.expressionType == NSConstantValueExpressionType &&
                [self.leftExpression.constantValue isKindOfClass:[NSString class]] &&
                [self.rightExpression.constantValue isKindOfClass:[NSString class]]) {
                NSExpression *rightLiteralExpression = [NSExpression expressionWithFormat:@"MGL_FUNCTION('literal', %@)", self.rightExpression];
                NSPredicate *literalPredicate = [NSComparisonPredicate predicateWithLeftExpression:self.leftExpression
                                                                                   rightExpression:rightLiteralExpression
                                                                                          modifier:NSDirectPredicateModifier
                                                                                              type:NSInPredicateOperatorType
                                                                                           options:self.options];
                return literalPredicate.mgl_jsonExpressionObject;
            }
            op = @"in";
            break;
        }
        case NSContainsPredicateOperatorType: {
            NSPredicate *inPredicate = [NSComparisonPredicate predicateWithLeftExpression:self.rightExpression
                                                                          rightExpression:self.leftExpression
                                                                                 modifier:self.comparisonPredicateModifier
                                                                                     type:NSInPredicateOperatorType
                                                                                  options:self.options];
            return inPredicate.mgl_jsonExpressionObject;
        }
        case NSMatchesPredicateOperatorType:
        case NSLikePredicateOperatorType:
        case NSBeginsWithPredicateOperatorType:
        case NSEndsWithPredicateOperatorType:
        case NSCustomSelectorPredicateOperatorType:
            [NSException raise:NSInvalidArgumentException
                        format:@"NSPredicateOperatorType:%lu is not supported.", (unsigned long)self.predicateOperatorType];
    }
    if (!op) {
        return nil;
    }
    NSArray *comparisonArray = @[op, self.leftExpression.mgl_jsonExpressionObject, self.rightExpression.mgl_jsonExpressionObject];
    if (self.options) {
        NSDictionary *collatorObject = @{
            @"case-sensitive": @(!(self.options & NSCaseInsensitivePredicateOption)),
            @"diacritic-sensitive": @(!(self.options & NSDiacriticInsensitivePredicateOption)),
        };
        return [comparisonArray arrayByAddingObject:@[@"collator", collatorObject]];
    }
    return comparisonArray;
}

@end
