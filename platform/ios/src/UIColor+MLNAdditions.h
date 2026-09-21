#import <UIKit/UIKit.h>

#include <mln/style/property_value.hpp>
#include <mln/util/color.hpp>

@interface UIColor (MLNAdditions)

- (mln::Color)mgl_color;

- (mln::style::PropertyValue<mln::Color>)mgl_colorPropertyValue;

- (mln::Color)mgl_colorForPremultipliedValue;

+ (UIColor *)mgl_colorWithColor:(mln::Color)color;

@end

@interface NSExpression (MLNColorAdditions)

+ (NSExpression *)mgl_expressionForRGBComponents:(NSArray<NSExpression *> *)components;
+ (NSExpression *)mgl_expressionForRGBAComponents:(NSArray<NSExpression *> *)components;
+ (UIColor *)mgl_colorWithRGBComponents:(NSArray<NSExpression *> *)components;

@end
