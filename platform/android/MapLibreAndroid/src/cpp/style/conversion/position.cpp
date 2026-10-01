#include "position.hpp"

namespace mln {
namespace android {
namespace conversion {

Result<jni::Local<jni::Object<Position>>>
Converter<jni::Local<jni::Object<Position>>, mln::style::Position>::operator()(
    jni::JNIEnv &env, const mln::style::Position &value) const {
    std::array<float, 3> cartPosition = value.getSpherical();
    return Position::fromPosition(env, cartPosition[0], cartPosition[1], cartPosition[2]);
}

Result<mln::style::Position> Converter<mln::style::Position, jni::Object<Position>>::operator()(
    jni::JNIEnv &env, const jni::Object<Position> &value) const {
    float radialCoordinate = Position::getRadialCoordinate(env, value);
    float azimuthalAngle = Position::getAzimuthalAngle(env, value);
    float polarAngle = Position::getPolarAngle(env, value);
    std::array<float, 3> cartPosition{{radialCoordinate, azimuthalAngle, polarAngle}};
    mln::style::Position position{};
    position.set(cartPosition);
    return position;
}

} // namespace conversion
} // namespace android
} // namespace mln
