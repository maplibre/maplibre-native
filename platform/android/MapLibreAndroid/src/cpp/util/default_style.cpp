#include "default_style.hpp"

namespace mln {
namespace android {

jni::Local<jni::Object<DefaultStyle>> DefaultStyle::New(jni::JNIEnv& env,
                                                        const mln::util::DefaultStyle& jDefaultStyle) {
    static auto& javaClass = jni::Class<DefaultStyle>::Singleton(env);
    static auto constructor = javaClass.GetConstructor<jni::String, jni::String, int>(env);
    return javaClass.New(env,
                         constructor,
                         jni::Make<jni::String>(env, jDefaultStyle.getUrl()),
                         jni::Make<jni::String>(env, jDefaultStyle.getName()),
                         jDefaultStyle.getCurrentVersion());
}

mln::util::DefaultStyle DefaultStyle::getDefaultStyle(jni::JNIEnv& env, const jni::Object<DefaultStyle>& options) {
    static auto& javaClass = jni::Class<DefaultStyle>::Singleton(env);
    return mln::util::DefaultStyle(
        jni::Make<std::string>(env, options.Get(env, javaClass.GetField<jni::String>(env, "url"))),
        jni::Make<std::string>(env, options.Get(env, javaClass.GetField<jni::String>(env, "name"))),
        options.Get(env, javaClass.GetField<int>(env, "version")));
}

void DefaultStyle::registerNative(jni::JNIEnv& env) {
    jni::Class<DefaultStyle>::Singleton(env);
}

} // namespace android
} // namespace mln
