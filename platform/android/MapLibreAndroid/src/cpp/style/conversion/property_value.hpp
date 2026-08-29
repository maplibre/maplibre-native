#pragma once

#include <mln/style/color_ramp_property_value.hpp>
#include <mln/style/property_value.hpp>

#include "../../conversion/conversion.hpp"
#include "../../conversion/constant.hpp"
#include "property_expression.hpp"

namespace mln {
namespace android {
namespace conversion {

/**
 * Conversion from core property value types to Java property value types
 */
template <typename T>
class PropertyValueEvaluator {
public:
    PropertyValueEvaluator(jni::JNIEnv& _env)
        : env(_env) {}

    jni::Local<jni::Object<>> operator()(const mln::style::Undefined) const {
        return jni::Local<jni::Object<>>(env, nullptr);
    }

    jni::Local<jni::Object<>> operator()(const T& value) const {
        return std::move(*convert<jni::Local<jni::Object<>>>(env, value));
    }

    jni::Local<jni::Object<>> operator()(const mln::style::PropertyExpression<T>& value) const {
        return std::move(*convert<jni::Local<jni::Object<android::gson::JsonElement>>>(env, value));
    }

private:
    jni::JNIEnv& env;
};

/**
 * Convert core property values to java
 */
template <class T>
struct Converter<jni::Local<jni::Object<>>, mln::style::PropertyValue<T>> {
    Result<jni::Local<jni::Object<>>> operator()(jni::JNIEnv& env, const mln::style::PropertyValue<T>& value) const {
        PropertyValueEvaluator<T> evaluator(env);
        return value.evaluate(evaluator);
    }
};

/**
 * Convert core heat map color property value to java
 */
template <>
struct Converter<jni::Local<jni::Object<>>, mln::style::ColorRampPropertyValue> {
    Result<jni::Local<jni::Object<>>> operator()(jni::JNIEnv& env,
                                                 const mln::style::ColorRampPropertyValue& value) const {
        PropertyValueEvaluator<mln::style::ColorRampPropertyValue> evaluator(env);
        return std::move(*convert<jni::Local<jni::Object<>>>(env, value.evaluate(evaluator)));
    }
};

} // namespace conversion
} // namespace android
} // namespace mln
