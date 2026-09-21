#pragma once

#include "value.hpp"

#include <mln/util/feature.hpp>
#include <mln/util/logging.hpp>
#include <mln/style/conversion/geojson.hpp>
#include <mln/style/conversion_impl.hpp>

#include <jni/jni.hpp>
#include <optional>

namespace mln {
namespace style {
namespace conversion {

template <>
class ConversionTraits<mln::android::Value> {
public:
    static bool isUndefined(const mln::android::Value& value) { return value.isNull(); }

    static bool isArray(const mln::android::Value& value) { return value.isArray(); }

    static bool isObject(const mln::android::Value& value) { return value.isObject(); }

    static std::size_t arrayLength(const mln::android::Value& value) {
        return value.getLength();
        ;
    }

    static mln::android::Value arrayMember(const mln::android::Value& value, std::size_t i) { return value.get(i); }

    static std::optional<mln::android::Value> objectMember(const mln::android::Value& value, const char* key) {
        mln::android::Value member = value.get(key);
        if (!member.isNull()) {
            return member;
        } else {
            return {};
        }
    }

    template <class Fn>
    static std::optional<Error> eachMember(const mln::android::Value& value, Fn&& fn) {
        assert(value.isObject());
        mln::android::Value keys = value.keyArray();
        std::size_t length = arrayLength(keys);
        for (std::size_t i = 0; i < length; ++i) {
            const auto k = keys.get(i).toString();
            auto v = value.get(k.c_str());
            std::optional<Error> result = fn(k, std::move(v));
            if (result) {
                return result;
            }
        }
        return {};
    }

    static std::optional<bool> toBool(const mln::android::Value& value) {
        if (value.isBool()) {
            return value.toBool();
        } else {
            return {};
        }
    }

    static std::optional<float> toNumber(const mln::android::Value& value) {
        if (value.isNumber()) {
            auto num = value.toFloat();
            return num;
        } else {
            return {};
        }
    }

    static std::optional<double> toDouble(const mln::android::Value& value) {
        if (value.isNumber()) {
            return value.toDouble();
        } else {
            return {};
        }
    }

    static std::optional<std::string> toString(const mln::android::Value& value) {
        if (value.isString()) {
            return value.toString();
        } else {
            return {};
        }
    }

    static std::optional<Value> toValue(const mln::android::Value& value) {
        if (value.isNull()) {
            return {};
        } else if (value.isBool()) {
            return {value.toBool()};
        } else if (value.isString()) {
            return {value.toString()};
        } else if (value.isNumber()) {
            return {value.toDouble()};
        } else {
            return {};
        }
    }

    static std::optional<GeoJSON> toGeoJSON(const mln::android::Value& value, Error& error) {
        if (value.isNull()) {
            error = {"no json data found"};
            return {};
        }

        if (value.isString()) {
            return parseGeoJSON(value.toString(), error);
        }

        if (value.isObject()) {
            mln::android::Value keys = value.keyArray();
            std::size_t length = arrayLength(keys);
            for (std::size_t i = 0; i < length; ++i) {
                if (keys.get(i).toString() == "json") {
                    auto v = value.get("json");
                    if (v.isString()) {
                        return parseGeoJSON(v.toString(), error);
                    } else {
                        break;
                    }
                }
            }
        }
        error = {"no json data found"};
        return {};
    }
};

template <class T, class... Args>
std::optional<T> convert(mln::android::Value&& value, Error& error, Args&&... args) {
    return convert<T>(Convertible(std::move(value)), error, std::forward<Args>(args)...);
}

} // namespace conversion
} // namespace style
} // namespace mln
