#import <Mapbox.h>
#import <XCTest/XCTest.h>

#define MLNConstantExpression(constant) \
    [NSExpression expressionForConstantValue:constant]

@interface MLNStyleLayerTests : XCTestCase <MLNMapViewDelegate>

@property (nonatomic, copy, readonly, class) NSString *layerType;

- (void)testPropertyName:(NSString *)name isBoolean:(BOOL)isBoolean;

@end

@interface NSString (MLNStyleLayerTestAdditions)

@property (nonatomic, readonly, copy) NSArray<NSString *> *lexicalClasses;
@property (nonatomic, readonly, copy) NSString *lemma;

@end

@interface NSValue (MLNStyleLayerTestAdditions)

+ (instancetype)valueWithMLNVector:(CGVector)vector;

@property (readonly) CGVector MLNVectorValue;

@end
