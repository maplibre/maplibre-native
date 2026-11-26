#pragma once

#include "source.hpp"
#include <mln/style/source.hpp>
#include <jni/jni.hpp>

namespace mln {
namespace android {

class UnknownSource : public Source {
public:
    using SuperTag = Source;
    static constexpr auto Name() { return "org/maplibre/android/style/sources/UnknownSource"; };

    static void registerNative(jni::JNIEnv&);

    UnknownSource(jni::JNIEnv&, mln::style::Source&, AndroidRendererFrontend*);

    ~UnknownSource() = default;

private:
    jni::Local<jni::Object<Source>> createJavaPeer(jni::JNIEnv&);

}; // class UnknownSource

} // namespace android
} // namespace mln
