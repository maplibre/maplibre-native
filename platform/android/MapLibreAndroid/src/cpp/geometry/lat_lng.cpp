#include "lat_lng.hpp"

namespace mln {
namespace android {

jni::Local<jni::Object<LatLng>> LatLng::New(jni::JNIEnv& env, const mln::LatLng& latLng) {
    static auto& javaClass = jni::Class<LatLng>::Singleton(env);
    static auto constructor = javaClass.GetConstructor<double, double>(env);
    return javaClass.New(env, constructor, latLng.latitude(), latLng.longitude());
}

mln::Point<double> LatLng::getGeometry(jni::JNIEnv& env, const jni::Object<LatLng>& latLng) {
    static auto& javaClass = jni::Class<LatLng>::Singleton(env);
    static auto latitudeField = javaClass.GetField<jni::jdouble>(env, "latitude");
    static auto longitudeField = javaClass.GetField<jni::jdouble>(env, "longitude");
    return mln::Point<double>(latLng.Get(env, longitudeField), latLng.Get(env, latitudeField));
}

mln::LatLng LatLng::getLatLng(jni::JNIEnv& env, const jni::Object<LatLng>& latLng) {
    auto point = LatLng::getGeometry(env, latLng);
    return mln::LatLng(point.y, point.x);
}

void LatLng::registerNative(jni::JNIEnv& env) {
    jni::Class<LatLng>::Singleton(env);
}

} // namespace android
} // namespace mln
