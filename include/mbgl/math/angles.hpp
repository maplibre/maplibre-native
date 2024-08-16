#pragma once

#include <numbers>

namespace mbgl {
namespace util {

/**
 * @brief Converts degrees to radians
 *
 * @param deg Degrees as double.
 * @return Radians as double.
 */
constexpr double deg2rad(double deg) noexcept {
    return deg * std::numbers::pi / 180.0;
}

/**
 * @brief Converts degrees to radians
 *
 * @param deg Degrees as float.
 * @return Radians as float.
 */
constexpr float deg2radf(float deg) noexcept {
    return deg * std::numbers::pi_v<float> / 180.0F;
}

/**
 * @brief Converts radians to degrees
 *
 * @param rad Radians as double.
 * @return Degrees as double.
 */
constexpr double rad2deg(double rad) noexcept {
    return rad * 180.0 / std::numbers::pi;
}

/**
 * @brief Converts radians to degrees
 *
 * @param rad Radians as float.
 * @return Degrees as float.
 */
constexpr float rad2degf(float rad) noexcept {
    return rad * 180.0F / std::numbers::pi_v<float>;
}

} // namespace util
} // namespace mbgl
