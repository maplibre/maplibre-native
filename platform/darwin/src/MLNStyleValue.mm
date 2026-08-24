#import "MLNStyleValue_Private.h"

#include <mln/style/expression/expression.hpp>

const MLNStyleFunctionOption MLNStyleFunctionOptionInterpolationBase =
    @"MLNStyleFunctionOptionInterpolationBase";
const MLNStyleFunctionOption MLNStyleFunctionOptionDefaultValue =
    @"MLNStyleFunctionOptionDefaultValue";

id MLNJSONObjectFromMBGLValue(const mln::Value &value) {
  return value.match(
      [](const mln::NullValue) -> id { return [NSNull null]; },
      [](const bool value) { return @(value); }, [](const float value) { return @(value); },
      [](const int64_t value) { return @(value); }, [](const uint64_t value) { return @(value); },
      [](const double value) { return @(value); },
      [](const std::string &value) { return @(value.c_str()); },
      [](const mln::Color &value) { return [MLNColor mgl_colorWithColor:value]; },
      [](const mln::style::Position &value) {
        std::array<float, 3> spherical = value.getSpherical();
        MLNSphericalPosition position =
            MLNSphericalPositionMake(spherical[0], spherical[1], spherical[2]);
        return [NSValue valueWithMLNSphericalPosition:position];
      },
      [&](const std::vector<mln::Value> &vector) {
        NSMutableArray *array = [NSMutableArray arrayWithCapacity:vector.size()];
        for (auto value : vector) {
          [array addObject:MLNJSONObjectFromMBGLValue(value)];
        }
        return array;
      },
      [&](const std::unordered_map<std::string, mln::Value> &map) {
        NSMutableDictionary *dictionary = [NSMutableDictionary dictionaryWithCapacity:map.size()];
        for (auto &item : map) {
          dictionary[@(item.first.c_str())] = MLNJSONObjectFromMBGLValue(item.second);
        }
        return dictionary;
      },
      [](const auto &) -> id { return nil; });
}

id MLNJSONObjectFromMBGLExpression(const mln::style::expression::Expression &mbglExpression) {
  return MLNJSONObjectFromMBGLValue(mbglExpression.serialize());
}

mln::Value MLNValueFromJSONObject(id object) {
  if (!object || object == [NSNull null]) {
    return {};
  }
  if ([object isKindOfClass:[NSString class]]) {
    return {std::string([(NSString *)object UTF8String])};
  }
  if ([object isKindOfClass:[NSNumber class]]) {
    NSNumber *number = (NSNumber *)object;
    if ((strcmp([number objCType], @encode(char)) == 0) ||
        (strcmp([number objCType], @encode(BOOL)) == 0)) {
      return {(bool)number.boolValue};
    }
    if ((strcmp([number objCType], @encode(double)) == 0) ||
        (strcmp([number objCType], @encode(float)) == 0)) {
      return {(double)number.doubleValue};
    }
    if ([number compare:@(0)] == NSOrderedAscending) {
      return {(int64_t)number.longLongValue};
    }
    return {(uint64_t)number.unsignedLongLongValue};
  }
  if ([object isKindOfClass:[NSArray class]]) {
    mapbox::base::ValueArray array;
    array.reserve([(NSArray *)object count]);
    for (id element in (NSArray *)object) {
      array.push_back(MLNValueFromJSONObject(element));
    }
    return array;
  }
  if ([object isKindOfClass:[NSDictionary class]]) {
    mapbox::base::ValueObject dictionary;
    NSDictionary *dictionaryObject = (NSDictionary *)object;
    for (NSString *key in dictionaryObject) {
      dictionary.emplace(std::string(key.UTF8String),
                         MLNValueFromJSONObject(dictionaryObject[key]));
    }
    return dictionary;
  }
  [NSException raise:NSInvalidArgumentException format:@"Can’t convert %@ to mln::Value", object];
  return {};
}

std::unique_ptr<mln::style::expression::Expression> MLNClusterPropertyFromNSExpression(
    NSExpression *expression) {
  if (!expression) {
    return nullptr;
  }

  NSArray *jsonExpression = expression.mgl_jsonExpressionObject;

  auto expr = mln::style::expression::dsl::createExpression(
      mln::style::conversion::makeConvertible(jsonExpression));

  return expr;
}
