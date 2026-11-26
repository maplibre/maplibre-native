#pragma once

#include <mln/map/map_snapshotter.hpp>

#include <jni/jni.hpp>

#include "../geometry/lat_lng.hpp"
#include "../graphics/pointf.hpp"

#include <vector>
#include <string>

namespace mln {
namespace android {

class MapSnapshot {
public:
    using PointForFn = mln::MapSnapshotter::PointForFn;
    using LatLngForFn = mln::MapSnapshotter::LatLngForFn;

    static constexpr auto Name() { return "org/maplibre/android/snapshotter/MapSnapshot"; };

    static void registerNative(jni::JNIEnv&);

    static jni::Local<jni::Object<MapSnapshot>> New(JNIEnv& env,
                                                    PremultipliedImage&& image,
                                                    float pixelRatio,
                                                    std::vector<std::string> attributions,
                                                    bool showLogo,
                                                    bool showAttribution,
                                                    PointForFn pointForFn,
                                                    LatLngForFn latLngForFn);

    MapSnapshot(jni::JNIEnv&) {};
    MapSnapshot(float pixelRatio, PointForFn, LatLngForFn);
    ~MapSnapshot();

    jni::Local<jni::Object<PointF>> pixelForLatLng(jni::JNIEnv&, jni::Object<LatLng>&);
    jni::Local<jni::Object<LatLng>> latLngForPixel(jni::JNIEnv&, jni::Object<PointF>&);

private:
    float pixelRatio;
    mln::MapSnapshotter::PointForFn pointForFn;
    mln::MapSnapshotter::LatLngForFn latLngForFn;
};

} // namespace android
} // namespace mln
