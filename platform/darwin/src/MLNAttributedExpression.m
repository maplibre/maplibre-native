#import "MLNAttributedExpression.h"
#import "MLNLoggingConfiguration_Private.h"

const MLNAttributedExpressionKey MLNFontNamesAttribute = @"text-font";
const MLNAttributedExpressionKey MLNFontScaleAttribute = @"font-scale";
const MLNAttributedExpressionKey MLNFontColorAttribute = @"text-color";

@implementation MLNAttributedExpression

- (instancetype)initWithExpression:(NSExpression *)expression {
    self = [self initWithExpression:expression attributes:@{}];
    return self;
}

+ (instancetype)attributedExpression:(NSExpression *)expression fontNames:(nullable NSArray<NSString *> *)fontNames fontScale:(nullable NSNumber *)fontScale {
    MLNAttributedExpression *attributedExpression;
    
    NSMutableDictionary *attrs = [NSMutableDictionary dictionary];
    
    if (fontNames && fontNames.count > 0) {
        attrs[MLNFontNamesAttribute] = [NSExpression expressionForConstantValue:fontNames];
    }
    
    if (fontScale) {
        attrs[MLNFontScaleAttribute] = [NSExpression expressionForConstantValue:fontScale];
    }
    
    attributedExpression = [[self alloc] initWithExpression:expression attributes:attrs];
    return attributedExpression;
}

+ (instancetype)attributedExpression:(NSExpression *)expression attributes:(nonnull NSDictionary<MLNAttributedExpressionKey, NSExpression *> *)attrs {
    MLNAttributedExpression *attributedExpression;
    
    attributedExpression = [[self alloc] initWithExpression:expression attributes:attrs];
    
    return attributedExpression;
}

- (instancetype)initWithExpression:(NSExpression *)expression attributes:(nonnull NSDictionary<MLNAttributedExpressionKey, NSExpression *> *)attrs {
    if (self = [super init])
    {
        MLNLogInfo(@"Starting %@ initialization.", NSStringFromClass([self class]));
        _expression = expression;
        _attributes = attrs;
        
        MLNLogInfo(@"Finalizing %@ initialization.", NSStringFromClass([self class]));
    }
    return self;
}

- (BOOL)isEqual:(id)object {
    BOOL result = NO;
    
    if ([object isKindOfClass:[self class]]) {
        MLNAttributedExpression *otherObject = object;
        result = [self.expression isEqual:otherObject.expression] &&
        [_attributes isEqual:otherObject.attributes];
    }
    
    return result;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"MLNAttributedExpression<Expression: %@ Attributes: %@>", self.expression, self.attributes];
}

@end
