#include "constant.hpp"
#include "collection.hpp"
#include "../style/formatted.hpp"

#include <mbgl/style/conversion/stringify.hpp>
#include <mbgl/util/string.hpp>

namespace mbgl {
namespace android {
namespace conversion {

Result<jni::Local<jni::Object<>>> Converter<jni::Local<jni::Object<>>, bool>::operator()(jni::JNIEnv& env,
                                                                                         const bool& value) const {
    return jni::Box(env, value ? jni::jni_true : jni::jni_false);
}

Result<jni::Local<jni::Object<>>> Converter<jni::Local<jni::Object<>>, float>::operator()(jni::JNIEnv& env,
                                                                                          const float& value) const {
    return jni::Box(env, value);
}

Result<jni::Local<jni::Object<>>> Converter<jni::Local<jni::Object<>>, double>::operator()(jni::JNIEnv& env,
                                                                                           const double& value) const {
    return jni::Box(env, value);
}

Result<jni::Local<jni::Object<>>> Converter<jni::Local<jni::Object<>>, std::string>::operator()(
    jni::JNIEnv& env, const std::string& value) const {
    return jni::Make<jni::String>(env, value);
}

Result<jni::Local<jni::Object<>>> Converter<jni::Local<jni::Object<>>, Color>::operator()(jni::JNIEnv& env,
                                                                                          const Color& value) const {
    return jni::Make<jni::String>(env, value.stringify());
}

Result<jni::Local<jni::Object<>>> Converter<jni::Local<jni::Object<>>, Padding>::operator()(
    jni::JNIEnv& env, const Padding& value) const {
    const auto values = value.toArray();
    auto result = jni::Array<jni::Float>::New(env, values.size());
    for (size_t i = 0; i < values.size(); i++) {
        result.Set(env, i, jni::Box(env, values[i]));
    }
    return result;
}

Result<jni::Local<jni::Object<>>> Converter<jni::Local<jni::Object<>>, VariableAnchorOffsetCollection>::operator()(
    jni::JNIEnv& env, const VariableAnchorOffsetCollection& value) const {
    auto variableAnchorOffsets = jni::Array<jni::Object<>>::New(env, value.size() * 2);
    for (std::size_t i = 0; i < value.size(); i++) {
        auto anchorOffsetPair = value[i];
        auto anchorType = jni::Make<jni::String>(env,
                                                 Enum<style::SymbolAnchorType>::toString(anchorOffsetPair.anchorType));
        auto offset = jni::Array<jni::Float>::New(env, anchorOffsetPair.offset.size());
        for (size_t j = 0; j < anchorOffsetPair.offset.size(); j++) {
            offset.Set(env, j, jni::Box(env, anchorOffsetPair.offset[j]));
        }

        variableAnchorOffsets.Set(env, i, anchorType);
        variableAnchorOffsets.Set(env, i + 1, offset);
    }

    return variableAnchorOffsets;
}

Result<jni::Local<jni::Object<>>> Converter<jni::Local<jni::Object<>>, style::expression::Formatted>::operator()(
    jni::JNIEnv& env, const style::expression::Formatted& value) const {
    return Formatted::New(env, value);
}

Result<jni::Local<jni::Object<>>> Converter<jni::Local<jni::Object<>>, std::vector<std::string>>::operator()(
    jni::JNIEnv& env, const std::vector<std::string>& value) const {
    auto result = jni::Array<jni::String>::New(env, value.size());

    for (std::size_t i = 0; i < value.size(); i++) {
        result.Set(env, i, jni::Make<jni::String>(env, value.at(i)));
    }

    return result;
}

Result<jni::Local<jni::Object<>>> Converter<jni::Local<jni::Object<>>, std::vector<float>>::operator()(
    jni::JNIEnv& env, const std::vector<float>& value) const {
    auto result = jni::Array<jni::Float>::New(env, value.size());

    for (std::size_t i = 0; i < value.size(); i++) {
        result.Set(env, i, jni::Box(env, value.at(i)));
    }

    return result;
}

Result<jni::Local<jni::Object<>>> Converter<jni::Local<jni::Object<>>, std::vector<double>>::operator()(
    jni::JNIEnv& env, const std::vector<double>& value) const {
    auto result = jni::Array<jni::Double>::New(env, value.size());

    for (std::size_t i = 0; i < value.size(); i++) {
        result.Set(env, i, jni::Box(env, value.at(i)));
    }

    return result;
}

Result<jni::Local<jni::Object<>>> Converter<jni::Local<jni::Object<>>, style::expression::Image>::operator()(
    jni::JNIEnv& env, const style::expression::Image& value) const {
    return jni::Make<jni::String>(env, value.id());
}

Result<jni::Local<jni::Object<>>> Converter<jni::Local<jni::Object<>>, mbgl::style::Rotation>::operator()(
    jni::JNIEnv& env, const mbgl::style::Rotation& value) const {
    return jni::Box(env, value.getAngle());
}

} // namespace conversion
} // namespace android
} // namespace mbgl
