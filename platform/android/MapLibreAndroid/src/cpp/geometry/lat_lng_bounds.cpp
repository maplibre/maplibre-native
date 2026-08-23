#include "lat_lng_bounds.hpp"

namespace mln {
namespace android {

jni::Local<jni::Object<LatLngBounds>> LatLngBounds::New(jni::JNIEnv& env, mln::LatLngBounds bounds) {
    static auto& javaClass = jni::Class<LatLngBounds>::Singleton(env);
    static auto constructor = javaClass.GetConstructor<double, double, double, double>(env);
    return javaClass.New(env, constructor, bounds.north(), bounds.east(), bounds.south(), bounds.west());
}

mln::LatLngBounds LatLngBounds::getLatLngBounds(jni::JNIEnv& env, const jni::Object<LatLngBounds>& bounds) {
    static auto& javaClass = jni::Class<LatLngBounds>::Singleton(env);
    static auto swLatField = javaClass.GetField<jni::jdouble>(env, "latitudeSouth");
    static auto swLonField = javaClass.GetField<jni::jdouble>(env, "longitudeWest");
    static auto neLatField = javaClass.GetField<jni::jdouble>(env, "latitudeNorth");
    static auto neLonField = javaClass.GetField<jni::jdouble>(env, "longitudeEast");

    mln::LatLng sw = {bounds.Get(env, swLatField), bounds.Get(env, swLonField)};
    mln::LatLng ne = {bounds.Get(env, neLatField), bounds.Get(env, neLonField)};

    return mln::LatLngBounds::hull(sw, ne);
}

void LatLngBounds::registerNative(jni::JNIEnv& env) {
    jni::Class<LatLngBounds>::Singleton(env);
}

} // namespace android
} // namespace mln
