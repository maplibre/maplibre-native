#import <XCTest/XCTest.h>
#import <Mapbox/Mapbox.h>
#import "MGLTestUtility.h"

#define MGLTestFail(myself, ...) \
_XCTPrimitiveFail(myself, __VA_ARGS__)

#define MGLTestAssert(myself, expression, ...) \
    _XCTPrimitiveAssertTrue(myself, expression, @#expression, __VA_ARGS__)

#define MGLTestAssertEqualWithAccuracy(myself, expression1, expression2, accuracy, ...) \
    _XCTPrimitiveAssertEqualWithAccuracy(myself, expression1, @#expression1, expression2, @#expression2, accuracy, @#accuracy, __VA_ARGS__)
#define MGLTestAssertNil(myself, expression, ...) \
    _XCTPrimitiveAssertNil(myself, expression, @#expression, __VA_ARGS__)

#define MGLTestAssertNotNil(myself, expression, ...) \
    _XCTPrimitiveAssertNotNil(myself, expression, @#expression, __VA_ARGS__)

#define MGLTestWarning(expression, format, ...) \
    ({ \
        if (!(expression))  { \
            NSString *message = [NSString stringWithFormat:format, ##__VA_ARGS__]; \
            printf("warning: Test Case '%s' at line %d: '%s' %s\n", __PRETTY_FUNCTION__, __LINE__, #expression, message.UTF8String); \
        } \
    })

@interface MGLIntegrationTestCase: XCTestCase
@end
