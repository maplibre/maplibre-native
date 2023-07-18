#import <XCTest/XCTest.h>
#import <Mapbox/Mapbox.h>
#import "MLNTestUtility.h"

#define MLNTestFail(myself, ...) \
_XCTPrimitiveFail(myself, __VA_ARGS__)

#define MLNTestAssert(myself, expression, ...) \
    _XCTPrimitiveAssertTrue(myself, expression, @#expression, __VA_ARGS__)

#define MLNTestAssertEqualWithAccuracy(myself, expression1, expression2, accuracy, ...) \
    _XCTPrimitiveAssertEqualWithAccuracy(myself, expression1, @#expression1, expression2, @#expression2, accuracy, @#accuracy, __VA_ARGS__)
#define MLNTestAssertNil(myself, expression, ...) \
    _XCTPrimitiveAssertNil(myself, expression, @#expression, __VA_ARGS__)

#define MLNTestAssertNotNil(myself, expression, ...) \
    _XCTPrimitiveAssertNotNil(myself, expression, @#expression, __VA_ARGS__)

#define MLNTestWarning(expression, format, ...) \
    ({ \
        if (!(expression))  { \
            NSString *message = [NSString stringWithFormat:format, ##__VA_ARGS__]; \
            printf("warning: Test Case '%s' at line %d: '%s' %s\n", __PRETTY_FUNCTION__, __LINE__, #expression, message.UTF8String); \
        } \
    })

@interface MLNIntegrationTestCase: XCTestCase
@end
