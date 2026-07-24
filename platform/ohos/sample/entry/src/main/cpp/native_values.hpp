#pragma once

#include <mbgl/map/bound_options.hpp>
#include <mbgl/map/camera.hpp>
#include <mbgl/map/mode.hpp>

#include <cstdint>
#include <optional>
#include <string>

#include <napi/native_api.h>

namespace mbgl {
namespace ohos {

bool getString(napi_env env, napi_value value, std::string& result);
bool isNullOrUndefined(napi_env env, napi_value value);
bool getDouble(napi_env env, napi_value value, double& result);
bool getBool(napi_env env, napi_value value, bool& result);
bool isObject(napi_env env, napi_value value);
bool getOptionalStringProperty(napi_env env, napi_value object, const char* name, std::optional<std::string>& result);
bool getRequiredInt32Property(napi_env env, napi_value object, const char* name, std::int32_t& result);

bool getCameraOptionsObject(napi_env env, napi_value value, CameraOptions& cameraOptions);
bool getBoundOptions(napi_env env, napi_value value, BoundOptions& boundOptions);

napi_value createCameraOptionsObject(napi_env env, const CameraOptions& cameraOptions);
napi_value createStringValue(napi_env env, const std::string& value);

} // namespace ohos
} // namespace mbgl
