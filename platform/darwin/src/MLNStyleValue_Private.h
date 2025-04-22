#import <Foundation/Foundation.h>
#include <mbgl/util/constants.hpp>

#import "MLNStyleValue.h"

#import "MLNTypes.h"
#import "NSDate+MLNAdditions.h"
#import "NSExpression+MLNPrivateAdditions.h"
#import "NSValue+MLNAdditions.h"
#import "NSValue+MLNStyleAttributeAdditions.h"

#include <mbgl/style/conversion/color_ramp_property_value.hpp>
#include <mbgl/style/conversion/position.hpp>
#include <mbgl/style/conversion/property_value.hpp>
#include <mbgl/style/expression/dsl.hpp>
#import <mbgl/style/transition_options.hpp>
#import <mbgl/style/types.hpp>
#import "MLNConversion.h"
#import "MLNSymbolStyleLayer.h"

#import <mbgl/util/enum.hpp>
#include <mbgl/util/interpolate.hpp>
#include <mbgl/util/unitbezier.hpp>

#include <memory>

#if TARGET_OS_IPHONE
#import "UIColor+MLNAdditions.h"
#else
#import "NSColor+MLNAdditions.h"
#endif

namespace mbgl {
namespace style {
namespace expression {
class Expression;
}
}  // namespace style
}  // namespace mbgl

id MLNJSONObjectFromMBGLValue(const mbgl::Value &value);

NS_INLINE MLNTransition MLNTransitionFromOptions(const mbgl::style::TransitionOptions &options) {
  MLNTransition transition;
  transition.duration =
      MLNTimeIntervalFromDuration(options.duration.value_or(mbgl::Duration::zero()));
  transition.delay = MLNTimeIntervalFromDuration(options.delay.value_or(mbgl::Duration::zero()));

  return transition;
}

NS_INLINE mbgl::style::TransitionOptions MLNOptionsFromTransition(MLNTransition transition) {
  mbgl::util::UnitBezier ease = mbgl::util::DEFAULT_TRANSITION_EASE;
  if (transition.ease) {
    CAMediaTimingFunction *function = [CAMediaTimingFunction functionWithName:transition.ease];
    float p1[2], p2[2];
    [function getControlPointAtIndex:1 values:p1];
    [function getControlPointAtIndex:2 values:p2];
    ease = {p1[0], p1[1], p2[0], p2[1]};
  }

  mbgl::style::TransitionOptions options{{MLNDurationFromTimeInterval(transition.duration)},
                                         {MLNDurationFromTimeInterval(transition.delay)},
                                         ease};
  return options;
}

std::unique_ptr<mbgl::style::expression::Expression> MLNClusterPropertyFromNSExpression(
    NSExpression *expression);

id MLNJSONObjectFromMBGLExpression(const mbgl::style::expression::Expression &mbglExpression);

template <typename MBGLType, typename ObjCType, typename MBGLElement = MBGLType,
          typename ObjCEnum = ObjCType>
class MLNStyleValueTransformer {
 public:
  /// Convert an mbgl property value into an mgl style value
  NSExpression *toExpression(const mbgl::style::PropertyValue<MBGLType> &mbglValue) {
    PropertyExpressionEvaluator evaluator;
    return mbglValue.evaluate(evaluator);
  }

  // Convert an mbgl heatmap color property value into an mgl style value
  NSExpression *toExpression(const mbgl::style::ColorRampPropertyValue &mbglValue) {
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
  typename std::enable_if_t<!std::is_same<MBGLValue, mbgl::style::ColorRampPropertyValue>::value,
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

    mbgl::style::conversion::Error valueError;
    auto value = mbgl::style::conversion::convert<MBGLValue>(
        mbgl::style::conversion::makeConvertible(jsonExpression), valueError, allowDataExpressions,
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
  typename std::enable_if_t<std::is_same<MBGLValue, mbgl::style::ColorRampPropertyValue>::value,
                            MBGLValue>
  toPropertyValue(NSExpression *expression) {
    if (!expression) {
      return {};
    }

    NSArray *jsonExpression = expression.mgl_jsonExpressionObject;

    mbgl::style::conversion::Error valueError;
    auto value = mbgl::style::conversion::convert<mbgl::style::ColorRampPropertyValue>(
        mbgl::style::conversion::makeConvertible(jsonExpression), valueError);
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
    return @(mbgl::Enum<MLNEnum>::toString(mglEnum));
  }

  NSObject *toRawStyleSpecValue(MLNColor *color, MBGLType &) {
    return @(color.mgl_color.stringify().c_str());
  }

  // Bool
  void getMBGLValue(NSNumber *rawValue, bool &mbglValue) { mbglValue = !!rawValue.boolValue; }

  // Float
  void getMBGLValue(NSNumber *rawValue, float &mbglValue) { mbglValue = rawValue.floatValue; }

  void getMBGLValue(NSNumber *rawValue, double &mbglValue) { mbglValue = rawValue.doubleValue; }

  void getMBGLValue(NSNumber *rawValue, mbgl::style::Rotation &mbglValue) {
    mbglValue = rawValue.floatValue;
  }

  // String
  void getMBGLValue(NSString *rawValue, std::string &mbglValue) { mbglValue = rawValue.UTF8String; }

  // Formatted
  void getMBGLValue(NSString *rawValue, mbgl::style::expression::Formatted &mbglValue) {
    mbglValue = mbgl::style::expression::Formatted(rawValue.UTF8String);
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

  void getMBGLValue(id rawValue, std::array<double, 3> &mbglValue) {
    if ([rawValue isKindOfClass:[NSValue class]]) {
      mbglValue = [rawValue mgl_locationArrayValue];
    } else if ([rawValue isKindOfClass:[NSArray class]]) {
      NSArray *array = (NSArray *)rawValue;
      getMBGLValue(array[0], mbglValue[0]);
      getMBGLValue(array[1], mbglValue[1]);
      getMBGLValue(array[2], mbglValue[2]);
    }
  }

  // Padding type (supports numbers and float arrays w/ sizes 1 to 4)
  void getMBGLValue(id rawValue, mbgl::Padding &mbglValue) {
    if ([rawValue isKindOfClass:[NSNumber class]]) {
      NSNumber *number = (NSNumber *)rawValue;
      mbglValue = mbgl::Padding(number.floatValue);
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
      mbglValue = mbgl::Padding(std::span<float>(values.begin(), array.count));
    } else if ([rawValue isKindOfClass:[NSValue class]]) {
      mbglValue = mbgl::Padding([rawValue mgl_paddingArrayValue]);
    }
  }

  // Color
  void getMBGLValue(MLNColor *rawValue, mbgl::Color &mbglValue) { mbglValue = rawValue.mgl_color; }

  // VariableAnchorOffsetCollection
  void getMBGLValue(id rawValue, mbgl::VariableAnchorOffsetCollection &mbglValue) {
    if ([rawValue isKindOfClass:[NSArray class]]) {
      NSArray *array = (NSArray *)rawValue;
      if (array.count % 2 != 0) {
        [NSException
             raise:NSInvalidArgumentException
            format:@"VariableTextAnchorOffset array should have an even number of elements."];
      }

      std::vector<mbgl::AnchorOffsetPair> anchorOffsets;
      anchorOffsets.reserve(array.count / 2);
      for (NSUInteger i = 0; i < array.count; i += 2) {
        mbgl::style::SymbolAnchorType anchor{0};
        getMBGLValue<mbgl::style::SymbolAnchorType, MLNTextAnchor>(array[i], anchor);

        std::array<float, 2> offsetArray;
        getMBGLValue(array[i + 1], offsetArray);

        anchorOffsets.emplace_back(anchor, offsetArray);
      }

      mbglValue = mbgl::VariableAnchorOffsetCollection(std::move(anchorOffsets));
    }
  }

  // Image
  void getMBGLValue(NSString *rawValue, mbgl::style::expression::Image &mbglValue) {
    mbglValue = mbgl::style::expression::Image(rawValue.UTF8String);
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

  void getMBGLValue(NSValue *rawValue, mbgl::style::Position &mbglValue) {
    auto spherical = rawValue.mgl_lightPositionArrayValue;
    mbgl::style::Position position(spherical);
    mbglValue = position;
  }

  // Enumerations
  template <typename MBGLEnum = MBGLType, typename MLNEnum = ObjCEnum,
            class = typename std::enable_if<std::is_enum<MBGLEnum>::value>::type,
            class = typename std::enable_if<std::is_enum<MLNEnum>::value>::type>
  void getMBGLValue(id rawValue, MBGLEnum &mbglValue) {
    if ([rawValue isKindOfClass:[NSString class]]) {
      mbglValue = *mbgl::Enum<MBGLEnum>::toEnum([(NSString *)rawValue UTF8String]);
    } else {
      MLNEnum mglEnum;
      [(NSValue *)rawValue getValue:&mglEnum];
      auto str = mbgl::Enum<MLNEnum>::toString(mglEnum);
      mbglValue = *mbgl::Enum<MBGLEnum>::toEnum(str);
    }
  }

 private:  // Private utilities for converting from mbgl to mgl values
  // Bool
  static NSNumber *toMLNRawStyleValue(const bool mbglStopValue) { return @(mbglStopValue); }

  // Float
  static NSNumber *toMLNRawStyleValue(const float mbglStopValue) { return @(mbglStopValue); }

  static NSNumber *toMLNRawStyleValue(const double mbglStopValue) { return @(mbglStopValue); }

  static NSNumber *toMLNRawStyleValue(const mbgl::style::Rotation mbglStopValue) {
    return @(mbglStopValue.getAngle());
  }

  // Integer
  static NSNumber *toMLNRawStyleValue(const int64_t mbglStopValue) { return @(mbglStopValue); }

  // String
  static NSString *toMLNRawStyleValue(const std::string &mbglStopValue) {
    return @(mbglStopValue.c_str());
  }

  // Formatted
  static NSString *toMLNRawStyleValue(const mbgl::style::expression::Formatted &mbglStopValue) {
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

  static NSValue *toMLNRawStyleValue(const std::array<double, 3> &mbglStopValue) {
    return [NSValue mgl_valueWithLocationArray:mbglStopValue];
  }

  // Padding type
  static NSValue *toMLNRawStyleValue(const mbgl::Padding &mbglStopValue) {
    return [NSValue mgl_valueWithPaddingArray:mbglStopValue.toArray()];
  }

  // Color
  static MLNColor *toMLNRawStyleValue(const mbgl::Color mbglStopValue) {
    return [MLNColor mgl_colorWithColor:mbglStopValue];
  }

  // VariableAnchorOffsetCollection
  static NSArray<NSExpression *> *toMLNRawStyleValue(
      const mbgl::VariableAnchorOffsetCollection mbglStopValue) {
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
  static NSString *toMLNRawStyleValue(const mbgl::style::expression::Image &mbglImageValue) {
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

  static NSValue *toMLNRawStyleValue(const mbgl::style::Position &mbglStopValue) {
    std::array<float, 3> spherical = mbglStopValue.getSpherical();
    MLNSphericalPosition position =
        MLNSphericalPositionMake(spherical[0], spherical[1], spherical[2]);
    return [NSValue valueWithMLNSphericalPosition:position];
  }

  // Enumerations
  template <typename MBGLEnum = MBGLType, typename MLNEnum = ObjCEnum>
  static NSString *toMLNRawStyleValue(const MBGLEnum &value) {
    return @(mbgl::Enum<MBGLEnum>::toString(value));
  }

  /// Converts all types of mbgl property values into an equivalent NSExpression.
  class PropertyExpressionEvaluator {
   public:
    NSExpression *operator()(const mbgl::style::Undefined) const { return nil; }

    NSExpression *operator()(const MBGLType &value) const {
      id constantValue = toMLNRawStyleValue(value);
      if ([constantValue isKindOfClass:[NSArray class]]) {
        return [NSExpression expressionForAggregate:constantValue];
      }
      return [NSExpression expressionForConstantValue:constantValue];
    }

    NSExpression *operator()(const mbgl::style::PropertyExpression<MBGLType> &mbglValue) const {
      return [NSExpression
          expressionWithMLNJSONObject:MLNJSONObjectFromMBGLExpression(mbglValue.getExpression())];
    }
  };
};
