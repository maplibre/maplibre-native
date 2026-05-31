#include "native_values.hpp"

#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

namespace mbgl {
namespace ohos {
namespace {

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

bool getBool(napi_env env, napi_value value, bool& result) {
    return value != nullptr && napi_get_value_bool(env, value, &result) == napi_ok;
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

napi_value createStringValue(napi_env env, const std::string& value) {
    napi_value result = nullptr;
    napi_create_string_utf8(env, value.c_str(), value.size(), &result);
    return result;
}

} // namespace ohos
} // namespace mbgl
