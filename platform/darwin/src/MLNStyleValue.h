#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>

#import "MLNFoundation.h"
#import "MLNTypes.h"

NS_ASSUME_NONNULL_BEGIN

typedef NSString *MLNStyleFunctionOption NS_STRING_ENUM NS_UNAVAILABLE;

FOUNDATION_EXTERN MLN_EXPORT const MLNStyleFunctionOption MLNStyleFunctionOptionInterpolationBase
    __attribute__((unavailable(
        "Use NSExpression instead, applying the mgl_interpolate:withCurveType:parameters:stops: "
        "function with a curve type of “exponential” and a non-nil parameter.")));

FOUNDATION_EXTERN MLN_EXPORT const MLNStyleFunctionOption MLNStyleFunctionOptionDefaultValue
    __attribute__((unavailable(
        "Use +[NSExpression expressionForConditional:trueExpression:falseExpression:] instead.")));

typedef NS_ENUM(NSUInteger, MLNInterpolationMode) {
  MLNInterpolationModeExponential __attribute__((unavailable(
      "Use NSExpression instead, applying the mgl_interpolate:withCurveType:parameters:stops: "
      "function with a curve type of “exponential”."))) = 0,
  MLNInterpolationModeInterval __attribute__((
      unavailable("Use NSExpression instead, calling the mgl_step:from:stops: function."))),
  MLNInterpolationModeCategorical __attribute__((unavailable("Use NSExpression instead."))),
  MLNInterpolationModeIdentity
  __attribute__((unavailable("Use +[NSExpression expressionForKeyPath:] instead.")))
} __attribute__((unavailable("Use NSExpression instead.")));

MLN_EXPORT __attribute__((unavailable("Use NSExpression instead.")))
@interface MLNStyleValue<T> : NSObject
@end

MLN_EXPORT __attribute__((unavailable("Use +[NSExpression expressionForConstantValue:] instead.")))
@interface MLNConstantStyleValue<T> : MLNStyleValue<T>
@end

@compatibility_alias MLNStyleConstantValue MLNConstantStyleValue;

MLN_EXPORT
__attribute__((unavailable("Use NSExpression instead, calling the mgl_step:from:stops: or "
                           "mgl_interpolate:withCurveType:parameters:stops: function.")))
@interface MLNStyleFunction<T> : MLNStyleValue<T>
@end

MLN_EXPORT __attribute__((unavailable(
    "Use NSExpression instead, applying the mgl_step:from:stops: or "
    "mgl_interpolate:withCurveType:parameters:stops: function to the $zoomLevel variable.")))
@interface MLNCameraStyleFunction<T> : MLNStyleFunction<T>
@end

MLN_EXPORT __attribute__((unavailable(
    "Use NSExpression instead, applying the mgl_step:from:stops: or "
    "mgl_interpolate:withCurveType:parameters:stops: function to a key path expression.")))
@interface MLNSourceStyleFunction<T> : MLNStyleFunction<T>
@end

MLN_EXPORT
__attribute__((unavailable("Use a NSExpression instead with nested mgl_step:from:stops: or "
                           "mgl_interpolate:withCurveType:parameters:stops: function calls.")))
@interface MLNCompositeStyleFunction<T> : MLNStyleFunction<T>
@end

NS_ASSUME_NONNULL_END
