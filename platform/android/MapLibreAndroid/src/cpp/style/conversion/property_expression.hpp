#pragma once

#include "../../conversion/conversion.hpp"
#include "../../gson/json_element.hpp"

#include <mln/style/property_expression.hpp>

#include <jni/jni.hpp>

namespace mln {
namespace android {
namespace conversion {

template <class T>
struct Converter<jni::Local<jni::Object<android::gson::JsonElement>>, mln::style::PropertyExpression<T>> {
    Result<jni::Local<jni::Object<android::gson::JsonElement>>> operator()(
        jni::JNIEnv& env, const mln::style::PropertyExpression<T>& value) const {
        return gson::JsonElement::New(env, value.getExpression().serialize());
    }
};

} // namespace conversion
} // namespace android
} // namespace mln
