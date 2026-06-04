#pragma once

#include "source.hpp"
#include <mbgl/style/sources/custom_vector_source.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <jni/jni.hpp>

namespace mbgl {
namespace android {

class CustomVectorSource : public Source {
public:
    using SuperTag = Source;
    static constexpr auto Name() { return "org/maplibre/android/style/sources/CustomVectorSource"; }

    static void registerNative(jni::JNIEnv&);

    CustomVectorSource(jni::JNIEnv&, const jni::String&, jni::jint minZoom, jni::jint maxZoom);
    ~CustomVectorSource();

    bool removeFromMap(JNIEnv&, const jni::Object<Source>&, mbgl::Map&) override;
    void addToMap(JNIEnv&, const jni::Object<Source>&, mbgl::Map&, AndroidRendererFrontend&) override;

    void fetchTile(const mbgl::CanonicalTileID& tileID);
    void cancelTile(const mbgl::CanonicalTileID& tileID);
    void onAddedToMap();
    void onRemovedFromMap();

private:
    void setTileData(jni::JNIEnv&, jni::jint z, jni::jint x, jni::jint y,
                     const jni::Array<jni::jbyte>& data, jni::jint format);
    void setTileError(jni::JNIEnv&, jni::jint z, jni::jint x, jni::jint y,
                      const jni::String& message);
    void invalidateTile(jni::JNIEnv&, jni::jint z, jni::jint x, jni::jint y);
};

} // namespace android
} // namespace mbgl
