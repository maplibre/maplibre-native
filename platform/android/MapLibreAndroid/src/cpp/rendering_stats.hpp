#pragma once

#include <mln/gfx/rendering_stats.hpp>

#include <jni.h>
#include <jni/jni.hpp>

namespace mln {
namespace android {

struct RenderingStats {
    static constexpr auto Name() { return "org/maplibre/android/maps/RenderingStats"; }

    static void registerNative(jni::JNIEnv& env);

    static jni::Local<jni::Object<RenderingStats>> Create(jni::JNIEnv&);
    static void Update(jni::JNIEnv&, jni::Object<RenderingStats>&, const gfx::RenderingStats&);
};

} // namespace android
} // namespace mln
