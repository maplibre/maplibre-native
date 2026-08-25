#include <mln/util/geo.hpp>
#include "pointf.hpp"

namespace mln {
namespace android {

jni::Local<jni::Object<PointF>> PointF::New(jni::JNIEnv& env, float x, float y) {
    static auto& javaClass = jni::Class<PointF>::Singleton(env);
    static auto constructor = javaClass.GetConstructor<float, float>(env);
    return javaClass.New(env, constructor, x, y);
}

mln::ScreenCoordinate PointF::getScreenCoordinate(jni::JNIEnv& env, const jni::Object<PointF>& point) {
    static auto& javaClass = jni::Class<PointF>::Singleton(env);
    static auto xField = javaClass.GetField<jni::jfloat>(env, "x");
    static auto yField = javaClass.GetField<jni::jfloat>(env, "y");
    return mln::ScreenCoordinate{point.Get(env, xField), point.Get(env, yField)};
}

void PointF::registerNative(jni::JNIEnv& env) {
    jni::Class<PointF>::Singleton(env);
}

} // namespace android
} // namespace mln
