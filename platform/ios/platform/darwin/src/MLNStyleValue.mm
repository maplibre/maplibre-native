#import "MLNStyleValue_Private.h"

#include <mbgl/style/expression/expression.hpp>

const MLNStyleFunctionOption MLNStyleFunctionOptionInterpolationBase = @"MLNStyleFunctionOptionInterpolationBase";
const MLNStyleFunctionOption MLNStyleFunctionOptionDefaultValue = @"MLNStyleFunctionOptionDefaultValue";

id MLNJSONObjectFromMBGLValue(const mbgl::Value &value) {
    return value.match([](const mbgl::NullValue) -> id {
        return [NSNull null];
    }, [](const bool value) {
        return @(value);
    }, [](const float value) {
        return @(value);
    }, [](const int64_t value) {
        return @(value);
    }, [](const uint64_t value) {
        return @(value);        
    }, [](const double value) {
        return @(value);
    }, [](const std::string &value) {
        return @(value.c_str());
    }, [](const mbgl::Color &value) {
        return [MLNColor mgl_colorWithColor:value];
    }, [](const mbgl::style::Position &value) {
        std::array<float, 3> spherical = value.getSpherical();
        MLNSphericalPosition position = MLNSphericalPositionMake(spherical[0], spherical[1], spherical[2]);
        return [NSValue valueWithMLNSphericalPosition:position];
    }, [&](const std::vector<mbgl::Value> &vector) {
        NSMutableArray *array = [NSMutableArray arrayWithCapacity:vector.size()];
        for (auto value : vector) {
            [array addObject:MLNJSONObjectFromMBGLValue(value)];
        }
        return array;
    }, [&](const std::unordered_map<std::string, mbgl::Value> &map) {
        NSMutableDictionary *dictionary = [NSMutableDictionary dictionaryWithCapacity:map.size()];
        for (auto &item : map) {
            dictionary[@(item.first.c_str())] = MLNJSONObjectFromMBGLValue(item.second);
        }
        return dictionary;
    }, [](const auto &) -> id {
        return nil;
    });
}

id MLNJSONObjectFromMBGLExpression(const mbgl::style::expression::Expression &mbglExpression) {
    return MLNJSONObjectFromMBGLValue(mbglExpression.serialize());
}


std::unique_ptr<mbgl::style::expression::Expression> MLNClusterPropertyFromNSExpression(NSExpression *expression) {
    if (!expression) {
        return nullptr;
    }

    NSArray *jsonExpression = expression.mgl_jsonExpressionObject;

    auto expr = mbgl::style::expression::dsl::createExpression(mbgl::style::conversion::makeConvertible(jsonExpression));

    return expr;
}
