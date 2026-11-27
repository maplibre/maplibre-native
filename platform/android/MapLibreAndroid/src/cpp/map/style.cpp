#include "style.hpp"

#include <mln/util/crs.hpp>

#include <stdexcept>

namespace mln {
namespace android {

void Style::addProjection(jni::JNIEnv& env,
                          const jni::Class<Style>&,
                          const jni::Object<CoordinateReferenceSystem>& projection) {
    try {
        mln::addProjection(CoordinateReferenceSystem::getCRS(env, projection));
    } catch (const std::invalid_argument& error) {
        jni::ThrowNew(env, jni::FindClass(env, "java/lang/IllegalArgumentException"), error.what());
    }
}

void Style::removeProjection(jni::JNIEnv& env, const jni::Class<Style>&, const jni::String& name) {
    try {
        mln::removeProjection(jni::Make<std::string>(env, name));
    } catch (const std::invalid_argument& error) {
        jni::ThrowNew(env, jni::FindClass(env, "java/lang/IllegalArgumentException"), error.what());
    }
}

void Style::registerNative(jni::JNIEnv& env) {
    static auto& javaClass = jni::Class<Style>::Singleton(env);
    jni::RegisterNatives(
        env,
        *javaClass,
        jni::MakeNativeMethod<decltype(&Style::addProjection), &Style::addProjection>("nativeAddProjection"),
        jni::MakeNativeMethod<decltype(&Style::removeProjection), &Style::removeProjection>("nativeRemoveProjection"));
}

} // namespace android
} // namespace mln
