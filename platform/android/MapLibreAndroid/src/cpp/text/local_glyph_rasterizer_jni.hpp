#pragma once

#include <mln/util/image.hpp>

#include <jni/jni.hpp>

#include <cstdint>

/*
    android::LocalGlyphRasterizer is the JNI wrapper of
    org/maplibre/android/text/LocalGlyphRasterizer

    mln::LocalGlyphRasterizer is the portable interface
    Both implementations are in local_glyph_rasterizer.cpp
 */

namespace mln {
namespace android {

class LocalGlyphRasterizer {
public:
    static constexpr auto Name() { return "org/maplibre/android/text/LocalGlyphRasterizer"; };

    static void registerNative(jni::JNIEnv&);

    LocalGlyphRasterizer();

    PremultipliedImage drawGlyphBitmap(const std::string& fontFamily, const bool bold, const char16_t glyphID);
    float getLastGlyphTop();

private:
    jni::Global<jni::Object<LocalGlyphRasterizer>, jni::EnvAttachingDeleter> javaObject;
};

} // namespace android
} // namespace mln
