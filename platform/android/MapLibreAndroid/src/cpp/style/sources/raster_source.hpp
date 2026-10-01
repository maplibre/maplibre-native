#pragma once

#include "source.hpp"
#include <mln/style/sources/raster_source.hpp>
#include <jni/jni.hpp>

namespace mln {
namespace android {

class RasterSource : public Source {
public:
    using SuperTag = Source;
    static constexpr auto Name() { return "org/maplibre/android/style/sources/RasterSource"; };

    static void registerNative(jni::JNIEnv&);

    RasterSource(jni::JNIEnv&, const jni::String&, const jni::Object<>&, jni::jint);
    RasterSource(jni::JNIEnv&, mln::style::Source&, AndroidRendererFrontend*);
    ~RasterSource();

    jni::Local<jni::String> getURL(jni::JNIEnv&);

private:
    jni::Local<jni::Object<Source>> createJavaPeer(jni::JNIEnv&);

}; // class RasterSource

} // namespace android
} // namespace mln
