#pragma once

#include <mln/util/noncopyable.hpp>

#include <jni/jni.hpp>

namespace mln {
namespace android {

class FormattedSection : private mln::util::noncopyable {
public:
    static constexpr auto Name() { return "org/maplibre/android/style/types/FormattedSection"; };

    static void registerNative(jni::JNIEnv&);
};

} // namespace android
} // namespace mln
