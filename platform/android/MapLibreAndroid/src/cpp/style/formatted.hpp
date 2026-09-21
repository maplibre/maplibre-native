#pragma once

#include <mln/util/noncopyable.hpp>

#include <jni/jni.hpp>
#include <mln/style/expression/formatted.hpp>

namespace mln {
namespace android {

using SuperTag = jni::ObjectTag;
class Formatted : private mln::util::noncopyable {
public:
    static constexpr auto Name() { return "org/maplibre/android/style/types/Formatted"; };

    static jni::Local<jni::Object<Formatted>> New(jni::JNIEnv &, const style::expression::Formatted &value);

    static void registerNative(jni::JNIEnv &);
};

} // namespace android
} // namespace mln
