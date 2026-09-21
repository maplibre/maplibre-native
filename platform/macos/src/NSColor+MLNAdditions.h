#import <Cocoa/Cocoa.h>

#include <mln/style/property_value.hpp>
#include <mln/util/color.hpp>

@interface NSColor (MLNAdditions)

/**
 Converts the color into an mln::Color in sRGB space.
 */
- (mln::Color)mgl_color;

- (mln::Color)mgl_colorForPremultipliedValue;

/**
 Instantiates `NSColor` from an `mln::Color`
 */
+ (NSColor *)mgl_colorWithColor:(mln::Color)color;

- (mln::style::PropertyValue<mln::Color>)mgl_colorPropertyValue;

@end

@interface NSExpression (MLNColorAdditions)

+ (NSExpression *)mgl_expressionForRGBComponents:(NSArray<NSExpression *> *)components;
+ (NSExpression *)mgl_expressionForRGBAComponents:(NSArray<NSExpression *> *)components;
+ (NSColor *)mgl_colorWithRGBComponents:(NSArray<NSExpression *> *)componentExpressions;

@end
