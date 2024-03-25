#pragma once

#include <mbgl/util/size.hpp>
#include <mbgl/util/quaternion.hpp>

#include <optional>

namespace mbgl {

class LatLng;

namespace util {

class Camera {
public:
    Camera() noexcept;

    vec3 getPosition() const noexcept;
    mat4 getCameraToWorld(double scale, bool flippedY) const noexcept;
    mat4 getWorldToCamera(double scale, bool flippedY) const noexcept;
    mat4 getCameraToClipPerspective(double fovy, double aspectRatio, double nearZ, double farZ) const noexcept;

    vec3 forward() const noexcept;
    vec3 right() const noexcept;
    vec3 up() const noexcept;

    const Quaternion& getOrientation() const noexcept { return orientation; }
    void getOrientation(double& pitch, double& bearing) const noexcept;
    void setOrientation(double pitch, double bearing) noexcept;
    void setOrientation(const Quaternion& orientation_) noexcept;
    void setPosition(const vec3& position) noexcept;

    // Computes orientation using forward and up vectors of the provided
    // coordinate frame. Only bearing and pitch components will be used. Does
    // not return a value if input is invalid
    static std::optional<Quaternion> orientationFromFrame(const vec3& forward, const vec3& up) noexcept;

private:
    Quaternion orientation;
    mat4 transform; // Position (mercator) and orientation of the camera
};

} // namespace util
} // namespace mbgl
