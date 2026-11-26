#pragma once

#include <mln/util/noncopyable.hpp>

#include <jni/jni.hpp>
#include <mln/style/image.hpp>

namespace mln {
namespace android {

class Image : private mln::util::noncopyable {
public:
    static constexpr auto Name() { return "org/maplibre/android/maps/Image"; };

    static mln::style::Image getImage(jni::JNIEnv&, const jni::Object<Image>&);

    static void registerNative(jni::JNIEnv&);
};

} // namespace android
} // namespace mln
