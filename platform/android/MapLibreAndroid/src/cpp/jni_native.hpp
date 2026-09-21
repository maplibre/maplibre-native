#pragma once

#include <mln/util/util.hpp>

#include "jni.hpp"

namespace mln {
namespace android {

MBGL_EXPORT void registerNatives(JavaVM* vm);

} // namespace android
} // namespace mln
