#pragma once

#include "conversion.hpp"

#include <mbgl/util/color.hpp>
#include <mbgl/util/padding.hpp>
#include <mbgl/util/enum.hpp>

#include <mbgl/style/expression/formatted.hpp>
#include <mbgl/style/expression/image.hpp>
#include <mbgl/style/variable_anchor_offset_collection.hpp>

#include <jni/jni.hpp>

#include <string>
#include <array>
#include <vector>
#include <mbgl/style/rotation.hpp>

namespace mbgl {
namespace android {
namespace conversion {

template <>
struct Converter<jni::Local<jni::Object<>>, bool> {
    Result<jni::Local<jni::Object<>>> operator()(jni::JNIEnv& env, const bool& value) const;
};

template <>
struct Converter<jni::Local<jni::Object<>>, float> {
    Result<jni::Local<jni::Object<>>> operator()(jni::JNIEnv& env, const float& value) const;
};

template <>
struct Converter<jni::Local<jni::Object<>>, double> {
    Result<jni::Local<jni::Object<>>> operator()(jni::JNIEnv& env, const double& value) const;
};

/**
 * All integrals. java is limited to 64 bit signed, so...
 * TODO: use BigDecimal for > 64 / unsigned?
 */
template <typename T>
struct Converter<jni::Local<jni::Object<>>, T, typename std::enable_if<std::is_integral<T>::value>::type> {
    Result<jni::Local<jni::Object<>>> operator()(jni::JNIEnv& env, const T& value) const {
        return jni::Box(env, jni::jlong(value));
    }
};

// TODO: convert integral types to primitive jni types

template <>
struct Converter<jni::Local<jni::Object<>>, std::string> {
    Result<jni::Local<jni::Object<>>> operator()(jni::JNIEnv& env, const std::string& value) const;
};

template <>
struct Converter<jni::Local<jni::Object<>>, Color> {
    Result<jni::Local<jni::Object<>>> operator()(jni::JNIEnv& env, const Color& value) const;
};

template <>
struct Converter<jni::Local<jni::Object<>>, Padding> {
    Result<jni::Local<jni::Object<>>> operator()(jni::JNIEnv& env, const Padding& value) const;
};

template <>
struct Converter<jni::Local<jni::Object<>>, VariableAnchorOffsetCollection> {
    Result<jni::Local<jni::Object<>>> operator()(jni::JNIEnv& env, const VariableAnchorOffsetCollection& value) const;
};

template <>
struct Converter<jni::Local<jni::Object<>>, style::expression::Formatted> {
    Result<jni::Local<jni::Object<>>> operator()(jni::JNIEnv& env, const style::expression::Formatted& value) const;
};

template <std::size_t N>
struct Converter<jni::Local<jni::Object<>>, std::array<float, N>> {
    Result<jni::Local<jni::Object<>>> operator()(jni::JNIEnv& env, const std::array<float, N>& value) const {
        std::vector<float> v;
        for (const float& id : value) {
            v.push_back(id);
        }
        return convert<jni::Local<jni::Object<>>, std::vector<float>>(env, v);
    }
};

template <std::size_t N>
struct Converter<jni::Local<jni::Object<>>, std::array<double, N>> {
    Result<jni::Local<jni::Object<>>> operator()(jni::JNIEnv& env, const std::array<double, N>& value) const {
        std::vector<double> v;
        for (const double& id : value) {
            v.push_back(id);
        }
        return convert<jni::Local<jni::Object<>>, std::vector<double>>(env, v);
    }
};

template <>
struct Converter<jni::Local<jni::Object<>>, std::vector<std::string>> {
    Result<jni::Local<jni::Object<>>> operator()(jni::JNIEnv& env, const std::vector<std::string>& value) const;
};

template <>
struct Converter<jni::Local<jni::Object<>>, std::vector<float>> {
    Result<jni::Local<jni::Object<>>> operator()(jni::JNIEnv& env, const std::vector<float>& value) const;
};

template <>
struct Converter<jni::Local<jni::Object<>>, std::vector<double>> {
    Result<jni::Local<jni::Object<>>> operator()(jni::JNIEnv& env, const std::vector<double>& value) const;
};

template <class T>
struct Converter<jni::Local<jni::Object<>>, T, typename std::enable_if_t<std::is_enum<T>::value>> {
    Result<jni::Local<jni::Object<>>> operator()(jni::JNIEnv& env, const T& value) const {
        return convert<jni::Local<jni::Object<>>, std::string>(env, Enum<T>::toString(value));
    }
};

template <class T>
struct Converter<jni::Local<jni::Object<>>, std::vector<T>, typename std::enable_if_t<std::is_enum<T>::value>> {
    Result<jni::Local<jni::Object<>>> operator()(jni::JNIEnv& env, const std::vector<T>& value) const {
        auto result = jni::Array<jni::String>::New(env, value.size());
        for (std::size_t i = 0; i < value.size(); ++i) {
            result.Set(env, i, jni::Make<jni::String>(env, Enum<T>::toString(value.at(i))));
        }
        return result;
    }
};

template <>
struct Converter<jni::Local<jni::Object<>>, style::expression::Image> {
    Result<jni::Local<jni::Object<>>> operator()(jni::JNIEnv& env, const style::expression::Image& value) const;
};

template <>
struct Converter<jni::Local<jni::Object<>>, mbgl::style::Rotation> {
    Result<jni::Local<jni::Object<>>> operator()(jni::JNIEnv& env, const mbgl::style::Rotation& value) const;
};

} // namespace conversion
} // namespace android
} // namespace mbgl
