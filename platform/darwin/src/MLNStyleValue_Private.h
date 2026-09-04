#import <Foundation/Foundation.h>

#import "MLNStyleValue.h"

#import "MLNTypes.h"
#import "NSDate+MLNAdditions.h"
#import "NSExpression+MLNPrivateAdditions.h"
#import "NSValue+MLNAdditions.h"
#import "NSValue+MLNStyleAttributeAdditions.h"

#include <mln/style/conversion/color_ramp_property_value.hpp>
#include <mln/style/conversion/position.hpp>
#include <mln/style/conversion/property_value.hpp>
#include <mln/style/expression/dsl.hpp>
#import <mln/style/transition_options.hpp>
#import <mln/style/types.hpp>
#import "MLNConversion.h"
#import "MLNSymbolStyleLayer.h"

#import <mln/util/enum.hpp>
#include <mln/util/interpolate.hpp>

#include <memory>

#if TARGET_OS_IPHONE
#import "UIColor+MLNAdditions.h"
#else
#import "NSColor+MLNAdditions.h"
#endif

namespace mln {
namespace style {
namespace expression {
class Expression;
}
}  // namespace style
}  // namespace mln

id MLNJSONObjectFromMBGLValue(const mln::Value &value);

NS_INLINE MLNTransition MLNTransitionFromOptions(const mln::style::TransitionOptions &options) {
  MLNTransition transition;
  transition.duration =
      MLNTimeIntervalFromDuration(options.duration.value_or(mln::Duration::zero()));
  transition.delay = MLNTimeIntervalFromDuration(options.delay.value_or(mln::Duration::zero()));

  return transition;
}

NS_INLINE mln::style::TransitionOptions MLNOptionsFromTransition(MLNTransition transition) {
  mln::style::TransitionOptions options{{MLNDurationFromTimeInterval(transition.duration)},
                                        {MLNDurationFromTimeInterval(transition.delay)}};
  return options;
}

std::unique_ptr<mln::style::expression::Expression> MLNClusterPropertyFromNSExpression(
    NSExpression *expression);

id MLNJSONObjectFromMBGLExpression(const mln::style::expression::Expression &mbglExpression);

template <typename MBGLType, typename ObjCType, typename MBGLElement = MBGLType,
          typename ObjCEnum = ObjCType>
class MLNStyleValueTransformer {
public:
  /// Convert an mbgl property value into an mgl style value
  NSExpression *toExpression(const mln::style::PropertyValue<MBGLType> &mbglValue) {
    PropertyExpressionEvaluator evaluator;
    return mbglValue.evaluate(evaluator);
  }

  // Convert an mbgl heatmap color property value into an mgl style value
  NSExpression *toExpression(const mln::style::ColorRampPropertyValue &mbglValue) {
    if (mbglValue.isUndefined()) {
      return nil;
    }
    return [NSExpression
        expressionWithMLNJSONObject:MLNJSONObjectFromMBGLExpression(mbglValue.getExpression())];
  }

  /**
   Converts an NSExpression to an mbgl property value.
   */
  template <typename MBGLValue>
  typename std::enable_if_t<!std::is_same<MBGLValue, mln::style::ColorRampPropertyValue>::value,
                            MBGLValue>
  toPropertyValue(NSExpression *expression, bool allowDataExpressions) {
    if (!expression) {
      return {};
    }

    if (expression.expressionType == NSConstantValueExpressionType) {
      MBGLType mbglValue;
      getMBGLValue(expression.constantValue, mbglValue);
      return mbglValue;
    }
    if (expression.expressionType == NSAggregateExpressionType) {
      MBGLType mbglValue;
      getMBGLValue(expression.collection, mbglValue);
      return mbglValue;
    }

    NSArray *jsonExpression = expression.mgl_jsonExpressionObject;

    mln::style::conversion::Error valueError;
    auto value = mln::style::conversion::convert<MBGLValue>(
        mln::style::conversion::makeConvertible(jsonExpression), valueError, allowDataExpressions,
        false);
    if (!value) {
      [NSException raise:NSInvalidArgumentException
                  format:@"Invalid property value: %@", @(valueError.message.c_str())];
      return {};
    }

    return *value;
  }

  /**
   Converts an NSExpression to an mbgl property value.
   */
  template <typename MBGLValue>
  typename std::enable_if_t<std::is_same<MBGLValue, mln::style::ColorRampPropertyValue>::value,
                            MBGLValue>
  toPropertyValue(NSExpression *expression) {
    if (!expression) {
      return {};
    }

    NSArray *jsonExpression = expression.mgl_jsonExpressionObject;

    mln::style::conversion::Error valueError;
    auto value = mln::style::conversion::convert<mln::style::ColorRampPropertyValue>(
        mln::style::conversion::makeConvertible(jsonExpression), valueError);
    if (!value) {
      [NSException raise:NSInvalidArgumentException
                  format:@"Invalid property value: %@", @(valueError.message.c_str())];
      return {};
    }

    return *value;
  }

private:  // Private utilities for converting from mgl to mbgl values
  /**
   As hack to allow converting enum => string values, we accept a second, dummy parameter in
   the toRawStyleSpecValue() methods for converting 'atomic' (non-style-function) values.
   This allows us to use `std::enable_if` to test (at compile time) whether or not MBGLType is an
   Enum.
   */
  template <typename MBGLEnum = MBGLType,
            class = typename std::enable_if<!std::is_enum<MBGLEnum>::value>::type,
            typename MLNEnum = ObjCEnum,
            class = typename std::enable_if<!std::is_enum<MLNEnum>::value>::type>
  NSObject *toRawStyleSpecValue(NSObject *rawMLNValue, MBGLEnum &) {
    if ([rawMLNValue isKindOfClass:[NSValue class]]) {
      const auto rawNSValue = (NSValue *)rawMLNValue;
      if (strcmp([rawNSValue objCType], @encode(CGVector)) == 0) {
        // offset [x, y]
        std::array<float, 2> mglValue = rawNSValue.mgl_offsetArrayValue;
        return [NSArray arrayWithObjects:@(mglValue[0]), @(mglValue[1]), nil];
      }
    }
    // noop pass-through plain NSObject-based items
    return rawMLNValue;
  }

  template <typename MBGLEnum = MBGLType,
            class = typename std::enable_if<std::is_enum<MBGLEnum>::value>::type,
            typename MLNEnum = ObjCEnum,
            class = typename std::enable_if<std::is_enum<MLNEnum>::value>::type>
  NSString *toRawStyleSpecValue(ObjCType rawValue, MBGLEnum &) {
    MLNEnum mglEnum;
    [rawValue getValue:&mglEnum];
    return @(mln::Enum<MLNEnum>::toString(mglEnum));
  }

  NSObject *toRawStyleSpecValue(MLNColor *color, MBGLType &) {
    return @(color.mgl_color.stringify().c_str());
  }

  // Bool
  void getMBGLValue(NSNumber *rawValue, bool &mbglValue) { mbglValue = !!rawValue.boolValue; }

  // Float
  void getMBGLValue(NSNumber *rawValue, float &mbglValue) { mbglValue = rawValue.floatValue; }

  // String
  void getMBGLValue(NSString *rawValue, std::string &mbglValue) { mbglValue = rawValue.UTF8String; }

  // Formatted
  void getMBGLValue(NSString *rawValue, mln::style::expression::Formatted &mbglValue) {
    mbglValue = mln::style::expression::Formatted(rawValue.UTF8String);
  }

  // Offsets
  void getMBGLValue(id rawValue, std::array<float, 2> &mbglValue) {
    if ([rawValue isKindOfClass:[NSValue class]]) {
      mbglValue = [rawValue mgl_offsetArrayValue];
    } else if ([rawValue isKindOfClass:[NSArray class]]) {
      NSArray *array = (NSArray *)rawValue;
      getMBGLValue(array[0], mbglValue[0]);
      getMBGLValue(array[1], mbglValue[1]);
    }
  }

  // Padding as array<float, 4>
  void getMBGLValue(id rawValue, std::array<float, 4> &mbglValue) {
    if ([rawValue isKindOfClass:[NSValue class]]) {
      mbglValue = [rawValue mgl_paddingArrayValue];
    } else if ([rawValue isKindOfClass:[NSArray class]]) {
      NSArray *array = (NSArray *)rawValue;
      getMBGLValue(array[0], mbglValue[0]);
      getMBGLValue(array[1], mbglValue[1]);
      getMBGLValue(array[2], mbglValue[2]);
      getMBGLValue(array[3], mbglValue[3]);
      getMBGLValue(array[4], mbglValue[4]);
    }
  }

  // Padding type (supports numbers and float arrays w/ sizes 1 to 4)
  void getMBGLValue(id rawValue, mln::Padding &mbglValue) {
    if ([rawValue isKindOfClass:[NSNumber class]]) {
      NSNumber *number = (NSNumber *)rawValue;
      mbglValue = mln::Padding(number.floatValue);
    } else if ([rawValue isKindOfClass:[NSArray class]]) {
      NSArray *array = (NSArray *)rawValue;
      if (array.count < 1 || array.count > 4) {
        [NSException raise:NSInvalidArgumentException
                    format:@"Padding array should have from 1 to 4 elements."];
      }
      std::array<float, 4> values;
      for (size_t i = 0; i < array.count; ++i) {
        getMBGLValue(array[i], values[i]);
      }
      mbglValue = mln::Padding(std::span<float>(values.begin(), array.count));
    } else if ([rawValue isKindOfClass:[NSValue class]]) {
      mbglValue = mln::Padding([rawValue mgl_paddingArrayValue]);
    }
  }

  // VerticalGradient type (supports Booleans and float arrays w/ sizes 1 to 2).
  // NSNumber covers both cases, so check for the array first; a bare NSNumber is treated as
  // the Boolean form, matching `fill-extrusion-vertical-gradient`'s original type.
  void getMBGLValue(id rawValue, mln::VerticalGradient &mbglValue) {
    if ([rawValue isKindOfClass:[NSArray class]]) {
      NSArray *array = (NSArray *)rawValue;
      if (array.count < 1 || array.count > 2) {
        [NSException raise:NSInvalidArgumentException
                    format:@"Vertical gradient array should have from 1 to 2 elements."];
      }
      std::array<float, 2> values;
      for (size_t i = 0; i < array.count; ++i) {
        getMBGLValue(array[i], values[i]);
      }
      const std::span<const float> span(values.begin(), array.count);
      if (!mln::VerticalGradient::isInRange(span)) {
        [NSException raise:NSInvalidArgumentException
                    format:@"Vertical gradient %s", mln::VerticalGradient::rangeErrorMessage];
      }
      mbglValue = mln::VerticalGradient(span);
    } else if ([rawValue isKindOfClass:[NSNumber class]]) {
      NSNumber *number = (NSNumber *)rawValue;
      mbglValue = mln::VerticalGradient(number.boolValue);
    }
  }

  // Color
  void getMBGLValue(MLNColor *rawValue, mln::Color &mbglValue) { mbglValue = rawValue.mgl_color; }

  // VariableAnchorOffsetCollection
  void getMBGLValue(id rawValue, mln::VariableAnchorOffsetCollection &mbglValue) {
    if ([rawValue isKindOfClass:[NSArray class]]) {
      NSArray *array = (NSArray *)rawValue;
      if (array.count % 2 != 0) {
        [NSException
             raise:NSInvalidArgumentException
            format:@"VariableTextAnchorOffset array should have an even number of elements."];
      }

      std::vector<mln::AnchorOffsetPair> anchorOffsets;
      anchorOffsets.reserve(array.count / 2);
      for (NSUInteger i = 0; i < array.count; i += 2) {
        mln::style::SymbolAnchorType anchor{0};
        getMBGLValue<mln::style::SymbolAnchorType, MLNTextAnchor>(array[i], anchor);

        std::array<float, 2> offsetArray;
        getMBGLValue(array[i + 1], offsetArray);

        anchorOffsets.emplace_back(anchor, offsetArray);
      }

      mbglValue = mln::VariableAnchorOffsetCollection(std::move(anchorOffsets));
    }
  }

  // Image
  void getMBGLValue(NSString *rawValue, mln::style::expression::Image &mbglValue) {
    mbglValue = mln::style::expression::Image(rawValue.UTF8String);
  }

  // Array
  void getMBGLValue(ObjCType rawValue, std::vector<MBGLElement> &mbglValue) {
    mbglValue.reserve(rawValue.count);
    for (id obj in rawValue) {
      id constantObject = obj;
      if ([obj isKindOfClass:[NSExpression class]] &&
          [obj expressionType] == NSConstantValueExpressionType) {
        constantObject = [constantObject constantValue];
      }
      MBGLElement mbglElement;
      getMBGLValue(constantObject, mbglElement);
      mbglValue.push_back(mbglElement);
    }
  }

  void getMBGLValue(NSValue *rawValue, mln::style::Position &mbglValue) {
    auto spherical = rawValue.mgl_lightPositionArrayValue;
    mln::style::Position position(spherical);
    mbglValue = position;
  }

  // Enumerations
  template <typename MBGLEnum = MBGLType, typename MLNEnum = ObjCEnum,
            class = typename std::enable_if<std::is_enum<MBGLEnum>::value>::type,
            class = typename std::enable_if<std::is_enum<MLNEnum>::value>::type>
  void getMBGLValue(id rawValue, MBGLEnum &mbglValue) {
    if ([rawValue isKindOfClass:[NSString class]]) {
      mbglValue = *mln::Enum<MBGLEnum>::toEnum([(NSString *)rawValue UTF8String]);
    } else {
      MLNEnum mglEnum;
      [(NSValue *)rawValue getValue:&mglEnum];
      auto str = mln::Enum<MLNEnum>::toString(mglEnum);
      mbglValue = *mln::Enum<MBGLEnum>::toEnum(str);
    }
  }

private:  // Private utilities for converting from mbgl to mgl values
  // Bool
  static NSNumber *toMLNRawStyleValue(const bool mbglStopValue) { return @(mbglStopValue); }

  // Float
  static NSNumber *toMLNRawStyleValue(const float mbglStopValue) { return @(mbglStopValue); }

  // Integer
  static NSNumber *toMLNRawStyleValue(const int64_t mbglStopValue) { return @(mbglStopValue); }

  // String
  static NSString *toMLNRawStyleValue(const std::string &mbglStopValue) {
    return @(mbglStopValue.c_str());
  }

  // Formatted
  static NSString *toMLNRawStyleValue(const mln::style::expression::Formatted &mbglStopValue) {
    return @(mbglStopValue.toString().c_str());
  }

  // Offsets
  static NSValue *toMLNRawStyleValue(const std::array<float, 2> &mbglStopValue) {
    return [NSValue mgl_valueWithOffsetArray:mbglStopValue];
  }

  // Padding as array<float, 4>
  static NSValue *toMLNRawStyleValue(const std::array<float, 4> &mbglStopValue) {
    return [NSValue mgl_valueWithPaddingArray:mbglStopValue];
  }

  // Padding type
  static NSValue *toMLNRawStyleValue(const mln::Padding &mbglStopValue) {
    return [NSValue mgl_valueWithPaddingArray:mbglStopValue.toArray()];
  }

  // VerticalGradient type. Always reported as the two-element array form rather than a
  // Boolean: `true` round-trips to `[depth, 150]`, which selects the same legacy shading
  // model, and `false` to a zero depth, which the shader treats as a no-op.
  static NSArray<NSNumber *> *toMLNRawStyleValue(const mln::VerticalGradient &mbglStopValue) {
    const auto values = mbglStopValue.toArray();
    return @[ @(values[0]), @(values[1]) ];
  }

  // Color
  static MLNColor *toMLNRawStyleValue(const mln::Color mbglStopValue) {
    return [MLNColor mgl_colorWithColor:mbglStopValue];
  }

  // VariableAnchorOffsetCollection
  static NSArray<NSExpression *> *toMLNRawStyleValue(
      const mln::VariableAnchorOffsetCollection mbglStopValue) {
    NSMutableArray *array = [NSMutableArray arrayWithCapacity:mbglStopValue.size() * 2];
    for (const auto &anchorOffset : mbglStopValue) {
      NSString *anchor = toMLNRawStyleValue(anchorOffset.anchorType);
      NSValue *offset = [NSValue mgl_valueWithOffsetArray:anchorOffset.offset];
      [array addObject:[NSExpression expressionForConstantValue:anchor]];
      [array addObject:[NSExpression expressionForConstantValue:offset]];
    }
    return array;
  }

  // Image
  static NSString *toMLNRawStyleValue(const mln::style::expression::Image &mbglImageValue) {
    return @(mbglImageValue.id().c_str());
  }

  // Array
  static NSArray<NSExpression *> *toMLNRawStyleValue(
      const std::vector<MBGLElement> &mbglStopValue) {
    NSMutableArray *array = [NSMutableArray arrayWithCapacity:mbglStopValue.size()];
    for (const auto &mbglElement : mbglStopValue) {
      [array addObject:[NSExpression expressionForConstantValue:toMLNRawStyleValue(mbglElement)]];
    }
    return array;
  }

  static NSValue *toMLNRawStyleValue(const mln::style::Position &mbglStopValue) {
    std::array<float, 3> spherical = mbglStopValue.getSpherical();
    MLNSphericalPosition position =
        MLNSphericalPositionMake(spherical[0], spherical[1], spherical[2]);
    return [NSValue valueWithMLNSphericalPosition:position];
  }

  // Enumerations
  template <typename MBGLEnum = MBGLType, typename MLNEnum = ObjCEnum>
  static NSString *toMLNRawStyleValue(const MBGLEnum &value) {
    return @(mln::Enum<MBGLEnum>::toString(value));
  }

  /// Converts all types of mbgl property values into an equivalent NSExpression.
  class PropertyExpressionEvaluator {
  public:
    NSExpression *operator()(const mln::style::Undefined) const { return nil; }

    NSExpression *operator()(const MBGLType &value) const {
      id constantValue = toMLNRawStyleValue(value);
      if ([constantValue isKindOfClass:[NSArray class]]) {
        return [NSExpression expressionForAggregate:constantValue];
      }
      return [NSExpression expressionForConstantValue:constantValue];
    }

    NSExpression *operator()(const mln::style::PropertyExpression<MBGLType> &mbglValue) const {
      return [NSExpression
          expressionWithMLNJSONObject:MLNJSONObjectFromMBGLExpression(mbglValue.getExpression())];
    }
  };
};
