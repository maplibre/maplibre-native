#include "polygon.hpp"

#include "../conversion/color.hpp"

namespace mln {
namespace android {

mln::FillAnnotation Polygon::toAnnotation(jni::JNIEnv& env, const jni::Object<Polygon>& polygon) {
    static auto& javaClass = jni::Class<Polygon>::Singleton(env);
    static auto points = javaClass.GetField<jni::Object<java::util::List>>(env, "points");
    static auto holes = javaClass.GetField<jni::Object<java::util::List>>(env, "holes");
    static auto alpha = javaClass.GetField<float>(env, "alpha");
    static auto fillColor = javaClass.GetField<int>(env, "fillColor");
    static auto strokeColor = javaClass.GetField<int>(env, "strokeColor");

    mln::Polygon<double> geometry{MultiPoint::toGeometry<mln::LinearRing<double>>(env, polygon.Get(env, points))};

    auto jHoleListsArray = java::util::List::toArray<java::util::List>(env, polygon.Get(env, holes));

    std::size_t jHoleListsSize = jHoleListsArray.Length(env);
    for (std::size_t i = 0; i < jHoleListsSize; i++) {
        geometry.push_back(MultiPoint::toGeometry<mln::LinearRing<double>>(env, jHoleListsArray.Get(env, i)));
    }

    mln::FillAnnotation annotation{geometry};
    annotation.opacity = polygon.Get(env, alpha);
    annotation.color = *conversion::convert<mln::Color>(env, polygon.Get(env, fillColor));
    annotation.outlineColor = *conversion::convert<mln::Color>(env, polygon.Get(env, strokeColor));

    return annotation;
}

void Polygon::registerNative(jni::JNIEnv& env) {
    jni::Class<Polygon>::Singleton(env);
}

} // namespace android
} // namespace mln
