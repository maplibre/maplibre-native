#pragma once

#include "../../conversion/conversion.hpp"
#include "../position.hpp"

#include <mln/style/position.hpp>
#include <jni/jni.hpp>

namespace mln {
namespace android {
namespace conversion {

template <>
struct Converter<jni::Local<jni::Object<Position>>, mln::style::Position> {
    Result<jni::Local<jni::Object<Position>>> operator()(jni::JNIEnv &env, const mln::style::Position &value) const;
};

template <>
struct Converter<mln::style::Position, jni::Object<Position>> {
    Result<mln::style::Position> operator()(jni::JNIEnv &env, const jni::Object<Position> &value) const;
};

} // namespace conversion
} // namespace android
} // namespace mln
