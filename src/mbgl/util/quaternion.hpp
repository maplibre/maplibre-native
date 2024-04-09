#pragma once

#include <mbgl/util/mat3.hpp>
#include <mbgl/util/mat4.hpp>

namespace mbgl {

struct Quaternion {
    union {
        vec4 m;
        struct {
            double x, y, z, w;
        };
    };

    Quaternion() noexcept
        : Quaternion(0.0, 0.0, 0.0, 0.0) {}
    Quaternion(double x_, double y_, double z_, double w_) noexcept
        : x(x_),
          y(y_),
          z(z_),
          w(w_) {}
    Quaternion(const vec4& vec) noexcept
        : m(vec) {}

    Quaternion conjugate() const noexcept;
    Quaternion normalized() const noexcept;
    Quaternion multiply(const Quaternion& o) const noexcept;
    double length() const noexcept;
    vec3 transform(const vec3& v) const noexcept;
    mat4 toRotationMatrix() const noexcept;

    static Quaternion fromAxisAngle(const vec3& axis, double angleRad) noexcept;
    static Quaternion fromEulerAngles(double x, double y, double z) noexcept;

    static Quaternion identity;
};

bool operator==(const Quaternion&, const Quaternion&) noexcept;
bool operator!=(const Quaternion&, const Quaternion&) noexcept;

} // namespace mbgl
