#include <mbgl/util/quaternion.hpp>
#include <cassert>
#include <cmath>

namespace mbgl {

Quaternion Quaternion::identity = Quaternion(0.0, 0.0, 0.0, 1.0);

Quaternion Quaternion::conjugate() const noexcept {
    return {-x, -y, -z, w};
}

Quaternion Quaternion::normalized() const noexcept {
    const double len = length();
    assert(len > 0.0);
    return {x / len, y / len, z / len, w / len};
}

Quaternion Quaternion::fromAxisAngle(const vec3& axis, double angleRad) noexcept {
    const double coss = std::cos(0.5 * angleRad);
    const double sins = std::sin(0.5 * angleRad);
    return {sins * axis[0], sins * axis[1], sins * axis[2], coss};
}

Quaternion Quaternion::fromEulerAngles(double x, double y, double z) noexcept {
    const double cz = std::cos(z * 0.5);
    const double sz = std::sin(z * 0.5);
    const double cy = std::cos(y * 0.5);
    const double sy = std::sin(y * 0.5);
    const double cx = std::cos(x * 0.5);
    const double sx = std::sin(x * 0.5);

    return {sx * cy * cz - cx * sy * sz,
            cx * sy * cz + sx * cy * sz,
            cx * cy * sz - sx * sy * cz,
            cx * cy * cz + sx * sy * sz};
}

Quaternion Quaternion::multiply(const Quaternion& o) const noexcept {
    return {w * o.w - x * o.x - y * o.y - z * o.z,
            w * o.x + x * o.w + y * o.z - z * o.y,
            w * o.y + y * o.w + z * o.x - x * o.z,
            w * o.z + z * o.w + x * o.y - y * o.x};
}

double Quaternion::length() const noexcept {
    return std::sqrt(x * x + y * y + z * z + w * w);
}

vec3 Quaternion::transform(const vec3& v) const noexcept {
    const Quaternion src = {v[0], v[1], v[2], 0.0};
    const Quaternion res = multiply(src).multiply(conjugate());
    return {res.x, res.y, res.z};
}

mat4 Quaternion::toRotationMatrix() const noexcept {
    mat4 mat;

    const double tx = 2.0 * x;
    const double ty = 2.0 * y;
    const double tz = 2.0 * z;
    const double twx = tx * w;
    const double twy = ty * w;
    const double twz = tz * w;
    const double txx = tx * x;
    const double txy = ty * x;
    const double txz = tz * x;
    const double tyy = ty * y;
    const double tyz = tz * y;
    const double tzz = tz * z;

    mat[0] = 1.0 - (tyy + tzz);
    mat[1] = txy + twz;
    mat[2] = txz - twy;
    mat[3] = 0.0;
    mat[4] = txy - twz;
    mat[5] = 1.0 - (txx + tzz);
    mat[6] = tyz + twx;
    mat[7] = 0.0;
    mat[8] = txz + twy;
    mat[9] = tyz - twx;
    mat[10] = 1.0 - (txx + tyy);
    mat[11] = 0.0;
    mat[12] = 0.0;
    mat[13] = 0.0;
    mat[14] = 0.0;
    mat[15] = 1.0;

    return mat;
}

bool operator!=(const Quaternion& a, const Quaternion& b) noexcept {
    return !(a == b);
}

bool operator==(const Quaternion& a, const Quaternion& b) noexcept {
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

} // namespace mbgl
