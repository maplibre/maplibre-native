#pragma once

#include <cmath>

namespace mbgl {
namespace util {

/**
 * @brief Converts degrees to radians
 *
 * @param deg Degrees as double.
 * @return Radians as double.
 */
constexpr double deg2rad(double deg) noexcept {
    return deg * M_PI / 180.0;
}

/**
 * @brief Converts degrees to radians
 *
 * @param deg Degrees as float.
 * @return Radians as float.
 */
constexpr float deg2radf(float deg) noexcept {
    return deg * static_cast<float>(M_PI) / 180.0F;
}

/**
 * @brief Converts radians to degrees
 *
 * @param rad Radians as double.
 * @return Degrees as double.
 */
constexpr double rad2deg(double rad) noexcept {
    return rad * 180.0 / M_PI;
}

/**
 * @brief Converts radians to degrees
 *
 * @param rad Radians as float.
 * @return Degrees as float.
 */
constexpr float rad2degf(float rad) noexcept {
    return rad * 180.0F / static_cast<float>(M_PI);
}

} // namespace util
} // namespace mbgl
