#include "offline_region_error.hpp"

namespace mln {
namespace android {

jni::Local<jni::Object<OfflineRegionError>> OfflineRegionError::New(jni::JNIEnv& env, mln::Response::Error error) {
    // Handle the value of reason independently of the underlying int value
    std::string reason;
    switch (error.reason) {
        case mln::Response::Error::Reason::Success:
            reason = "REASON_SUCCESS";
            break;
        case mln::Response::Error::Reason::NotFound:
            reason = "REASON_NOT_FOUND";
            break;
        case mln::Response::Error::Reason::Server:
            reason = "REASON_SERVER";
            break;
        case mln::Response::Error::Reason::Connection:
            reason = "REASON_CONNECTION";
            break;
        case mln::Response::Error::Reason::RateLimit:
            reason = "REASON_RATE_LIMIT";
            break;
        case mln::Response::Error::Reason::Other:
            reason = "REASON_OTHER";
            break;
    }

    static auto& javaClass = jni::Class<OfflineRegionError>::Singleton(env);
    static auto constructor = javaClass.GetConstructor<jni::String, jni::String>(env);

    return javaClass.New(
        env, constructor, jni::Make<jni::String>(env, reason), jni::Make<jni::String>(env, error.message));
}

void OfflineRegionError::registerNative(jni::JNIEnv& env) {
    jni::Class<OfflineRegionError>::Singleton(env);
}

} // namespace android
} // namespace mln
