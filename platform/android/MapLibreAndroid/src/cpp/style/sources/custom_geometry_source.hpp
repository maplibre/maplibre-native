#pragma once

#include "source.hpp"
#include <mln/style/sources/custom_geometry_source.hpp>
#include <mln/util/geojson.hpp>
#include <mln/tile/tile_id.hpp>
#include "../../geojson/geometry.hpp"
#include "../../geojson/feature.hpp"
#include "../../geojson/feature_collection.hpp"
#include "../../geometry/lat_lng_bounds.hpp"
#include <jni/jni.hpp>

namespace mln {
namespace android {

class CustomGeometrySource : public Source {
public:
    using SuperTag = Source;
    static constexpr auto Name() { return "org/maplibre/android/style/sources/CustomGeometrySource"; };

    static void registerNative(jni::JNIEnv&);

    CustomGeometrySource(jni::JNIEnv&, const jni::String&, const jni::Object<>&);
    CustomGeometrySource(jni::JNIEnv&, mln::style::Source&, AndroidRendererFrontend*);
    ~CustomGeometrySource();

    bool removeFromMap(JNIEnv&, const jni::Object<Source>&, mln::Map&) override;
    void addToMap(JNIEnv&, const jni::Object<Source>&, mln::Map&, AndroidRendererFrontend&) override;

    void fetchTile(const mln::CanonicalTileID& tileID);
    void cancelTile(const mln::CanonicalTileID& tileID);
    bool isCancelled(jni::jint z, jni::jint x, jni::jint y);
    void startThreads();
    void releaseThreads();

private:
    void setTileData(
        jni::JNIEnv& env, jni::jint z, jni::jint x, jni::jint y, const jni::Object<geojson::FeatureCollection>& jf);

    void invalidateTile(jni::JNIEnv& env, jni::jint z, jni::jint x, jni::jint y);
    void invalidateBounds(jni::JNIEnv& env, const jni::Object<LatLngBounds>& bounds);

    jni::Local<jni::Array<jni::Object<geojson::Feature>>> querySourceFeatures(jni::JNIEnv&,
                                                                              const jni::Array<jni::Object<>>&);

    jni::Local<jni::Object<Source>> createJavaPeer(jni::JNIEnv&);

}; // class CustomGeometrySource

} // namespace android
} // namespace mln
