#import "MLNShape_Private.h"

#include <mbgl/style/conversion_impl.hpp>

NS_ASSUME_NONNULL_BEGIN

namespace mbgl {
namespace style {
namespace conversion {

// A wrapper class for `id`, so as not to confuse ARC.
class Holder {
public:
    Holder(const id v) : value(v) {}
    const id value;
};

template <>
class ConversionTraits<Holder> {
public:
    static bool isUndefined(const Holder& holder) {
        const id value = holder.value;
        return !value || value == [NSNull null];
    }

    static bool isArray(const Holder& holder) {
        const id value = holder.value;
        return [value isKindOfClass:[NSArray class]];
    }

    static bool isObject(const Holder& holder) {
        const id value = holder.value;
        return [value isKindOfClass:[NSDictionary class]];
    }

    static std::size_t arrayLength(const Holder& holder) {
        const id value = holder.value;
        NSCAssert([value isKindOfClass:[NSArray class]], @"Value must be an NSArray for getLength().");
        NSArray *array = value;
        auto length = [array count];
        NSCAssert(length <= std::numeric_limits<size_t>::max(), @"Array length out of bounds.");
        return length;
    }

    static Holder arrayMember(const Holder& holder, std::size_t i) {
        const id value = holder.value;
        NSCAssert([value isKindOfClass:[NSArray class]], @"Value must be an NSArray for get(int).");
        NSCAssert(i < NSUIntegerMax, @"Index must be less than NSUIntegerMax");
        return {[value objectAtIndex: i]};
    }

    static std::optional<Holder> objectMember(const Holder& holder, const char *key) {
        const id value = holder.value;
        NSCAssert([value isKindOfClass:[NSDictionary class]], @"Value must be an NSDictionary for get(string).");
        NSObject *member = [value objectForKey: @(key)];
        if (member && member != [NSNull null]) {
            return {member};
        } else {
            return {};
        }
    }

// Compiler is wrong about `Fn` parameter missing a nullability specifier.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability-completeness"
    template <class Fn>
    static std::optional<Error> eachMember(const Holder& holder, Fn&& visit) {
#pragma clang diagnostic pop
        [holder.value enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull key, id  _Nonnull obj, BOOL * _Nonnull stop) {
            auto result = visit(std::string(static_cast<const char *>([key UTF8String])), obj);
            if (result) {
                *stop = YES;
            }
        }];
        return {};
    }

    static std::optional<bool> toBool(const Holder& holder) {
        const id value = holder.value;
        if (_isBool(value)) {
            return ((NSNumber *)value).boolValue;
        } else {
            return {};
        }
    }

    static std::optional<float> toNumber(const Holder& holder) {
        const id value = holder.value;
        if (_isNumber(value)) {
            return ((NSNumber *)value).floatValue;
        } else {
            return {};
        }
    }

    static std::optional<double> toDouble(const Holder& holder) {
        const id value = holder.value;
        if (_isNumber(value)) {
            return ((NSNumber *)value).doubleValue;
        } else {
            return {};
        }
    }

    static std::optional<std::string> toString(const Holder& holder) {
        const id value = holder.value;
        if (_isString(value)) {
            return std::string(static_cast<const char *>([value UTF8String]));
        } else {
            return {};
        }
    }

    static std::optional<mbgl::Value> toValue(const Holder& holder) {
        const id value = holder.value;
        if (isUndefined(value)) {
            return {};
        } else if (_isBool(value)) {
            return { *toBool(holder) };
        } else if ( _isString(value)) {
            return { *toString(holder) };
        } else if (_isNumber(value)) {
           return { *toDouble(holder) };
        } else {
            return {};
        }
    }

    static std::optional<GeoJSON> toGeoJSON(const Holder& holder, Error& error) {
        id object = holder.value;
        if ([object isKindOfClass:[NSDictionary class]]) {
            NSError *serializationError;
            NSData *data = [NSJSONSerialization dataWithJSONObject:object options:0 error:&serializationError];
            if (serializationError) {
                error = { std::string(serializationError.localizedDescription.UTF8String) };
                return {};
            }
            NSString *string = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
            return mapbox::geojson::parse(string.UTF8String);
        } else {
            error = { std::string([NSString stringWithFormat:@"%@ is not a GeoJSON object.", object].UTF8String) };
            return {};
        }
    }

private:
    static bool _isBool(const id value) {
        if (![value isKindOfClass:[NSNumber class]]) return false;
        // char: 32-bit boolean
        // BOOL: 64-bit boolean
        NSNumber *number = value;
        return ((strcmp([number objCType], @encode(char)) == 0) ||
                (strcmp([number objCType], @encode(BOOL)) == 0));
    }

    static bool _isNumber(const id value) {
        return [value isKindOfClass:[NSNumber class]] && !_isBool(value);
    }

    static bool _isString(const id value) {
        return [value isKindOfClass:[NSString class]];
    }
};

inline Convertible makeConvertible(const id value) {
    return Convertible(Holder(value));
}

} // namespace conversion
} // namespace style
} // namespace mbgl

NS_ASSUME_NONNULL_END
