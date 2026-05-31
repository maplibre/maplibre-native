#include "map_view.hpp"

#include "gesture_handler.hpp"
#include "native_values.hpp"

#include <mbgl/storage/resource_options.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/size.hpp>

#include <cmath>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <ace/xcomponent/native_interface_xcomponent.h>
#include <napi/native_api.h>
#include <native_window/external_window.h>

namespace {

using mbgl::ohos::createCameraOptionsObject;
using mbgl::ohos::createStringValue;
using mbgl::ohos::getBool;
using mbgl::ohos::getBoundOptions;
using mbgl::ohos::getCameraOptionsObject;
using mbgl::ohos::getDouble;
using mbgl::ohos::getOptionalStringProperty;
using mbgl::ohos::getRequiredInt32Property;
using mbgl::ohos::getString;
using mbgl::ohos::isNullOrUndefined;
using mbgl::ohos::isObject;

struct SurfaceBinding {
    OH_NativeXComponent* legacyComponent = nullptr;
    OHNativeWindow* window = nullptr;
    std::unique_ptr<mbgl::ohos::MapView> mapView;
    std::string style;
    std::uint64_t styleGeneration = 0;
    std::uint64_t appliedStyleGeneration = 0;
    float pixelRatio = 1.0f;
    std::uint64_t width = 0;
    std::uint64_t height = 0;
    bool renderingEnabled = true;
    bool legacyFrameRegistered = false;
    std::uint64_t frameCallbackCount = 0;
    bool surfaceVisible = false;
    std::string lastSurfaceError;
    mbgl::ohos::GestureState gesture;
    std::optional<OH_NativeXComponent_ExpectedRateRange> frameRateRange;
    std::chrono::steady_clock::time_point frameRateSampleTime;
    std::uint64_t frameRateRenderedFrames = 0;
    std::uint64_t frameRateCallbacks = 0;
    double renderedFrameRate = 0.0;
    double frameCallbackRate = 0.0;
};

std::unordered_map<OH_NativeXComponent*, std::unique_ptr<SurfaceBinding>>& legacyBindings() {
    static std::unordered_map<OH_NativeXComponent*, std::unique_ptr<SurfaceBinding>> bindings;
    return bindings;
}

std::uint32_t toSizeDimension(std::uint64_t value) {
    constexpr auto maxDimension = static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max());
    if (value > maxDimension) {
        return std::numeric_limits<std::uint32_t>::max();
    }
    return static_cast<std::uint32_t>(value);
}

mbgl::Size toSize(std::uint64_t width, std::uint64_t height) {
    return {toSizeDimension(width), toSizeDimension(height)};
}

bool exposedResourceOptionsEqual(const mbgl::ResourceOptions& lhs, const mbgl::ResourceOptions& rhs) {
    return lhs.apiKey() == rhs.apiKey() && lhs.cachePath() == rhs.cachePath() &&
           lhs.assetPath() == rhs.assetPath();
}

void clearBindingSurface(SurfaceBinding& binding) {
    if (binding.mapView) {
        if (mbgl::ohos::hasActiveGesture(binding.gesture)) {
            binding.mapView->setGestureInProgress(false);
        }
        binding.mapView->clearSurface();
    }
    binding.appliedStyleGeneration = 0;
    mbgl::ohos::resetGestureState(binding.gesture);
}

void clearBindingMapState(SurfaceBinding& binding) {
    clearBindingSurface(binding);
    binding.window = nullptr;
    binding.width = 0;
    binding.height = 0;
    binding.mapView.reset();
}

void applyDesiredStyle(SurfaceBinding& binding) {
    if (!binding.mapView || !binding.mapView->hasMap() || binding.style.empty() ||
        binding.appliedStyleGeneration == binding.styleGeneration) {
        return;
    }

    binding.mapView->setStyleURL(binding.style);
    binding.appliedStyleGeneration = binding.styleGeneration;
}

void renderBindingFrame(SurfaceBinding& binding) {
    if (binding.renderingEnabled && binding.surfaceVisible && binding.mapView) {
        binding.mapView->renderFrame();
    }
}

mbgl::ohos::MapView& ensureMapView(SurfaceBinding& binding) {
    if (!binding.mapView) {
        binding.mapView = std::make_unique<mbgl::ohos::MapView>(binding.pixelRatio);
    }
    return *binding.mapView;
}

void updateSurface(SurfaceBinding& binding) {
    auto& mapView = ensureMapView(binding);

    if (binding.window == nullptr || binding.width == 0 || binding.height == 0) {
        clearBindingSurface(binding);
        return;
    }

    try {
        const bool needsStyleReapply = !mapView.hasMap() || mapView.getNativeWindow() != binding.window;
        mapView.setSurface(binding.window, toSize(binding.width, binding.height));
        if (needsStyleReapply) {
            binding.appliedStyleGeneration = 0;
        }
        applyDesiredStyle(binding);
        renderBindingFrame(binding);
        binding.lastSurfaceError.clear();
    } catch (const std::exception& exception) {
        binding.lastSurfaceError = exception.what();
        clearBindingSurface(binding);
        mbgl::Log::Error(mbgl::Event::Render, exception.what());
    }
}

SurfaceBinding* findLegacyBinding(OH_NativeXComponent* component) {
    const auto it = legacyBindings().find(component);
    if (it == legacyBindings().end()) {
        return nullptr;
    }
    return it->second.get();
}

void updateLegacySurface(OH_NativeXComponent* component, void* window) {
    auto* binding = findLegacyBinding(component);
    if (binding == nullptr || window == nullptr) {
        return;
    }

    std::uint64_t width = 0;
    std::uint64_t height = 0;
    if (OH_NativeXComponent_GetXComponentSize(component, window, &width, &height) !=
        OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        mbgl::Log::Warning(mbgl::Event::OpenGL, "Could not query legacy XComponent surface size");
        return;
    }

    binding->window = static_cast<OHNativeWindow*>(window);
    binding->width = width;
    binding->height = height;
    binding->surfaceVisible = true;
    updateSurface(*binding);
}

napi_value getUndefined(napi_env env) {
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value throwError(napi_env env, const char* message) {
    napi_throw_error(env, nullptr, message);
    return getUndefined(env);
}

OH_NativeXComponent* getLegacyComponent(napi_env env, napi_value value) {
    if (value == nullptr) {
        return nullptr;
    }

    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, value, &type) != napi_ok || type != napi_object) {
        return nullptr;
    }

    bool hasXComponent = false;
    if (napi_has_named_property(env, value, OH_NATIVE_XCOMPONENT_OBJ, &hasXComponent) != napi_ok || !hasXComponent) {
        return nullptr;
    }

    napi_value xcomponent = nullptr;
    if (napi_get_named_property(env, value, OH_NATIVE_XCOMPONENT_OBJ, &xcomponent) != napi_ok ||
        xcomponent == nullptr) {
        return nullptr;
    }

    void* nativeXComponent = nullptr;
    if (napi_unwrap(env, xcomponent, &nativeXComponent) != napi_ok || nativeXComponent == nullptr) {
        return nullptr;
    }

    return static_cast<OH_NativeXComponent*>(nativeXComponent);
}

SurfaceBinding* getLegacyBinding(napi_env env, napi_value value) {
    auto* component = getLegacyComponent(env, value);
    return component != nullptr ? findLegacyBinding(component) : nullptr;
}

SurfaceBinding* resolveBinding(napi_env env, napi_value thisArg) {
    auto* binding = getLegacyBinding(env, thisArg);
    if (binding == nullptr) {
        throwError(env, "Expected a native XComponent context");
    }
    return binding;
}

bool isValidFrameRateRange(const OH_NativeXComponent_ExpectedRateRange& range) {
    return range.min > 0 && range.max >= range.min && range.expected >= range.min && range.expected <= range.max;
}

bool parseFrameRateRange(napi_env env, napi_value value, OH_NativeXComponent_ExpectedRateRange& range) {
    return isObject(env, value) && getRequiredInt32Property(env, value, "min", range.min) &&
           getRequiredInt32Property(env, value, "max", range.max) &&
           getRequiredInt32Property(env, value, "expected", range.expected) && isValidFrameRateRange(range);
}

napi_value createStringArray(napi_env env, const std::vector<std::string>& values) {
    napi_value array = nullptr;
    napi_create_array_with_length(env, values.size(), &array);
    for (std::size_t i = 0; i < values.size(); ++i) {
        napi_set_element(env, array, static_cast<std::uint32_t>(i), createStringValue(env, values[i]));
    }
    return array;
}

void setBoolProperty(napi_env env, napi_value object, const char* name, bool value) {
    napi_value property = nullptr;
    napi_get_boolean(env, value, &property);
    napi_set_named_property(env, object, name, property);
}

void setSizeProperty(napi_env env, napi_value object, const char* name, std::uint64_t value) {
    napi_value property = nullptr;
    napi_create_double(env, static_cast<double>(value), &property);
    napi_set_named_property(env, object, name, property);
}

void setDoubleProperty(napi_env env, napi_value object, const char* name, double value) {
    napi_value property = nullptr;
    napi_create_double(env, value, &property);
    napi_set_named_property(env, object, name, property);
}

void setStringProperty(napi_env env, napi_value object, const char* name, const std::string& value) {
    napi_set_named_property(env, object, name, createStringValue(env, value));
}

void updateFrameRates(SurfaceBinding& binding) {
    const auto now = std::chrono::steady_clock::now();
    const auto renderedFrames = binding.mapView ? binding.mapView->getRenderedFrameCount() : 0;
    const auto frameCallbacks = binding.frameCallbackCount;

    if (binding.frameRateSampleTime.time_since_epoch().count() == 0) {
        binding.frameRateSampleTime = now;
        binding.frameRateRenderedFrames = renderedFrames;
        binding.frameRateCallbacks = frameCallbacks;
        return;
    }

    const auto elapsed = std::chrono::duration<double>(now - binding.frameRateSampleTime).count();
    if (elapsed <= 0.0) {
        return;
    }

    binding.renderedFrameRate = static_cast<double>(renderedFrames - binding.frameRateRenderedFrames) / elapsed;
    binding.frameCallbackRate = static_cast<double>(frameCallbacks - binding.frameRateCallbacks) / elapsed;
    binding.frameRateSampleTime = now;
    binding.frameRateRenderedFrames = renderedFrames;
    binding.frameRateCallbacks = frameCallbacks;
}

napi_value createSurfaceStateObject(napi_env env, SurfaceBinding& binding) {
    updateFrameRates(binding);

    const bool hasWindow = binding.window != nullptr;
    const bool hasSurface = hasWindow && binding.width > 0 && binding.height > 0;
    const bool hasMap = binding.mapView && binding.mapView->hasMap();
    const bool needsRender = binding.mapView && binding.mapView->hasPendingRender();
    const bool styleLoaded = binding.mapView && binding.mapView->hasLoadedStyle();
    const bool mapLoaded = binding.mapView && binding.mapView->hasLoadedMap();
    const bool fullyLoaded = binding.mapView && binding.mapView->isFullyLoaded();
    std::string backendLabel;
    if (binding.mapView && binding.mapView->getGlesContextClientVersion() > 0) {
        backendLabel = std::string{"OpenGL ES "} + std::to_string(binding.mapView->getGlesContextClientVersion());
    } else if (binding.mapView && !binding.mapView->getRendererDiagnostic().empty()) {
        backendLabel = "Vulkan";
    }

    napi_value object = nullptr;
    napi_create_object(env, &object);
    setSizeProperty(env, object, "width", binding.width);
    setSizeProperty(env, object, "height", binding.height);
    setBoolProperty(env, object, "hasWindow", hasWindow);
    setBoolProperty(env, object, "hasSurface", hasSurface);
    setBoolProperty(env, object, "hasMap", hasMap);
    setBoolProperty(env, object, "needsRender", needsRender);
    setBoolProperty(env, object, "styleLoaded", styleLoaded);
    setBoolProperty(env, object, "mapLoaded", mapLoaded);
    setBoolProperty(env, object, "fullyLoaded", fullyLoaded);
    setDoubleProperty(env, object, "renderedFrameRate", binding.renderedFrameRate);
    setDoubleProperty(env, object, "frameCallbackRate", binding.frameCallbackRate);
    if (!backendLabel.empty()) {
        setStringProperty(env, object, "backend", backendLabel);
    }
    setBoolProperty(env, object, "surfaceVisible", binding.surfaceVisible);
    if (!binding.lastSurfaceError.empty()) {
        setStringProperty(env, object, "lastSurfaceError", binding.lastSurfaceError);
    }
    if (binding.mapView && !binding.mapView->getLastMapLoadError().empty()) {
        setStringProperty(env, object, "lastMapLoadError", binding.mapView->getLastMapLoadError());
    }
    if (binding.mapView && !binding.mapView->getLastRenderError().empty()) {
        setStringProperty(env, object, "lastRenderError", binding.mapView->getLastRenderError());
    }
    if (binding.mapView && !binding.mapView->getLastStyleImageMissing().empty()) {
        setStringProperty(env, object, "lastStyleImageMissing", binding.mapView->getLastStyleImageMissing());
    }
    if (binding.mapView && !binding.mapView->getLastGlyphsError().empty()) {
        setStringProperty(env, object, "lastGlyphsError", binding.mapView->getLastGlyphsError());
    }
    if (binding.mapView && !binding.mapView->getLastSpritesError().empty()) {
        setStringProperty(env, object, "lastSpritesError", binding.mapView->getLastSpritesError());
    }
    return object;
}

bool applyFrameRateRange(SurfaceBinding& binding, const OH_NativeXComponent_ExpectedRateRange& range) {
    if (binding.legacyComponent != nullptr) {
        auto mutableRange = range;
        return OH_NativeXComponent_SetExpectedFrameRateRange(binding.legacyComponent, &mutableRange) ==
               OH_NATIVEXCOMPONENT_RESULT_SUCCESS;
    }

    return false;
}

void onLegacySurfaceCreated(OH_NativeXComponent* component, void* window) {
    updateLegacySurface(component, window);
}

void onLegacySurfaceChanged(OH_NativeXComponent* component, void* window) {
    updateLegacySurface(component, window);
}

void onLegacySurfaceDestroyed(OH_NativeXComponent* component, void*) {
    auto* binding = findLegacyBinding(component);
    if (binding == nullptr) {
        return;
    }

    binding->surfaceVisible = false;
    clearBindingSurface(*binding);
    binding->window = nullptr;
    binding->width = 0;
    binding->height = 0;
}

void onLegacyTouchEvent(OH_NativeXComponent* component, void* window) {
    auto* binding = findLegacyBinding(component);
    if (binding == nullptr || window == nullptr) {
        return;
    }

    OH_NativeXComponent_TouchEvent event{};
    if (OH_NativeXComponent_GetTouchEvent(component, window, &event) != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        mbgl::Log::Warning(mbgl::Event::General, "Could not query legacy XComponent touch event");
        return;
    }

    mbgl::ohos::handleTouchEvent(binding->gesture, binding->mapView.get(), event);
}

void onLegacyFrame(OH_NativeXComponent* component, std::uint64_t, std::uint64_t) {
    auto* binding = findLegacyBinding(component);
    if (binding == nullptr) {
        return;
    }

    ++binding->frameCallbackCount;
    renderBindingFrame(*binding);
}

OH_NativeXComponent_Callback& legacyXComponentCallback() {
    static OH_NativeXComponent_Callback callback{
        onLegacySurfaceCreated,
        onLegacySurfaceChanged,
        onLegacySurfaceDestroyed,
        onLegacyTouchEvent,
    };
    return callback;
}

void registerLegacyExportedXComponent(napi_env env, napi_value exports) {
    bool hasXComponent = false;
    if (napi_has_named_property(env, exports, OH_NATIVE_XCOMPONENT_OBJ, &hasXComponent) != napi_ok || !hasXComponent) {
        return;
    }

    napi_value xcomponent = nullptr;
    if (napi_get_named_property(env, exports, OH_NATIVE_XCOMPONENT_OBJ, &xcomponent) != napi_ok ||
        xcomponent == nullptr) {
        return;
    }

    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, xcomponent, &type) != napi_ok || type != napi_object) {
        return;
    }

    void* nativeXComponent = nullptr;
    if (napi_unwrap(env, xcomponent, &nativeXComponent) != napi_ok || nativeXComponent == nullptr) {
        return;
    }

    auto* component = static_cast<OH_NativeXComponent*>(nativeXComponent);
    auto& bindings = legacyBindings();
    if (bindings.find(component) == bindings.end()) {
        auto binding = std::make_unique<SurfaceBinding>();
        binding->legacyComponent = component;
        bindings.emplace(component, std::move(binding));
    }

    if (OH_NativeXComponent_RegisterCallback(component, &legacyXComponentCallback()) !=
        OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        mbgl::Log::Warning(mbgl::Event::Setup, "Could not register legacy XComponent surface callbacks");
    }

    if (OH_NativeXComponent_RegisterOnFrameCallback(component, onLegacyFrame) !=
        OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        mbgl::Log::Warning(mbgl::Event::Setup, "Could not register legacy XComponent frame callback");
    } else if (auto* binding = findLegacyBinding(component)) {
        binding->legacyFrameRegistered = true;
    }

    if (auto* binding = findLegacyBinding(component); binding != nullptr && binding->frameRateRange) {
        if (!applyFrameRateRange(*binding, *binding->frameRateRange)) {
            mbgl::Log::Warning(mbgl::Event::Setup, "Could not apply legacy XComponent frame-rate range");
        }
    }
}

void disposeLegacyBinding(SurfaceBinding* binding) {
    if (binding == nullptr || binding->legacyComponent == nullptr) {
        return;
    }

    auto* component = binding->legacyComponent;
    if (binding->legacyFrameRegistered &&
        OH_NativeXComponent_UnregisterOnFrameCallback(component) != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        mbgl::Log::Warning(mbgl::Event::Setup, "Could not unregister legacy XComponent frame callback");
    }
    binding->legacyFrameRegistered = false;
    clearBindingMapState(*binding);

    const auto it = legacyBindings().find(component);
    if (it != legacyBindings().end() && it->second.get() == binding) {
        legacyBindings().erase(it);
    }
}

napi_value destroy(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent context");
    }

    if (auto* component = getLegacyComponent(env, thisArg)) {
        if (auto* binding = findLegacyBinding(component)) {
            disposeLegacyBinding(binding);
        }
        return getUndefined(env);
    }

    return throwError(env, "Expected a native XComponent context");
}

napi_value renderFrame(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent context");
    }

    auto* binding = resolveBinding(env, thisArg);
    if (binding == nullptr) {
        return getUndefined(env);
    }

    renderBindingFrame(*binding);

    return getUndefined(env);
}

napi_value reduceMemoryUse(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent context");
    }

    auto* binding = resolveBinding(env, thisArg);
    if (binding == nullptr) {
        return getUndefined(env);
    }

    if (binding->mapView) {
        binding->mapView->reduceMemoryUse();
    }

    return getUndefined(env);
}

napi_value setStyleUrl(napi_env env, napi_callback_info info) {
    std::size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected style URL");
    }

    auto* binding = resolveBinding(env, thisArg);
    if (binding == nullptr) {
        return getUndefined(env);
    }

    std::string style;
    if (!getString(env, argv[0], style)) {
        return throwError(env, "Expected style URL string");
    }

    binding->style = std::move(style);
    ++binding->styleGeneration;
    applyDesiredStyle(*binding);

    return getUndefined(env);
}

napi_value jumpTo(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected camera options");
    }

    auto* binding = resolveBinding(env, thisArg);
    if (binding == nullptr) {
        return getUndefined(env);
    }
    mbgl::CameraOptions cameraOptions;
    if (!getCameraOptionsObject(env, argv[0], cameraOptions)) {
        return throwError(env, "Expected valid camera options object");
    }

    try {
        ensureMapView(*binding).jumpTo(std::move(cameraOptions));
        renderBindingFrame(*binding);
    } catch (const std::exception& exception) {
        return throwError(env, exception.what());
    }

    return getUndefined(env);
}

napi_value setPixelRatio(napi_env env, napi_callback_info info) {
    std::size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected a pixel ratio");
    }

    auto* binding = resolveBinding(env, thisArg);
    if (binding == nullptr) {
        return getUndefined(env);
    }

    double pixelRatio = 0.0;
    if (!getDouble(env, argv[0], pixelRatio) || !std::isfinite(pixelRatio) || pixelRatio <= 0.0) {
        return throwError(env, "Expected a finite positive pixel ratio");
    }

    try {
        const auto newPixelRatio = static_cast<float>(pixelRatio);
        if (!binding->mapView) {
            if (binding->pixelRatio == newPixelRatio) {
                return getUndefined(env);
            }
            binding->mapView = std::make_unique<mbgl::ohos::MapView>(newPixelRatio);
        } else if (binding->mapView->getPixelRatio() == newPixelRatio) {
            return getUndefined(env);
        }
        binding->mapView->setPixelRatio(newPixelRatio);
        binding->pixelRatio = newPixelRatio;
        binding->appliedStyleGeneration = 0;
        applyDesiredStyle(*binding);
        renderBindingFrame(*binding);
    } catch (const std::exception& exception) {
        return throwError(env, exception.what());
    }

    return getUndefined(env);
}

napi_value setBounds(napi_env env, napi_callback_info info) {
    std::size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected bounds options");
    }

    auto* binding = resolveBinding(env, thisArg);
    if (binding == nullptr) {
        return getUndefined(env);
    }

    mbgl::BoundOptions boundOptions;
    if (!getBoundOptions(env, argv[0], boundOptions)) {
        return throwError(env, "Expected valid bounds options object");
    }

    try {
        ensureMapView(*binding).setBounds(std::move(boundOptions));
        renderBindingFrame(*binding);
    } catch (const std::exception& exception) {
        return throwError(env, exception.what());
    }

    return getUndefined(env);
}

napi_value setRenderingEnabled(napi_env env, napi_callback_info info) {
    std::size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected rendering enabled boolean");
    }

    auto* binding = resolveBinding(env, thisArg);
    if (binding == nullptr) {
        return getUndefined(env);
    }

    bool enabled = false;
    if (!getBool(env, argv[0], enabled)) {
        return throwError(env, "Expected rendering enabled boolean");
    }

    binding->renderingEnabled = enabled;
    if (enabled) {
        renderBindingFrame(*binding);
    }

    return getUndefined(env);
}

napi_value setFrameRateRange(napi_env env, napi_callback_info info) {
    std::size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected frame-rate range");
    }

    auto* binding = resolveBinding(env, thisArg);
    if (binding == nullptr) {
        return getUndefined(env);
    }

    OH_NativeXComponent_ExpectedRateRange range{};
    if (!parseFrameRateRange(env, argv[0], range)) {
        return throwError(env, "Expected frame-rate range object with positive min <= expected <= max");
    }

    if (!applyFrameRateRange(*binding, range)) {
        return throwError(env, "Could not apply XComponent frame-rate range");
    }

    binding->frameRateRange = range;
    return getUndefined(env);
}

napi_value setTileCacheEnabled(napi_env env, napi_callback_info info) {
    std::size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected tile cache enabled boolean");
    }

    auto* binding = resolveBinding(env, thisArg);
    if (binding == nullptr) {
        return getUndefined(env);
    }

    bool enabled = false;
    if (!getBool(env, argv[0], enabled)) {
        return throwError(env, "Expected tile cache enabled boolean");
    }

    ensureMapView(*binding).setTileCacheEnabled(enabled);

    return getUndefined(env);
}

napi_value setClientOptions(napi_env env, napi_callback_info info) {
    std::size_t argc = 3;
    napi_value argv[3] = {nullptr, nullptr, nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected client name and optional client version");
    }

    auto* binding = resolveBinding(env, thisArg);
    if (binding == nullptr) {
        return getUndefined(env);
    }

    std::string clientName;
    if (!getString(env, argv[0], clientName)) {
        return throwError(env, "Expected client name string");
    }

    std::string clientVersion;
    if (argc > 1 && !isNullOrUndefined(env, argv[1])) {
        if (!getString(env, argv[1], clientVersion)) {
            return throwError(env, "Expected client version string");
        }
    }

    try {
        if (!binding->mapView && clientName.empty() && clientVersion.empty()) {
            return getUndefined(env);
        }
        if (binding->mapView && binding->mapView->getClientName() == clientName &&
            binding->mapView->getClientVersion() == clientVersion) {
            return getUndefined(env);
        }
        ensureMapView(*binding).setClientOptions(std::move(clientName), std::move(clientVersion));
        binding->appliedStyleGeneration = 0;
        applyDesiredStyle(*binding);
        renderBindingFrame(*binding);
    } catch (const std::exception& exception) {
        return throwError(env, exception.what());
    }

    return getUndefined(env);
}

napi_value setResourceOptions(napi_env env, napi_callback_info info) {
    std::size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected resource options");
    }

    auto* binding = resolveBinding(env, thisArg);
    if (binding == nullptr) {
        return getUndefined(env);
    }
    if (!isObject(env, argv[0])) {
        return throwError(env, "Expected resource options object");
    }

    std::optional<std::string> apiKey;
    std::optional<std::string> cachePath;
    std::optional<std::string> assetPath;
    if (!getOptionalStringProperty(env, argv[0], "apiKey", apiKey) ||
        !getOptionalStringProperty(env, argv[0], "cachePath", cachePath) ||
        !getOptionalStringProperty(env, argv[0], "assetPath", assetPath)) {
        return throwError(env, "Expected string resource option values");
    }

    try {
        if (!binding->mapView && !apiKey && !cachePath && !assetPath) {
            return getUndefined(env);
        }
        auto& mapView = ensureMapView(*binding);
        auto resourceOptions = mapView.getResourceOptions().clone();
        if (apiKey) {
            resourceOptions.withApiKey(*apiKey);
        }
        if (cachePath) {
            resourceOptions.withCachePath(*cachePath);
        }
        if (assetPath) {
            resourceOptions.withAssetPath(*assetPath);
        }

        if (exposedResourceOptionsEqual(resourceOptions, mapView.getResourceOptions())) {
            return getUndefined(env);
        }

        mapView.setResourceOptions(std::move(resourceOptions));
        binding->appliedStyleGeneration = 0;
        applyDesiredStyle(*binding);
        renderBindingFrame(*binding);
    } catch (const std::exception& exception) {
        return throwError(env, exception.what());
    }

    return getUndefined(env);
}

napi_value getPixelRatio(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent context");
    }

    auto* binding = resolveBinding(env, thisArg);
    if (binding == nullptr) {
        return getUndefined(env);
    }

    const auto pixelRatio = binding->mapView ? binding->mapView->getPixelRatio() : binding->pixelRatio;
    napi_value result = nullptr;
    napi_create_double(env, pixelRatio, &result);
    return result;
}

napi_value getStyleAttributions(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent context");
    }

    auto* binding = resolveBinding(env, thisArg);
    if (binding == nullptr) {
        return getUndefined(env);
    }

    return createStringArray(env, ensureMapView(*binding).getStyleAttributions());
}

napi_value getSurfaceState(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent context");
    }

    auto* binding = resolveBinding(env, thisArg);
    if (binding == nullptr) {
        return getUndefined(env);
    }

    return createSurfaceStateObject(env, *binding);
}

napi_value getCameraOptions(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent context");
    }

    auto* binding = resolveBinding(env, thisArg);
    if (binding == nullptr) {
        return getUndefined(env);
    }

    return createCameraOptionsObject(env, ensureMapView(*binding).getCameraOptions());
}

napi_value Init(napi_env env, napi_value exports) {
    registerLegacyExportedXComponent(env, exports);

    napi_property_descriptor properties[] = {
        {"destroy", nullptr, destroy, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getCameraOptions", nullptr, getCameraOptions, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getPixelRatio", nullptr, getPixelRatio, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getStyleAttributions", nullptr, getStyleAttributions, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getSurfaceState", nullptr, getSurfaceState, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"jumpTo", nullptr, jumpTo, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"reduceMemoryUse", nullptr, reduceMemoryUse, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"renderFrame", nullptr, renderFrame, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setBounds", nullptr, setBounds, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setClientOptions", nullptr, setClientOptions, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setFrameRateRange", nullptr, setFrameRateRange, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setPixelRatio", nullptr, setPixelRatio, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setRenderingEnabled", nullptr, setRenderingEnabled, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setResourceOptions", nullptr, setResourceOptions, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setTileCacheEnabled", nullptr, setTileCacheEnabled, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setStyleUrl", nullptr, setStyleUrl, nullptr, nullptr, nullptr, napi_default, nullptr},
    };

    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    return exports;
}

} // namespace

NAPI_MODULE(maplibre_native_ohos, Init)
