#import "MLNFoundation.h"

NS_ASSUME_NONNULL_BEGIN

/** Options for ``MLNAttributedExpression/attributes``. */
typedef NSString *MLNAttributedExpressionKey NS_TYPED_ENUM;

/** The font name string array expression used to format the text. */
FOUNDATION_EXTERN MLN_EXPORT MLNAttributedExpressionKey const MLNFontNamesAttribute;

/** The font scale number expression relative to ``MLNSymbolStyleLayer/textFontSize`` used to format
 * the text. */
FOUNDATION_EXTERN MLN_EXPORT MLNAttributedExpressionKey const MLNFontScaleAttribute;

/** The font color expression used to format the text. */
FOUNDATION_EXTERN MLN_EXPORT MLNAttributedExpressionKey const MLNFontColorAttribute;

/**
 An ``MLNAttributedExpression`` object associates text formatting attibutes (such as font size or
 font names) to an `NSExpression`.

 ### Example
 ```swift
 let redColor = UIColor.red
 let expression = NSExpression(forConstantValue: "Foo")
 let attributes: [MLNAttributedExpressionKey: NSExpression] = [.fontNamesAttribute :
 NSExpression(forConstantValue: ["DIN Offc Pro Italic", "Arial Unicode MS Regular"]),
                                                               .fontScaleAttribute:
 NSExpression(forConstantValue: 1.2), .fontColorAttribute: NSExpression(forConstantValue: redColor)]
 let attributedExpression = MLNAttributedExpression(expression, attributes:attributes)
 ```

 */
MLN_EXPORT
@interface MLNAttributedExpression : NSObject

/**
 The expression content of the receiver as `NSExpression`.
 */
@property (strong, nonatomic) NSExpression *expression;

#if TARGET_OS_IPHONE
/**
 The formatting attributes dictionary.
 Key | Value Type
 --- | ---
 ``MLNFontNamesAttribute`` | An `NSExpression` evaluating to an `NSString` array.
 ``MLNFontScaleAttribute`` | An `NSExpression` evaluating to an `NSNumber` value.
 ``MLNFontColorAttribute`` | An `NSExpression` evaluating to an `UIColor`.

 */
@property (strong, nonatomic, readonly)
    NSDictionary<MLNAttributedExpressionKey, NSExpression *> *attributes;
#else
/**
 The formatting attributes dictionary.
 Key | Value Type
 --- | ---
 ``MLNFontNamesAttribute`` | An `NSExpression` evaluating to an `NSString` array.
 ``MLNFontScaleAttribute`` | An `NSExpression` evaluating to an `NSNumber` value.
 ``MLNFontColorAttribute`` | An `NSExpression` evaluating to an `NSColor` on macos.
 */
@property (strong, nonatomic, readonly)
    NSDictionary<MLNAttributedExpressionKey, NSExpression *> *attributes;
#endif

/**
 Returns an ``MLNAttributedExpression`` object initialized with an expression and no attribute
 information.
 */
- (instancetype)initWithExpression:(NSExpression *)expression;

/**
 Returns an ``MLNAttributedExpression`` object initialized with an expression and text format
 attributes.
 */
- (instancetype)
    initWithExpression:(NSExpression *)expression
            attributes:(nonnull NSDictionary<MLNAttributedExpressionKey, NSExpression *> *)attrs;

/**
 Creates an ``MLNAttributedExpression`` object initialized with an expression and the format
 attributes for font names and font size.
 */
+ (instancetype)attributedExpression:(NSExpression *)expression
                           fontNames:(nullable NSArray<NSString *> *)fontNames
                           fontScale:(nullable NSNumber *)fontScale;

/**
 Creates an ``MLNAttributedExpression`` object initialized with an expression and the format
 attributes dictionary.
 */
+ (instancetype)
    attributedExpression:(NSExpression *)expression
              attributes:(nonnull NSDictionary<MLNAttributedExpressionKey, NSExpression *> *)attrs;

@end

NS_ASSUME_NONNULL_END
