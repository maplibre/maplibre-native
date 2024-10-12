#include "variable_anchor_offset.hpp"
#include "anchor_offset.hpp"
#include "../conversion/conversion.hpp"
#include "../conversion/constant.hpp"

namespace mbgl {
namespace android {

void VariableAnchorOffset::registerNative(jni::JNIEnv& env) {
    jni::Class<VariableAnchorOffset>::Singleton(env);
}

jni::Local<jni::Object<VariableAnchorOffset>> VariableAnchorOffset::New(jni::JNIEnv& env, const VariableAnchorOffsetCollection & value) {
    static auto& variableAnchorOffsetClass = jni::Class<VariableAnchorOffset>::Singleton(env);
    static auto& anchorOffsetClass = jni::Class<AnchorOffset>::Singleton(env);
    static auto variableAnchorOffsetConstructor = variableAnchorOffsetClass.GetConstructor<jni::Array<jni::Object<AnchorOffset>>>(env);
    static auto anchorOffsetConstructor = anchorOffsetClass.GetConstructor<jni::String, jni::Array<jni::Float>>(env);

    auto variableAnchorOffsets = jni::Array<jni::Object<AnchorOffset>>::New(env, value.size());
    for (std::size_t i = 0; i < value.size(); i++) {
        auto anchorOffsetPair = value[i];
        auto anchorType = jni::Make<jni::String>(env, Enum<style::SymbolAnchorType>::toString(anchorOffsetPair.anchorType));
        auto offset = jni::Array<jni::Float>::New(env, anchorOffsetPair.offset.size());
        for (size_t j = 0; j < anchorOffsetPair.offset.size(); j++) {
            offset.Set(env, j, jni::Box(env, anchorOffsetPair.offset[j]));
        }

        auto anchorOffset = anchorOffsetClass.New(env, anchorOffsetConstructor, anchorType, offset);
        variableAnchorOffsets.Set(env, i, anchorOffset);
    }

    return variableAnchorOffsetClass.New(env, variableAnchorOffsetConstructor, variableAnchorOffsets);
}

} // namespace android
} // namespace mbgl