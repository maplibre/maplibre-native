#include "native_values.hpp"

#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

namespace mbgl {
namespace ohos {
namespace {

constexpr std::array<const char*, 3> Vector3PropertyNames = {"x", "y", "z"};
constexpr std::array<const char*, 4> QuaternionPropertyNames = {"x", "y", "z", "w"};

constexpr std::uint32_t DebugOptionsMask = static_cast<std::uint32_t>(MapDebugOptions::TileBorders) |
                                           static_cast<std::uint32_t>(MapDebugOptions::ParseStatus) |
                                           static_cast<std::uint32_t>(MapDebugOptions::Timestamps) |
                                           static_cast<std::uint32_t>(MapDebugOptions::Collision) |
                                           static_cast<std::uint32_t>(MapDebugOptions::Overdraw) |
                                           static_cast<std::uint32_t>(MapDebugOptions::StencilClip) |
                                           static_cast<std::uint32_t>(MapDebugOptions::DepthBuffer);

bool isString(napi_env env, napi_value value) {
    napi_valuetype type = napi_undefined;
    return value != nullptr && napi_typeof(env, value, &type) == napi_ok && type == napi_string;
}

bool getInt32(napi_env env, napi_value value, std::int32_t& result) {
    return value != nullptr && napi_get_value_int32(env, value, &result) == napi_ok;
}

bool getOptionalObjectProperty(napi_env env, napi_value object, const char* name, napi_value& result) {
    result = nullptr;

    bool hasProperty = false;
    if (napi_has_named_property(env, object, name, &hasProperty) != napi_ok || !hasProperty) {
        return true;
    }

    napi_value value = nullptr;
    if (napi_get_named_property(env, object, name, &value) != napi_ok || isNullOrUndefined(env, value)) {
        return true;
    }

    if (!isObject(env, value)) {
        return false;
    }

    result = value;
    return true;
}

bool getOptionalFiniteDoubleProperty(napi_env env, napi_value object, const char* name, std::optional<double>& result) {
    bool hasProperty = false;
    if (napi_has_named_property(env, object, name, &hasProperty) != napi_ok || !hasProperty) {
        return true;
    }

    napi_value value = nullptr;
    if (napi_get_named_property(env, object, name, &value) != napi_ok || isNullOrUndefined(env, value)) {
        return true;
    }

    double number = 0.0;
    if (!getDouble(env, value, number) || !std::isfinite(number)) {
        return false;
    }

    result = number;
    return true;
}

template <std::size_t N>
bool getFiniteVectorObject(napi_env env,
                           napi_value value,
                           const std::array<const char*, N>& names,
                           std::array<double, N>& result) {
    if (!isObject(env, value)) {
        return false;
    }

    for (std::size_t i = 0; i < N; ++i) {
        std::optional<double> component;
        if (!getOptionalFiniteDoubleProperty(env, value, names[i], component) || !component) {
            return false;
        }
        result[i] = *component;
    }
    return true;
}

template <std::size_t N>
bool getOptionalFiniteVectorProperty(napi_env env,
                                     napi_value object,
                                     const char* name,
                                     const std::array<const char*, N>& names,
                                     std::optional<std::array<double, N>>& result) {
    napi_value vectorObject = nullptr;
    if (!getOptionalObjectProperty(env, object, name, vectorObject)) {
        return false;
    }
    if (vectorObject == nullptr) {
        return true;
    }

    std::array<double, N> vector{};
    if (!getFiniteVectorObject(env, vectorObject, names, vector)) {
        return false;
    }

    result = vector;
    return true;
}

bool getCameraCenter(napi_env env, napi_value object, std::optional<LatLng>& center) {
    std::optional<double> longitude;
    std::optional<double> latitude;
    if (!getOptionalFiniteDoubleProperty(env, object, "longitude", longitude) ||
        !getOptionalFiniteDoubleProperty(env, object, "latitude", latitude)) {
        return false;
    }

    napi_value centerObject = nullptr;
    if (!longitude && !latitude && !getOptionalObjectProperty(env, object, "center", centerObject)) {
        return false;
    }
    if (!longitude && !latitude && centerObject != nullptr) {
        if (!getOptionalFiniteDoubleProperty(env, centerObject, "longitude", longitude) ||
            !getOptionalFiniteDoubleProperty(env, centerObject, "latitude", latitude)) {
            return false;
        }
    }

    if (static_cast<bool>(longitude) != static_cast<bool>(latitude)) {
        return false;
    }
    if (!longitude) {
        return true;
    }

    try {
        center = LatLng(*latitude, *longitude);
    } catch (...) {
        return false;
    }
    return true;
}

bool getCameraPadding(napi_env env, napi_value object, std::optional<EdgeInsets>& padding) {
    napi_value paddingObject = nullptr;
    if (!getOptionalObjectProperty(env, object, "padding", paddingObject)) {
        return false;
    }
    if (paddingObject == nullptr) {
        return true;
    }

    std::optional<double> top;
    std::optional<double> left;
    std::optional<double> bottom;
    std::optional<double> right;
    if (!getOptionalFiniteDoubleProperty(env, paddingObject, "top", top) ||
        !getOptionalFiniteDoubleProperty(env, paddingObject, "left", left) ||
        !getOptionalFiniteDoubleProperty(env, paddingObject, "bottom", bottom) ||
        !getOptionalFiniteDoubleProperty(env, paddingObject, "right", right)) {
        return false;
    }

    try {
        padding = EdgeInsets(top.value_or(0.0), left.value_or(0.0), bottom.value_or(0.0), right.value_or(0.0));
    } catch (...) {
        return false;
    }
    return true;
}

bool getCameraAnchor(napi_env env, napi_value object, std::optional<ScreenCoordinate>& anchor) {
    napi_value anchorObject = nullptr;
    if (!getOptionalObjectProperty(env, object, "anchor", anchorObject)) {
        return false;
    }
    if (anchorObject == nullptr) {
        return true;
    }

    std::optional<double> x;
    std::optional<double> y;
    if (!getOptionalFiniteDoubleProperty(env, anchorObject, "x", x) ||
        !getOptionalFiniteDoubleProperty(env, anchorObject, "y", y) || static_cast<bool>(x) != static_cast<bool>(y)) {
        return false;
    }
    if (x) {
        anchor = ScreenCoordinate{*x, *y};
    }
    return true;
}

bool getOptionalLatLngBoundsProperty(napi_env env,
                                     napi_value object,
                                     const char* name,
                                     std::optional<LatLngBounds>& result) {
    napi_value boundsObject = nullptr;
    if (!getOptionalObjectProperty(env, object, name, boundsObject)) {
        return false;
    }
    if (boundsObject == nullptr) {
        return true;
    }

    std::optional<double> west;
    std::optional<double> south;
    std::optional<double> east;
    std::optional<double> north;
    if (!getOptionalFiniteDoubleProperty(env, boundsObject, "west", west) ||
        !getOptionalFiniteDoubleProperty(env, boundsObject, "south", south) ||
        !getOptionalFiniteDoubleProperty(env, boundsObject, "east", east) ||
        !getOptionalFiniteDoubleProperty(env, boundsObject, "north", north) || !west || !south || !east || !north ||
        *west > *east || *south > *north) {
        return false;
    }

    try {
        result = LatLngBounds::hull(LatLng(*south, *west), LatLng(*north, *east));
    } catch (...) {
        return false;
    }
    return true;
}

void setDoubleProperty(napi_env env, napi_value object, const char* name, double value) {
    napi_value property = nullptr;
    napi_create_double(env, value, &property);
    napi_set_named_property(env, object, name, property);
}

void setStringProperty(napi_env env, napi_value object, const char* name, const std::string& value) {
    napi_value property = nullptr;
    napi_create_string_utf8(env, value.c_str(), value.size(), &property);
    napi_set_named_property(env, object, name, property);
}

void setOptionalStringProperty(napi_env env, napi_value object, const char* name, const std::string& value) {
    if (!value.empty()) {
        setStringProperty(env, object, name, value);
    }
}

void setOptionalDoubleProperty(napi_env env, napi_value object, const char* name, const std::optional<double>& value) {
    if (value) {
        setDoubleProperty(env, object, name, *value);
    }
}

void setCameraPaddingProperty(napi_env env, napi_value object, const EdgeInsets& padding) {
    napi_value paddingObject = nullptr;
    napi_create_object(env, &paddingObject);
    setDoubleProperty(env, paddingObject, "top", padding.top());
    setDoubleProperty(env, paddingObject, "left", padding.left());
    setDoubleProperty(env, paddingObject, "bottom", padding.bottom());
    setDoubleProperty(env, paddingObject, "right", padding.right());
    napi_set_named_property(env, object, "padding", paddingObject);
}

void setCameraAnchorProperty(napi_env env, napi_value object, const ScreenCoordinate& anchor) {
    napi_value anchorObject = nullptr;
    napi_create_object(env, &anchorObject);
    setDoubleProperty(env, anchorObject, "x", anchor.x);
    setDoubleProperty(env, anchorObject, "y", anchor.y);
    napi_set_named_property(env, object, "anchor", anchorObject);
}

void setLatLngBoundsProperty(napi_env env, napi_value object, const char* name, const LatLngBounds& bounds) {
    napi_value boundsObject = nullptr;
    napi_create_object(env, &boundsObject);
    setDoubleProperty(env, boundsObject, "west", bounds.west());
    setDoubleProperty(env, boundsObject, "south", bounds.south());
    setDoubleProperty(env, boundsObject, "east", bounds.east());
    setDoubleProperty(env, boundsObject, "north", bounds.north());
    napi_set_named_property(env, object, name, boundsObject);
}

template <std::size_t N>
napi_value createVectorObject(napi_env env,
                              const std::array<double, N>& vector,
                              const std::array<const char*, N>& names) {
    napi_value object = nullptr;
    napi_create_object(env, &object);
    for (std::size_t i = 0; i < N; ++i) {
        setDoubleProperty(env, object, names[i], vector[i]);
    }
    return object;
}

void setUint32Property(napi_env env, napi_value object, const char* name, std::uint32_t value) {
    napi_value property = nullptr;
    napi_create_uint32(env, value, &property);
    napi_set_named_property(env, object, name, property);
}

} // namespace

bool getString(napi_env env, napi_value value, std::string& result) {
    if (!isString(env, value)) {
        return false;
    }

    std::size_t length = 0;
    if (napi_get_value_string_utf8(env, value, nullptr, 0, &length) != napi_ok) {
        return false;
    }

    std::vector<char> buffer(length + 1);
    if (napi_get_value_string_utf8(env, value, buffer.data(), buffer.size(), &length) != napi_ok) {
        return false;
    }

    result.assign(buffer.data(), length);
    return true;
}

bool isNullOrUndefined(napi_env env, napi_value value) {
    if (value == nullptr) {
        return true;
    }

    napi_valuetype type = napi_undefined;
    return napi_typeof(env, value, &type) == napi_ok && (type == napi_null || type == napi_undefined);
}

bool getDouble(napi_env env, napi_value value, double& result) {
    return value != nullptr && napi_get_value_double(env, value, &result) == napi_ok;
}

bool getUint32(napi_env env, napi_value value, std::uint32_t& result) {
    return value != nullptr && napi_get_value_uint32(env, value, &result) == napi_ok;
}

bool getBool(napi_env env, napi_value value, bool& result) {
    return value != nullptr && napi_get_value_bool(env, value, &result) == napi_ok;
}

bool isValidDebugOptions(std::uint32_t options) {
    return (options & ~DebugOptionsMask) == 0;
}

bool isObject(napi_env env, napi_value value) {
    napi_valuetype type = napi_undefined;
    return value != nullptr && napi_typeof(env, value, &type) == napi_ok && type == napi_object;
}

bool getOptionalStringProperty(napi_env env, napi_value object, const char* name, std::optional<std::string>& result) {
    bool hasProperty = false;
    if (napi_has_named_property(env, object, name, &hasProperty) != napi_ok || !hasProperty) {
        return true;
    }

    napi_value value = nullptr;
    if (napi_get_named_property(env, object, name, &value) != napi_ok || isNullOrUndefined(env, value)) {
        return true;
    }

    std::string valueString;
    if (!getString(env, value, valueString)) {
        return false;
    }

    result = std::move(valueString);
    return true;
}

bool getRequiredInt32Property(napi_env env, napi_value object, const char* name, std::int32_t& result) {
    bool hasProperty = false;
    if (napi_has_named_property(env, object, name, &hasProperty) != napi_ok || !hasProperty) {
        return false;
    }

    napi_value value = nullptr;
    return napi_get_named_property(env, object, name, &value) == napi_ok && getInt32(env, value, result);
}

bool getCameraOptionsObject(napi_env env, napi_value value, CameraOptions& cameraOptions) {
    if (!isObject(env, value)) {
        return false;
    }

    std::optional<LatLng> center;
    std::optional<EdgeInsets> padding;
    std::optional<ScreenCoordinate> anchor;
    if (!getCameraCenter(env, value, center) || !getCameraPadding(env, value, padding) ||
        !getCameraAnchor(env, value, anchor)) {
        return false;
    }

    std::optional<double> centerAltitude;
    std::optional<double> zoom;
    std::optional<double> bearing;
    std::optional<double> pitch;
    std::optional<double> roll;
    std::optional<double> fov;
    if (!getOptionalFiniteDoubleProperty(env, value, "centerAltitude", centerAltitude) ||
        !getOptionalFiniteDoubleProperty(env, value, "zoom", zoom) ||
        !getOptionalFiniteDoubleProperty(env, value, "bearing", bearing) ||
        !getOptionalFiniteDoubleProperty(env, value, "pitch", pitch) ||
        !getOptionalFiniteDoubleProperty(env, value, "roll", roll) ||
        !getOptionalFiniteDoubleProperty(env, value, "fov", fov)) {
        return false;
    }

    cameraOptions.withCenter(center)
        .withCenterAltitude(centerAltitude)
        .withPadding(padding)
        .withAnchor(anchor)
        .withZoom(zoom)
        .withBearing(bearing)
        .withPitch(pitch)
        .withRoll(roll)
        .withFov(fov);
    return true;
}

bool getFreeCameraOptionsObject(napi_env env, napi_value value, FreeCameraOptions& cameraOptions) {
    if (!isObject(env, value)) {
        return false;
    }

    if (!getOptionalFiniteVectorProperty(env, value, "position", Vector3PropertyNames, cameraOptions.position) ||
        !getOptionalFiniteVectorProperty(
            env, value, "orientation", QuaternionPropertyNames, cameraOptions.orientation)) {
        return false;
    }

    if (cameraOptions.orientation) {
        const auto& q = *cameraOptions.orientation;
        if (q[0] == 0.0 && q[1] == 0.0 && q[2] == 0.0 && q[3] == 0.0) {
            return false;
        }
    }

    return true;
}

bool getBoundOptions(napi_env env, napi_value value, BoundOptions& boundOptions) {
    if (!isObject(env, value)) {
        return false;
    }

    std::optional<LatLngBounds> bounds;
    std::optional<double> minZoom;
    std::optional<double> maxZoom;
    std::optional<double> minPitch;
    std::optional<double> maxPitch;
    if (!getOptionalLatLngBoundsProperty(env, value, "bounds", bounds) ||
        !getOptionalFiniteDoubleProperty(env, value, "minZoom", minZoom) ||
        !getOptionalFiniteDoubleProperty(env, value, "maxZoom", maxZoom) ||
        !getOptionalFiniteDoubleProperty(env, value, "minPitch", minPitch) ||
        !getOptionalFiniteDoubleProperty(env, value, "maxPitch", maxPitch)) {
        return false;
    }
    if (minZoom && maxZoom && *minZoom > *maxZoom) {
        return false;
    }
    if (minPitch && maxPitch && *minPitch > *maxPitch) {
        return false;
    }

    if (bounds) {
        boundOptions.withLatLngBounds(*bounds);
    }
    if (minZoom) {
        boundOptions.withMinZoom(*minZoom);
    }
    if (maxZoom) {
        boundOptions.withMaxZoom(*maxZoom);
    }
    if (minPitch) {
        boundOptions.withMinPitch(*minPitch);
    }
    if (maxPitch) {
        boundOptions.withMaxPitch(*maxPitch);
    }
    return true;
}

bool getCameraBoundsOptions(napi_env env, napi_value value, CameraBoundsOptions& options) {
    if (!isObject(env, value)) {
        return false;
    }

    std::optional<LatLngBounds> bounds;
    std::optional<EdgeInsets> padding;
    std::optional<double> bearing;
    std::optional<double> pitch;
    if (!getOptionalLatLngBoundsProperty(env, value, "bounds", bounds) || !bounds ||
        !getCameraPadding(env, value, padding) || !getOptionalFiniteDoubleProperty(env, value, "bearing", bearing) ||
        !getOptionalFiniteDoubleProperty(env, value, "pitch", pitch)) {
        return false;
    }

    options.bounds = *bounds;
    options.padding = padding.value_or(EdgeInsets());
    options.bearing = bearing;
    options.pitch = pitch;
    return true;
}

bool getAnimationOptions(napi_env env, napi_value value, AnimationOptions& animationOptions) {
    if (isNullOrUndefined(env, value)) {
        return true;
    }
    if (!isObject(env, value)) {
        return false;
    }

    std::optional<double> duration;
    std::optional<double> velocity;
    std::optional<double> minZoom;
    if (!getOptionalFiniteDoubleProperty(env, value, "duration", duration) ||
        !getOptionalFiniteDoubleProperty(env, value, "velocity", velocity) ||
        !getOptionalFiniteDoubleProperty(env, value, "minZoom", minZoom)) {
        return false;
    }

    if (duration) {
        if (*duration < 0.0) {
            return false;
        }
        animationOptions.duration = std::chrono::duration_cast<Duration>(
            std::chrono::duration<double, std::milli>(*duration));
    }
    if (velocity) {
        if (*velocity <= 0.0) {
            return false;
        }
        animationOptions.velocity = *velocity;
    }
    if (minZoom) {
        animationOptions.minZoom = *minZoom;
    }
    return true;
}

napi_value createCameraOptionsObject(napi_env env, const CameraOptions& cameraOptions) {
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (cameraOptions.center) {
        setDoubleProperty(env, object, "longitude", cameraOptions.center->longitude());
        setDoubleProperty(env, object, "latitude", cameraOptions.center->latitude());
    }
    setOptionalDoubleProperty(env, object, "centerAltitude", cameraOptions.centerAltitude);
    if (cameraOptions.padding) {
        setCameraPaddingProperty(env, object, *cameraOptions.padding);
    }
    if (cameraOptions.anchor) {
        setCameraAnchorProperty(env, object, *cameraOptions.anchor);
    }
    setOptionalDoubleProperty(env, object, "zoom", cameraOptions.zoom);
    setOptionalDoubleProperty(env, object, "bearing", cameraOptions.bearing);
    setOptionalDoubleProperty(env, object, "pitch", cameraOptions.pitch);
    setOptionalDoubleProperty(env, object, "roll", cameraOptions.roll);
    setOptionalDoubleProperty(env, object, "fov", cameraOptions.fov);
    return object;
}

napi_value createFreeCameraOptionsObject(napi_env env, const FreeCameraOptions& cameraOptions) {
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (cameraOptions.position) {
        napi_set_named_property(
            env, object, "position", createVectorObject(env, *cameraOptions.position, Vector3PropertyNames));
    }
    if (cameraOptions.orientation) {
        napi_set_named_property(
            env, object, "orientation", createVectorObject(env, *cameraOptions.orientation, QuaternionPropertyNames));
    }
    return object;
}

napi_value createBoundOptionsObject(napi_env env, const BoundOptions& boundOptions) {
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (boundOptions.bounds) {
        setLatLngBoundsProperty(env, object, "bounds", *boundOptions.bounds);
    }
    setOptionalDoubleProperty(env, object, "minZoom", boundOptions.minZoom);
    setOptionalDoubleProperty(env, object, "maxZoom", boundOptions.maxZoom);
    setOptionalDoubleProperty(env, object, "minPitch", boundOptions.minPitch);
    setOptionalDoubleProperty(env, object, "maxPitch", boundOptions.maxPitch);
    return object;
}

napi_value createClientOptionsObject(napi_env env, const std::string& name, const std::string& version) {
    napi_value object = nullptr;
    napi_create_object(env, &object);
    setStringProperty(env, object, "name", name);
    setOptionalStringProperty(env, object, "version", version);
    return object;
}

napi_value createResourceOptionsObject(napi_env env, const ResourceOptions& resourceOptions) {
    napi_value object = nullptr;
    napi_create_object(env, &object);
    setOptionalStringProperty(env, object, "apiKey", resourceOptions.apiKey());
    setOptionalStringProperty(env, object, "cachePath", resourceOptions.cachePath());
    setOptionalStringProperty(env, object, "assetPath", resourceOptions.assetPath());
    return object;
}

napi_value createStringValue(napi_env env, const std::string& value) {
    napi_value result = nullptr;
    napi_create_string_utf8(env, value.c_str(), value.size(), &result);
    return result;
}

napi_value createDebugOptionsObject(napi_env env) {
    napi_value object = nullptr;
    napi_create_object(env, &object);
    setUint32Property(env, object, "NoDebug", static_cast<std::uint32_t>(MapDebugOptions::NoDebug));
    setUint32Property(env, object, "TileBorders", static_cast<std::uint32_t>(MapDebugOptions::TileBorders));
    setUint32Property(env, object, "ParseStatus", static_cast<std::uint32_t>(MapDebugOptions::ParseStatus));
    setUint32Property(env, object, "Timestamps", static_cast<std::uint32_t>(MapDebugOptions::Timestamps));
    setUint32Property(env, object, "Collision", static_cast<std::uint32_t>(MapDebugOptions::Collision));
    setUint32Property(env, object, "Overdraw", static_cast<std::uint32_t>(MapDebugOptions::Overdraw));
    setUint32Property(env, object, "StencilClip", static_cast<std::uint32_t>(MapDebugOptions::StencilClip));
    setUint32Property(env, object, "DepthBuffer", static_cast<std::uint32_t>(MapDebugOptions::DepthBuffer));
    return object;
}

} // namespace ohos
} // namespace mbgl
