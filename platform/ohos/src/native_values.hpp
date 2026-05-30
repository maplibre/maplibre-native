#pragma once

#include "camera_bounds_options.hpp"

#include <mbgl/map/bound_options.hpp>
#include <mbgl/map/camera.hpp>
#include <mbgl/map/mode.hpp>
#include <mbgl/storage/resource_options.hpp>

#include <cstdint>
#include <optional>
#include <string>

#include <napi/native_api.h>

namespace mbgl {
namespace ohos {

bool getString(napi_env env, napi_value value, std::string& result);
bool isNullOrUndefined(napi_env env, napi_value value);
bool getDouble(napi_env env, napi_value value, double& result);
bool getUint32(napi_env env, napi_value value, std::uint32_t& result);
bool getBool(napi_env env, napi_value value, bool& result);
bool isValidDebugOptions(std::uint32_t options);
bool isObject(napi_env env, napi_value value);
bool getOptionalStringProperty(napi_env env, napi_value object, const char* name, std::optional<std::string>& result);
bool getRequiredInt32Property(napi_env env, napi_value object, const char* name, std::int32_t& result);

bool getCameraOptionsObject(napi_env env, napi_value value, CameraOptions& cameraOptions);
bool getFreeCameraOptionsObject(napi_env env, napi_value value, FreeCameraOptions& cameraOptions);
bool getBoundOptions(napi_env env, napi_value value, BoundOptions& boundOptions);
bool getCameraBoundsOptions(napi_env env, napi_value value, CameraBoundsOptions& options);
bool getAnimationOptions(napi_env env, napi_value value, AnimationOptions& animationOptions);

napi_value createCameraOptionsObject(napi_env env, const CameraOptions& cameraOptions);
napi_value createFreeCameraOptionsObject(napi_env env, const FreeCameraOptions& cameraOptions);
napi_value createBoundOptionsObject(napi_env env, const BoundOptions& boundOptions);
napi_value createClientOptionsObject(napi_env env, const std::string& name, const std::string& version);
napi_value createResourceOptionsObject(napi_env env, const ResourceOptions& resourceOptions);
napi_value createStringValue(napi_env env, const std::string& value);
napi_value createDebugOptionsObject(napi_env env);

} // namespace ohos
} // namespace mbgl
