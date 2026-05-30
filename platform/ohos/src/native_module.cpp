#include "map_view.hpp"

#include "gesture_handler.hpp"
#include "native_values.hpp"

#include <mbgl/storage/resource_options.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/size.hpp>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <limits>
#include <memory>
#include <new>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

#include <hilog/log.h>

#include <ace/xcomponent/native_interface_xcomponent.h>
#include <arkui/native_node.h>
#include <arkui/native_node_napi.h>
#include <arkui/ui_input_event.h>
#include <napi/native_api.h>
#include <native_window/external_window.h>

namespace {

constexpr unsigned int kMapLibreHilogDomain = 0x4d4c4e;
constexpr char kMapLibreHilogTag[] = "MapLibreNative";
constexpr std::uint64_t kDefaultDiagnosticsLogIntervalFrames = 180;

using mbgl::ohos::createBoundOptionsObject;
using mbgl::ohos::createCameraOptionsObject;
using mbgl::ohos::createClientOptionsObject;
using mbgl::ohos::createDebugOptionsObject;
using mbgl::ohos::createFreeCameraOptionsObject;
using mbgl::ohos::createResourceOptionsObject;
using mbgl::ohos::createStringValue;
using mbgl::ohos::getAnimationOptions;
using mbgl::ohos::getBool;
using mbgl::ohos::getBoundOptions;
using mbgl::ohos::getCameraBoundsOptions;
using mbgl::ohos::getCameraOptionsObject;
using mbgl::ohos::getDouble;
using mbgl::ohos::getFreeCameraOptionsObject;
using mbgl::ohos::getOptionalStringProperty;
using mbgl::ohos::getRequiredInt32Property;
using mbgl::ohos::getString;
using mbgl::ohos::getUint32;
using mbgl::ohos::isNullOrUndefined;
using mbgl::ohos::isObject;
using mbgl::ohos::isValidDebugOptions;

enum class StyleKind {
    None,
    URL,
    JSON,
};

struct SurfaceBinding {
    OH_ArkUI_SurfaceHolder* holder = nullptr;
    OH_ArkUI_SurfaceCallback* callback = nullptr;
    ArkUI_NodeHandle node = nullptr;
    OH_NativeXComponent* legacyComponent = nullptr;
    OHNativeWindow* window = nullptr;
    std::unique_ptr<mbgl::ohos::MapView> mapView;
    StyleKind styleKind = StyleKind::None;
    std::string style;
    std::uint64_t styleGeneration = 0;
    std::uint64_t appliedStyleGeneration = 0;
    float pixelRatio = 1.0f;
    std::uint64_t width = 0;
    std::uint64_t height = 0;
    bool renderingEnabled = true;
    bool nodeFrameRegistered = false;
    bool nodeTouchRegistered = false;
    bool legacyFrameRegistered = false;
    std::uint64_t frameCallbackCount = 0;
    std::uint64_t touchEventCount = 0;
    std::uint64_t gestureHandledCount = 0;
    bool surfaceVisible = false;
    std::uint64_t surfaceCreatedCount = 0;
    std::uint64_t surfaceChangedCount = 0;
    std::uint64_t surfaceDestroyedCount = 0;
    std::uint64_t surfaceShownCount = 0;
    std::uint64_t surfaceHiddenCount = 0;
    std::uint64_t surfaceErrorCount = 0;
    std::string lastSurfaceError;
    mbgl::ohos::GestureState gesture;
    std::optional<OH_NativeXComponent_ExpectedRateRange> frameRateRange;
    std::uint64_t diagnosticsLogIntervalFrames = kDefaultDiagnosticsLogIntervalFrames;
    std::uint64_t diagnosticsLogFrameCounter = 0;
};

const char* styleKindLabel(StyleKind kind) {
    switch (kind) {
        case StyleKind::URL:
            return "url";
        case StyleKind::JSON:
            return "json";
        default:
            return "none";
    }
}

std::string formatSurfaceDiagnostics(const SurfaceBinding& binding) {
    std::ostringstream stream;
    const bool hasMap = binding.mapView && binding.mapView->hasMap();
    stream << "surface=" << binding.width << 'x' << binding.height << " pr=" << binding.pixelRatio
           << " window=" << (binding.window != nullptr) << " map=" << hasMap
           << " visible=" << binding.surfaceVisible << " render=" << binding.renderingEnabled;
    if (binding.mapView) {
        stream << " frames=" << binding.mapView->getRenderedFrameCount() << '/'
               << binding.mapView->getCoreFrameCount() << " gl=" << binding.mapView->getGlesContextClientVersion()
               << " cb=" << binding.frameCallbackCount << " style=" << (binding.mapView->hasLoadedStyle() ? "loaded" : "pending")
               << " mapLoaded=" << (binding.mapView->hasLoadedMap() ? "yes" : "no")
               << " idle=" << (binding.mapView->isIdle() ? "yes" : "no")
               << " fullyLoaded=" << (binding.mapView->isFullyLoaded() ? "yes" : "no")
               << " needsRender=" << (binding.mapView->hasPendingRender() ? "yes" : "no")
               << " repaint=" << (binding.mapView->lastFrameNeededRepaint() ? "yes" : "no")
               << " frameComplete=" << (binding.mapView->lastFrameWasComplete() ? "yes" : "no")
               << " glyphs=" << binding.mapView->getGlyphsRequestedCount() << '/'
               << binding.mapView->getGlyphsLoadedCount() << '/' << binding.mapView->getGlyphsErrorCount()
               << " sprites=" << binding.mapView->getSpritesRequestedCount() << '/'
               << binding.mapView->getSpritesLoadedCount() << '/' << binding.mapView->getSpritesErrorCount()
               << " tiles=" << binding.mapView->getTileActionCount() << " src=" << binding.mapView->getSourceChangedCount()
               << " missingImg=" << binding.mapView->getStyleImageMissingCount();
        if (hasMap) {
            const auto camera = binding.mapView->getCameraOptions();
            if (camera.center) {
                stream << " cam=" << camera.center->latitude() << ',' << camera.center->longitude();
            }
            if (camera.zoom) {
                stream << " z=" << *camera.zoom;
            }
            if (camera.pitch) {
                stream << " p=" << *camera.pitch;
            }
            if (camera.bearing) {
                stream << " b=" << *camera.bearing;
            }
        }
        if (!binding.mapView->getLastMapLoadError().empty()) {
            stream << " mapErr=" << binding.mapView->getLastMapLoadError();
        }
        if (!binding.mapView->getLastRenderError().empty()) {
            stream << " renderErr=" << binding.mapView->getLastRenderError();
        }
        if (!binding.mapView->getLastSpritesError().empty()) {
            stream << " spriteErr=" << binding.mapView->getLastSpritesError();
        }
        if (!binding.mapView->getLastGlyphsError().empty()) {
            stream << " glyphErr=" << binding.mapView->getLastGlyphsError();
        }
    }
    stream << " styleKind=" << styleKindLabel(binding.styleKind);
    if (!binding.style.empty()) {
        const auto preview = binding.style.size() > 72 ? binding.style.substr(0, 72) + "..." : binding.style;
        stream << " styleRef=" << preview;
    }
    if (!binding.lastSurfaceError.empty()) {
        stream << " surfaceErr=" << binding.lastSurfaceError;
    }
    return stream.str();
}

void logSurfaceDiagnostics(const SurfaceBinding& binding, const char* label) {
    const auto message = std::string(label) + ": " + formatSurfaceDiagnostics(binding);
    OH_LOG_PrintMsg(LOG_APP, LOG_INFO, kMapLibreHilogDomain, kMapLibreHilogTag, message.c_str());
}

void maybeLogPeriodicDiagnostics(SurfaceBinding& binding) {
    if (!binding.mapView || binding.diagnosticsLogIntervalFrames == 0) {
        return;
    }
    ++binding.diagnosticsLogFrameCounter;
    if (binding.diagnosticsLogFrameCounter % binding.diagnosticsLogIntervalFrames != 0) {
        return;
    }
    logSurfaceDiagnostics(binding, "periodic");
}

struct ExternalBinding {
    SurfaceBinding* binding = nullptr;
};

struct ResolvedBinding {
    SurfaceBinding* binding = nullptr;
};

struct ResolvedBindingCall {
    SurfaceBinding* binding = nullptr;
    std::size_t offset = 0;
};

enum class CameraCommand {
    Jump,
    Ease,
    Fly,
};

std::unordered_map<ArkUI_NodeHandle, SurfaceBinding*>& nodeBindings() {
    static std::unordered_map<ArkUI_NodeHandle, SurfaceBinding*> bindings;
    return bindings;
}

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

bool updateNodeSurfaceSizeFromLayout(SurfaceBinding& binding) {
    if (binding.node == nullptr) {
        return false;
    }

    ArkUI_IntSize layoutSize{};
    if (OH_ArkUI_NodeUtils_GetLayoutSize(binding.node, &layoutSize) != ARKUI_ERROR_CODE_NO_ERROR ||
        layoutSize.width <= 0 || layoutSize.height <= 0) {
        return false;
    }

    binding.width = static_cast<std::uint64_t>(layoutSize.width);
    binding.height = static_cast<std::uint64_t>(layoutSize.height);
    return true;
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
    if (!binding.mapView || !binding.mapView->hasMap() || binding.styleKind == StyleKind::None ||
        binding.appliedStyleGeneration == binding.styleGeneration) {
        return;
    }

    if (binding.styleKind == StyleKind::URL) {
        binding.mapView->setStyleURL(binding.style);
    } else {
        binding.mapView->setStyleJSON(binding.style);
    }
    binding.appliedStyleGeneration = binding.styleGeneration;
}

void renderBindingFrame(SurfaceBinding& binding) {
    if (binding.renderingEnabled && binding.surfaceVisible && binding.mapView) {
        binding.mapView->renderFrame();
        maybeLogPeriodicDiagnostics(binding);
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
        ++binding.surfaceErrorCount;
        binding.lastSurfaceError = exception.what();
        clearBindingSurface(binding);
        mbgl::Log::Error(mbgl::Event::OpenGL, exception.what());
    }
}

void onNodeFrame(ArkUI_NodeHandle node, std::uint64_t, std::uint64_t) {
    const auto it = nodeBindings().find(node);
    if (it == nodeBindings().end() || it->second == nullptr) {
        return;
    }

    auto* binding = it->second;
    ++binding->frameCallbackCount;
    renderBindingFrame(*binding);
}

void onNodeTouchEvent(ArkUI_NodeEvent* event) {
    if (event == nullptr || OH_ArkUI_NodeEvent_GetEventType(event) != NODE_ON_TOUCH_INTERCEPT) {
        return;
    }

    auto* binding = static_cast<SurfaceBinding*>(OH_ArkUI_NodeEvent_GetUserData(event));
    if (binding == nullptr) {
        const auto node = OH_ArkUI_NodeEvent_GetNodeHandle(event);
        const auto it = nodeBindings().find(node);
        if (it == nodeBindings().end()) {
            return;
        }
        binding = it->second;
    }
    if (binding == nullptr) {
        return;
    }

    ++binding->touchEventCount;
    auto* inputEvent = OH_ArkUI_NodeEvent_GetInputEvent(event);
    if (inputEvent != nullptr) {
        OH_ArkUI_PointerEvent_SetInterceptHitTestMode(inputEvent, HTM_BLOCK);
    }
    if (mbgl::ohos::handleInputEvent(binding->gesture, binding->mapView.get(), inputEvent)) {
        ++binding->gestureHandledCount;
        renderBindingFrame(*binding);
    }
}

SurfaceBinding* findLegacyBinding(OH_NativeXComponent* component) {
    const auto it = legacyBindings().find(component);
    if (it == legacyBindings().end()) {
        return nullptr;
    }
    return it->second.get();
}

std::string legacyComponentId(OH_NativeXComponent* component) {
    char id[OH_XCOMPONENT_ID_LEN_MAX + 1] = {};
    std::uint64_t idSize = OH_XCOMPONENT_ID_LEN_MAX + 1;
    if (OH_NativeXComponent_GetXComponentId(component, id, &idSize) != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        return {};
    }
    return id;
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

bool hasPendingException(napi_env env) {
    bool pending = false;
    napi_is_exception_pending(env, &pending);
    return pending;
}

bool isExternal(napi_env env, napi_value value) {
    if (value == nullptr) {
        return false;
    }

    napi_valuetype type = napi_undefined;
    return napi_typeof(env, value, &type) == napi_ok && type == napi_external;
}

ExternalBinding* getExternalHandle(napi_env env, napi_value value) {
    void* data = nullptr;
    if (napi_get_value_external(env, value, &data) != napi_ok || data == nullptr) {
        throwError(env, "Expected a native XComponent binding handle");
        return nullptr;
    }

    return static_cast<ExternalBinding*>(data);
}

SurfaceBinding* getBinding(napi_env env, napi_value value) {
    auto* handle = getExternalHandle(env, value);
    if (handle == nullptr) {
        return nullptr;
    }

    if (handle->binding == nullptr) {
        throwError(env, "Native XComponent binding has been destroyed");
        return nullptr;
    }

    return handle->binding;
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

SurfaceBinding* getExternalBindingIfPresent(napi_env env, napi_value value) {
    if (value == nullptr) {
        return nullptr;
    }

    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, value, &type) != napi_ok || type != napi_external) {
        return nullptr;
    }

    return getBinding(env, value);
}

bool resolveBindingCall(
    napi_env env, napi_value thisArg, std::size_t argc, napi_value* argv, ResolvedBindingCall& result) {
    result.binding = argc > 0 ? getExternalBindingIfPresent(env, argv[0]) : nullptr;
    if (hasPendingException(env)) {
        return false;
    }

    result.offset = result.binding != nullptr ? 1 : 0;
    if (result.binding == nullptr) {
        result.binding = getLegacyBinding(env, thisArg);
    }
    if (result.binding == nullptr) {
        throwError(env, "Expected a native XComponent binding handle or context");
        return false;
    }
    return true;
}

bool resolveBinding(napi_env env, napi_value thisArg, std::size_t argc, napi_value* argv, ResolvedBinding& result) {
    ResolvedBindingCall call;
    if (!resolveBindingCall(env, thisArg, argc, argv, call)) {
        return false;
    }
    result.binding = call.binding;
    return true;
}

bool isValidFrameRateRange(const OH_NativeXComponent_ExpectedRateRange& range) {
    return range.min > 0 && range.max >= range.min && range.expected >= range.min && range.expected <= range.max;
}

bool parseFrameRateRange(napi_env env, napi_value value, OH_NativeXComponent_ExpectedRateRange& range) {
    return isObject(env, value) && getRequiredInt32Property(env, value, "min", range.min) &&
           getRequiredInt32Property(env, value, "max", range.max) &&
           getRequiredInt32Property(env, value, "expected", range.expected) && isValidFrameRateRange(range);
}

void setInt32Property(napi_env env, napi_value object, const char* name, std::int32_t value) {
    napi_value property = nullptr;
    napi_create_int32(env, value, &property);
    napi_set_named_property(env, object, name, property);
}

napi_value createFrameRateRangeObject(napi_env env, const OH_NativeXComponent_ExpectedRateRange& range) {
    napi_value object = nullptr;
    napi_create_object(env, &object);
    setInt32Property(env, object, "min", range.min);
    setInt32Property(env, object, "max", range.max);
    setInt32Property(env, object, "expected", range.expected);
    return object;
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

void setStringProperty(napi_env env, napi_value object, const char* name, const std::string& value) {
    napi_set_named_property(env, object, name, createStringValue(env, value));
}

napi_value createSurfaceStateObject(napi_env env, const SurfaceBinding& binding) {
    const bool hasWindow = binding.window != nullptr;
    const bool hasSurface = hasWindow && binding.width > 0 && binding.height > 0;
    const bool hasMap = binding.mapView && binding.mapView->hasMap();
    const bool needsRender = binding.mapView && binding.mapView->hasPendingRender();
    const bool styleLoaded = binding.mapView && binding.mapView->hasLoadedStyle();
    const bool mapLoaded = binding.mapView && binding.mapView->hasLoadedMap();
    const bool fullyLoaded = binding.mapView && binding.mapView->isFullyLoaded();
    const bool idle = binding.mapView && binding.mapView->isIdle();
    const bool lastFrameNeededRepaint = binding.mapView && binding.mapView->lastFrameNeededRepaint();
    const bool lastFrameComplete = binding.mapView && binding.mapView->lastFrameWasComplete();
    const auto renderedFrameCount = binding.mapView ? binding.mapView->getRenderedFrameCount() : 0;
    const auto coreFrameCount = binding.mapView ? binding.mapView->getCoreFrameCount() : 0;
    const auto mapLoadErrorCount = binding.mapView ? binding.mapView->getMapLoadErrorCount() : 0;
    const auto renderErrorCount = binding.mapView ? binding.mapView->getRenderErrorCount() : 0;
    const auto sourceChangedCount = binding.mapView ? binding.mapView->getSourceChangedCount() : 0;
    const auto styleImageMissingCount = binding.mapView ? binding.mapView->getStyleImageMissingCount() : 0;
    const auto glyphsRequestedCount = binding.mapView ? binding.mapView->getGlyphsRequestedCount() : 0;
    const auto glyphsLoadedCount = binding.mapView ? binding.mapView->getGlyphsLoadedCount() : 0;
    const auto glyphsErrorCount = binding.mapView ? binding.mapView->getGlyphsErrorCount() : 0;
    const auto tileActionCount = binding.mapView ? binding.mapView->getTileActionCount() : 0;
    const auto spritesRequestedCount = binding.mapView ? binding.mapView->getSpritesRequestedCount() : 0;
    const auto spritesLoadedCount = binding.mapView ? binding.mapView->getSpritesLoadedCount() : 0;
    const auto spritesErrorCount = binding.mapView ? binding.mapView->getSpritesErrorCount() : 0;
    const auto glesContextClientVersion = binding.mapView ? binding.mapView->getGlesContextClientVersion() : 0;

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
    setBoolProperty(env, object, "idle", idle);
    setBoolProperty(env, object, "lastFrameNeededRepaint", lastFrameNeededRepaint);
    setBoolProperty(env, object, "lastFrameComplete", lastFrameComplete);
    setSizeProperty(env, object, "renderedFrameCount", renderedFrameCount);
    setSizeProperty(env, object, "coreFrameCount", coreFrameCount);
    setSizeProperty(env, object, "mapLoadErrorCount", mapLoadErrorCount);
    setSizeProperty(env, object, "renderErrorCount", renderErrorCount);
    setSizeProperty(env, object, "sourceChangedCount", sourceChangedCount);
    setSizeProperty(env, object, "styleImageMissingCount", styleImageMissingCount);
    setSizeProperty(env, object, "glyphsRequestedCount", glyphsRequestedCount);
    setSizeProperty(env, object, "glyphsLoadedCount", glyphsLoadedCount);
    setSizeProperty(env, object, "glyphsErrorCount", glyphsErrorCount);
    setSizeProperty(env, object, "tileActionCount", tileActionCount);
    setSizeProperty(env, object, "spritesRequestedCount", spritesRequestedCount);
    setSizeProperty(env, object, "spritesLoadedCount", spritesLoadedCount);
    setSizeProperty(env, object, "spritesErrorCount", spritesErrorCount);
    setInt32Property(env, object, "glesContextClientVersion", glesContextClientVersion);
    setSizeProperty(env, object, "frameCallbackCount", binding.frameCallbackCount);
    setSizeProperty(env, object, "touchEventCount", binding.touchEventCount);
    setSizeProperty(env, object, "gestureHandledCount", binding.gestureHandledCount);
    setBoolProperty(env, object, "surfaceVisible", binding.surfaceVisible);
    setSizeProperty(env, object, "surfaceCreatedCount", binding.surfaceCreatedCount);
    setSizeProperty(env, object, "surfaceChangedCount", binding.surfaceChangedCount);
    setSizeProperty(env, object, "surfaceDestroyedCount", binding.surfaceDestroyedCount);
    setSizeProperty(env, object, "surfaceShownCount", binding.surfaceShownCount);
    setSizeProperty(env, object, "surfaceHiddenCount", binding.surfaceHiddenCount);
    setSizeProperty(env, object, "surfaceErrorCount", binding.surfaceErrorCount);
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
    if (binding.node != nullptr) {
        return OH_ArkUI_XComponent_SetExpectedFrameRateRange(binding.node, range) == ARKUI_ERROR_CODE_NO_ERROR;
    }

    if (binding.legacyComponent != nullptr) {
        auto mutableRange = range;
        return OH_NativeXComponent_SetExpectedFrameRateRange(binding.legacyComponent, &mutableRange) ==
               OH_NATIVEXCOMPONENT_RESULT_SUCCESS;
    }

    return false;
}

void onLegacySurfaceCreated(OH_NativeXComponent* component, void* window) {
    if (auto* binding = findLegacyBinding(component)) {
        ++binding->surfaceCreatedCount;
    }
    updateLegacySurface(component, window);
}

void onLegacySurfaceChanged(OH_NativeXComponent* component, void* window) {
    if (auto* binding = findLegacyBinding(component)) {
        ++binding->surfaceChangedCount;
    }
    updateLegacySurface(component, window);
}

void onLegacySurfaceDestroyed(OH_NativeXComponent* component, void*) {
    auto* binding = findLegacyBinding(component);
    if (binding == nullptr) {
        return;
    }

    ++binding->surfaceDestroyedCount;
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

    ++binding->touchEventCount;
    OH_NativeXComponent_TouchEvent event{};
    if (OH_NativeXComponent_GetTouchEvent(component, window, &event) != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        mbgl::Log::Warning(mbgl::Event::General, "Could not query legacy XComponent touch event");
        return;
    }

    if (mbgl::ohos::handleTouchEvent(binding->gesture, binding->mapView.get(), event)) {
        ++binding->gestureHandledCount;
        renderBindingFrame(*binding);
    }
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

    const auto id = legacyComponentId(component);
    if (!id.empty()) {
        mbgl::Log::Info(mbgl::Event::Setup, "Registered legacy XComponent " + id);
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

void onSurfaceCreated(OH_ArkUI_SurfaceHolder* holder) {
    auto* binding = static_cast<SurfaceBinding*>(OH_ArkUI_SurfaceHolder_GetUserData(holder));
    if (binding == nullptr) {
        return;
    }

    ++binding->surfaceCreatedCount;
    binding->surfaceVisible = true;
    binding->window = OH_ArkUI_XComponent_GetNativeWindow(holder);
    if ((binding->width == 0 || binding->height == 0) && !updateNodeSurfaceSizeFromLayout(*binding)) {
        mbgl::Log::Warning(mbgl::Event::OpenGL, "Could not query ArkUI XComponent layout size");
    }
    updateSurface(*binding);
}

void onSurfaceChanged(OH_ArkUI_SurfaceHolder* holder, std::uint64_t width, std::uint64_t height) {
    auto* binding = static_cast<SurfaceBinding*>(OH_ArkUI_SurfaceHolder_GetUserData(holder));
    if (binding == nullptr) {
        return;
    }

    ++binding->surfaceChangedCount;
    binding->surfaceVisible = true;
    binding->width = width;
    binding->height = height;
    binding->window = OH_ArkUI_XComponent_GetNativeWindow(holder);
    if ((binding->width == 0 || binding->height == 0) && !updateNodeSurfaceSizeFromLayout(*binding)) {
        mbgl::Log::Warning(mbgl::Event::OpenGL, "Could not query ArkUI XComponent layout size");
    }
    updateSurface(*binding);
}

void onSurfaceDestroyed(OH_ArkUI_SurfaceHolder* holder) {
    auto* binding = static_cast<SurfaceBinding*>(OH_ArkUI_SurfaceHolder_GetUserData(holder));
    if (binding == nullptr) {
        return;
    }

    ++binding->surfaceDestroyedCount;
    binding->surfaceVisible = false;
    clearBindingSurface(*binding);
    binding->window = nullptr;
    binding->width = 0;
    binding->height = 0;
}

void onSurfaceShow(OH_ArkUI_SurfaceHolder* holder) {
    auto* binding = static_cast<SurfaceBinding*>(OH_ArkUI_SurfaceHolder_GetUserData(holder));
    if (binding == nullptr) {
        return;
    }

    ++binding->surfaceShownCount;
    binding->surfaceVisible = true;
    binding->window = OH_ArkUI_XComponent_GetNativeWindow(holder);
    if ((binding->width == 0 || binding->height == 0) && !updateNodeSurfaceSizeFromLayout(*binding)) {
        mbgl::Log::Warning(mbgl::Event::OpenGL, "Could not query ArkUI XComponent layout size on show");
    }
    updateSurface(*binding);
}

void onSurfaceHide(OH_ArkUI_SurfaceHolder* holder) {
    auto* binding = static_cast<SurfaceBinding*>(OH_ArkUI_SurfaceHolder_GetUserData(holder));
    if (binding == nullptr) {
        return;
    }

    ++binding->surfaceHiddenCount;
    binding->surfaceVisible = false;
    if (binding->mapView && mbgl::ohos::hasActiveGesture(binding->gesture)) {
        binding->mapView->setGestureInProgress(false);
    }
    mbgl::ohos::resetGestureState(binding->gesture);
}

void disposeBinding(SurfaceBinding* binding) {
    if (binding == nullptr) {
        return;
    }

    if (binding->node != nullptr) {
        if (binding->nodeTouchRegistered) {
            OH_ArkUI_NativeModule_UnregisterCommonEvent(binding->node, NODE_ON_TOUCH_INTERCEPT);
            binding->nodeTouchRegistered = false;
        }
        if (binding->nodeFrameRegistered) {
            OH_ArkUI_XComponent_UnregisterOnFrameCallback(binding->node);
            binding->nodeFrameRegistered = false;
        }
        const auto it = nodeBindings().find(binding->node);
        if (it != nodeBindings().end() && it->second == binding) {
            nodeBindings().erase(it);
        }
        binding->node = nullptr;
    }
    if (binding->holder != nullptr) {
        OH_ArkUI_SurfaceHolder_SetUserData(binding->holder, nullptr);
        if (binding->callback != nullptr) {
            OH_ArkUI_SurfaceHolder_RemoveSurfaceCallback(binding->holder, binding->callback);
        }
    }

    clearBindingMapState(*binding);

    if (binding->callback != nullptr) {
        OH_ArkUI_SurfaceCallback_Dispose(binding->callback);
    }
    if (binding->holder != nullptr) {
        OH_ArkUI_SurfaceHolder_Dispose(binding->holder);
    }

    delete binding;
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

void finalizeBinding(napi_env, void* data, void*) {
    auto* handle = static_cast<ExternalBinding*>(data);
    if (handle == nullptr) {
        return;
    }

    disposeBinding(handle->binding);
    delete handle;
}

napi_value registerXComponentNode(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    if (napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected an ArkUI XComponent node");
    }

    ArkUI_NodeHandle node = nullptr;
    if (OH_ArkUI_GetNodeHandleFromNapiValue(env, argv[0], &node) != ARKUI_ERROR_CODE_NO_ERROR || node == nullptr) {
        return throwError(env, "Could not obtain an ArkUI node handle from the provided value");
    }

    if (nodeBindings().find(node) != nodeBindings().end()) {
        return throwError(env, "ArkUI XComponent node is already registered");
    }

    auto* binding = new (std::nothrow) SurfaceBinding();
    if (binding == nullptr) {
        return throwError(env, "Could not allocate XComponent surface binding");
    }

    binding->node = node;
    binding->holder = OH_ArkUI_SurfaceHolder_Create(node);
    binding->callback = OH_ArkUI_SurfaceCallback_Create();
    if (binding->holder == nullptr || binding->callback == nullptr) {
        disposeBinding(binding);
        return throwError(env, "Could not create XComponent surface holder callback");
    }

    OH_ArkUI_SurfaceCallback_SetSurfaceCreatedEvent(binding->callback, onSurfaceCreated);
    OH_ArkUI_SurfaceCallback_SetSurfaceChangedEvent(binding->callback, onSurfaceChanged);
    OH_ArkUI_SurfaceCallback_SetSurfaceDestroyedEvent(binding->callback, onSurfaceDestroyed);
    OH_ArkUI_SurfaceCallback_SetSurfaceShowEvent(binding->callback, onSurfaceShow);
    OH_ArkUI_SurfaceCallback_SetSurfaceHideEvent(binding->callback, onSurfaceHide);

    if (OH_ArkUI_SurfaceHolder_SetUserData(binding->holder, binding) != ARKUI_ERROR_CODE_NO_ERROR) {
        disposeBinding(binding);
        return throwError(env, "Could not attach XComponent surface binding data");
    }

    if (OH_ArkUI_SurfaceHolder_AddSurfaceCallback(binding->holder, binding->callback) != ARKUI_ERROR_CODE_NO_ERROR) {
        disposeBinding(binding);
        return throwError(env, "Could not register XComponent surface callback");
    }

    nodeBindings()[node] = binding;
    if (OH_ArkUI_XComponent_RegisterOnFrameCallback(node, onNodeFrame) != ARKUI_ERROR_CODE_NO_ERROR) {
        mbgl::Log::Warning(mbgl::Event::Setup, "Could not register ArkUI XComponent frame callback");
    } else {
        binding->nodeFrameRegistered = true;
    }

    if (binding->frameRateRange && !applyFrameRateRange(*binding, *binding->frameRateRange)) {
        mbgl::Log::Warning(mbgl::Event::Setup, "Could not apply ArkUI XComponent frame-rate range");
    }

    if (OH_ArkUI_NativeModule_RegisterCommonEvent(node, NODE_ON_TOUCH_INTERCEPT, binding, onNodeTouchEvent) !=
        ARKUI_ERROR_CODE_NO_ERROR) {
        mbgl::Log::Warning(mbgl::Event::Setup, "Could not register ArkUI XComponent touch callback");
    } else {
        binding->nodeTouchRegistered = true;
    }

    auto* handle = new (std::nothrow) ExternalBinding{binding};
    if (handle == nullptr) {
        disposeBinding(binding);
        return throwError(env, "Could not allocate native XComponent binding handle");
    }

    napi_value external = nullptr;
    if (napi_create_external(env, handle, finalizeBinding, nullptr, &external) != napi_ok) {
        disposeBinding(binding);
        delete handle;
        return throwError(env, "Could not create native XComponent binding handle");
    }

    return external;
}

napi_value destroy(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent binding handle or context");
    }

    if (argc > 0 && isExternal(env, argv[0])) {
        auto* handle = getExternalHandle(env, argv[0]);
        if (handle == nullptr) {
            return getUndefined(env);
        }

        disposeBinding(handle->binding);
        handle->binding = nullptr;
        return getUndefined(env);
    }

    if (auto* component = getLegacyComponent(env, thisArg)) {
        if (auto* binding = findLegacyBinding(component)) {
            disposeLegacyBinding(binding);
        }
        return getUndefined(env);
    }

    return throwError(env, "Expected a native XComponent binding handle or context");
}

napi_value renderFrame(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent binding handle");
    }

    ResolvedBinding resolved;
    if (!resolveBinding(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }

    renderBindingFrame(*resolved.binding);

    return getUndefined(env);
}

napi_value reduceMemoryUse(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent binding handle");
    }

    ResolvedBinding resolved;
    if (!resolveBinding(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }

    if (resolved.binding->mapView) {
        resolved.binding->mapView->reduceMemoryUse();
    }

    return getUndefined(env);
}

napi_value runLoopOnce(napi_env env, napi_callback_info) {
    mbgl::util::RunLoop::Get()->runOnce();
    return getUndefined(env);
}

napi_value setStyleUrl(napi_env env, napi_callback_info info) {
    std::size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected a native XComponent binding handle and style URL");
    }

    ResolvedBindingCall resolved;
    if (!resolveBindingCall(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }
    if (resolved.offset == 1 && (argc < 2 || argv[1] == nullptr)) {
        return throwError(env, "Expected a style URL");
    }
    auto* binding = resolved.binding;
    napi_value styleArg = argv[resolved.offset];

    std::string style;
    if (!getString(env, styleArg, style)) {
        return throwError(env, "Expected style URL string");
    }

    binding->styleKind = StyleKind::URL;
    binding->style = std::move(style);
    ++binding->styleGeneration;
    applyDesiredStyle(*binding);
    logSurfaceDiagnostics(*binding, "setStyleUrl");

    return getUndefined(env);
}

napi_value setStyleJson(napi_env env, napi_callback_info info) {
    std::size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected a native XComponent binding handle and style JSON");
    }

    ResolvedBindingCall resolved;
    if (!resolveBindingCall(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }
    if (resolved.offset == 1 && (argc < 2 || argv[1] == nullptr)) {
        return throwError(env, "Expected style JSON");
    }
    auto* binding = resolved.binding;
    napi_value styleArg = argv[resolved.offset];

    std::string style;
    if (!getString(env, styleArg, style)) {
        return throwError(env, "Expected style JSON string");
    }

    binding->styleKind = StyleKind::JSON;
    binding->style = std::move(style);
    ++binding->styleGeneration;
    applyDesiredStyle(*binding);
    logSurfaceDiagnostics(*binding, "setStyleJson");

    return getUndefined(env);
}

napi_value logSurfaceState(napi_env env, napi_callback_info info) {
    std::size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected a native XComponent binding handle");
    }

    ResolvedBindingCall resolved;
    if (!resolveBindingCall(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }

    std::string label = "manual";
    if (resolved.offset == 1 && argc >= 2 && argv[1] != nullptr) {
        getString(env, argv[1], label);
    }
    logSurfaceDiagnostics(*resolved.binding, label.c_str());
    return getUndefined(env);
}

napi_value fitBounds(napi_env env, napi_callback_info info) {
    std::size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected camera bounds options");
    }

    ResolvedBindingCall resolved;
    if (!resolveBindingCall(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }
    auto* binding = resolved.binding;
    const auto offset = resolved.offset;
    if (offset == 1 && argc < 2) {
        return throwError(env, "Expected a native XComponent binding handle and camera bounds options");
    }

    mbgl::ohos::CameraBoundsOptions options;
    if (argc <= offset || !getCameraBoundsOptions(env, argv[offset], options)) {
        return throwError(env, "Expected valid camera bounds options object");
    }

    try {
        ensureMapView(*binding).fitBounds(std::move(options));
        renderBindingFrame(*binding);
    } catch (const std::exception& exception) {
        return throwError(env, exception.what());
    }

    return getUndefined(env);
}

napi_value cameraForBounds(napi_env env, napi_callback_info info) {
    std::size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected camera bounds options");
    }

    ResolvedBindingCall resolved;
    if (!resolveBindingCall(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }
    auto* binding = resolved.binding;
    const auto offset = resolved.offset;
    if (offset == 1 && argc < 2) {
        return throwError(env, "Expected a native XComponent binding handle and camera bounds options");
    }

    mbgl::ohos::CameraBoundsOptions options;
    if (argc <= offset || !getCameraBoundsOptions(env, argv[offset], options)) {
        return throwError(env, "Expected valid camera bounds options object");
    }
    if (!binding->mapView || !binding->mapView->hasMap()) {
        return throwError(env, "Camera for bounds requires an active map surface");
    }

    try {
        return createCameraOptionsObject(env, binding->mapView->cameraForBounds(options));
    } catch (const std::exception& exception) {
        return throwError(env, exception.what());
    }
}

napi_value applyCameraCommand(napi_env env,
                              napi_callback_info info,
                              CameraCommand command,
                              bool allowPositionalCamera) {
    std::size_t argc = 7;
    napi_value argv[7] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected camera options");
    }

    ResolvedBindingCall resolved;
    if (!resolveBindingCall(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }
    auto* binding = resolved.binding;
    const auto offset = resolved.offset;

    if (argc <= offset || argv[offset] == nullptr) {
        return throwError(env, "Expected camera options");
    }

    mbgl::CameraOptions cameraOptions;
    mbgl::AnimationOptions animationOptions;
    if (isObject(env, argv[offset])) {
        if (!getCameraOptionsObject(env, argv[offset], cameraOptions)) {
            return throwError(env, "Expected valid camera options object");
        }
        if (command != CameraCommand::Jump && argc > offset + 1 &&
            !getAnimationOptions(env, argv[offset + 1], animationOptions)) {
            return throwError(env, "Expected animation options object");
        }
    } else {
        if (!allowPositionalCamera || argc < offset + 2) {
            return throwError(env, "Expected camera options object");
        }

        double longitude = 0.0;
        double latitude = 0.0;
        if (!getDouble(env, argv[offset], longitude) || !std::isfinite(longitude) ||
            !getDouble(env, argv[offset + 1], latitude) || !std::isfinite(latitude)) {
            return throwError(env, "Expected numeric longitude and latitude");
        }

        std::optional<double> zoom;
        if (argc > offset + 2 && !isNullOrUndefined(env, argv[offset + 2])) {
            double zoomValue = 0.0;
            if (!getDouble(env, argv[offset + 2], zoomValue) || !std::isfinite(zoomValue)) {
                return throwError(env, "Expected numeric zoom");
            }
            zoom = zoomValue;
        }

        std::optional<double> bearing;
        if (argc > offset + 3 && !isNullOrUndefined(env, argv[offset + 3])) {
            double bearingValue = 0.0;
            if (!getDouble(env, argv[offset + 3], bearingValue) || !std::isfinite(bearingValue)) {
                return throwError(env, "Expected numeric bearing");
            }
            bearing = bearingValue;
        }

        std::optional<double> pitch;
        if (argc > offset + 4 && !isNullOrUndefined(env, argv[offset + 4])) {
            double pitchValue = 0.0;
            if (!getDouble(env, argv[offset + 4], pitchValue) || !std::isfinite(pitchValue)) {
                return throwError(env, "Expected numeric pitch");
            }
            pitch = pitchValue;
        }

        try {
            cameraOptions.withCenter(mbgl::LatLng(latitude, longitude))
                .withZoom(zoom)
                .withBearing(bearing)
                .withPitch(pitch);
        } catch (const std::exception& exception) {
            return throwError(env, exception.what());
        }
    }

    try {
        auto& mapView = ensureMapView(*binding);
        switch (command) {
            case CameraCommand::Jump:
                mapView.jumpTo(std::move(cameraOptions));
                break;
            case CameraCommand::Ease:
                mapView.easeTo(std::move(cameraOptions), std::move(animationOptions));
                break;
            case CameraCommand::Fly:
                mapView.flyTo(std::move(cameraOptions), std::move(animationOptions));
                break;
        }
        renderBindingFrame(*binding);
    } catch (const std::exception& exception) {
        return throwError(env, exception.what());
    }

    return getUndefined(env);
}

napi_value jumpTo(napi_env env, napi_callback_info info) {
    return applyCameraCommand(env, info, CameraCommand::Jump, true);
}

napi_value setCameraOptions(napi_env env, napi_callback_info info) {
    return applyCameraCommand(env, info, CameraCommand::Jump, false);
}

napi_value setFreeCameraOptions(napi_env env, napi_callback_info info) {
    std::size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected free camera options");
    }

    ResolvedBindingCall resolved;
    if (!resolveBindingCall(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }
    auto* binding = resolved.binding;
    const auto offset = resolved.offset;
    if (offset == 1 && argc < 2) {
        return throwError(env, "Expected a native XComponent binding handle and free camera options");
    }

    mbgl::FreeCameraOptions cameraOptions;
    if (argc <= offset || !getFreeCameraOptionsObject(env, argv[offset], cameraOptions)) {
        return throwError(env, "Expected valid free camera options object");
    }

    try {
        ensureMapView(*binding).setFreeCameraOptions(std::move(cameraOptions));
        renderBindingFrame(*binding);
    } catch (const std::exception& exception) {
        return throwError(env, exception.what());
    }

    return getUndefined(env);
}

napi_value easeTo(napi_env env, napi_callback_info info) {
    return applyCameraCommand(env, info, CameraCommand::Ease, false);
}

napi_value flyTo(napi_env env, napi_callback_info info) {
    return applyCameraCommand(env, info, CameraCommand::Fly, false);
}

napi_value setPixelRatio(napi_env env, napi_callback_info info) {
    std::size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected a pixel ratio");
    }

    ResolvedBindingCall resolved;
    if (!resolveBindingCall(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }
    auto* binding = resolved.binding;
    const auto offset = resolved.offset;
    if (offset == 1 && argc < 2) {
        return throwError(env, "Expected a native XComponent binding handle and pixel ratio");
    }

    double pixelRatio = 0.0;
    if (argc <= offset || !getDouble(env, argv[offset], pixelRatio) || !std::isfinite(pixelRatio) ||
        pixelRatio <= 0.0) {
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

napi_value setDebugOptions(napi_env env, napi_callback_info info) {
    std::size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected debug options");
    }

    ResolvedBindingCall resolved;
    if (!resolveBindingCall(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }
    auto* binding = resolved.binding;
    const auto offset = resolved.offset;
    if (offset == 1 && argc < 2) {
        return throwError(env, "Expected a native XComponent binding handle and debug options");
    }

    std::uint32_t debugOptions = 0;
    if (argc <= offset || !getUint32(env, argv[offset], debugOptions) || !isValidDebugOptions(debugOptions)) {
        return throwError(env, "Expected valid debug options");
    }

    try {
        ensureMapView(*binding).setDebugOptions(static_cast<mbgl::MapDebugOptions>(debugOptions));
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

    ResolvedBindingCall resolved;
    if (!resolveBindingCall(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }
    auto* binding = resolved.binding;
    const auto offset = resolved.offset;
    if (offset == 1 && argc < 2) {
        return throwError(env, "Expected a native XComponent binding handle and bounds options");
    }

    mbgl::BoundOptions boundOptions;
    if (argc <= offset || !getBoundOptions(env, argv[offset], boundOptions)) {
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

    ResolvedBindingCall resolved;
    if (!resolveBindingCall(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }
    auto* binding = resolved.binding;
    const auto offset = resolved.offset;
    if (offset == 1 && argc < 2) {
        return throwError(env, "Expected a native XComponent binding handle and rendering enabled boolean");
    }

    bool enabled = false;
    if (argc <= offset || !getBool(env, argv[offset], enabled)) {
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

    ResolvedBindingCall resolved;
    if (!resolveBindingCall(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }
    auto* binding = resolved.binding;
    const auto offset = resolved.offset;
    if (offset == 1 && argc < 2) {
        return throwError(env, "Expected a native XComponent binding handle and frame-rate range");
    }

    OH_NativeXComponent_ExpectedRateRange range{};
    if (argc <= offset || !parseFrameRateRange(env, argv[offset], range)) {
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

    ResolvedBindingCall resolved;
    if (!resolveBindingCall(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }
    auto* binding = resolved.binding;
    const auto offset = resolved.offset;
    if (offset == 1 && argc < 2) {
        return throwError(env, "Expected a native XComponent binding handle and tile cache enabled boolean");
    }

    bool enabled = false;
    if (argc <= offset || !getBool(env, argv[offset], enabled)) {
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

    ResolvedBindingCall resolved;
    if (!resolveBindingCall(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }
    auto* binding = resolved.binding;
    const auto offset = resolved.offset;
    if (offset == 1 && argc < 2) {
        return throwError(env, "Expected a native XComponent binding handle, client name, and optional client version");
    }

    std::string clientName;
    if (argc <= offset || !getString(env, argv[offset], clientName)) {
        return throwError(env, "Expected client name string");
    }

    std::string clientVersion;
    if (argc > offset + 1 && !isNullOrUndefined(env, argv[offset + 1])) {
        if (!getString(env, argv[offset + 1], clientVersion)) {
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

    ResolvedBindingCall resolved;
    if (!resolveBindingCall(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }
    auto* binding = resolved.binding;
    const auto offset = resolved.offset;
    if (offset == 1 && argc < 2) {
        return throwError(env, "Expected a native XComponent binding handle and resource options");
    }
    if (argc <= offset || !isObject(env, argv[offset])) {
        return throwError(env, "Expected resource options object");
    }

    std::optional<std::string> apiKey;
    std::optional<std::string> cachePath;
    std::optional<std::string> assetPath;
    if (!getOptionalStringProperty(env, argv[offset], "apiKey", apiKey) ||
        !getOptionalStringProperty(env, argv[offset], "cachePath", cachePath) ||
        !getOptionalStringProperty(env, argv[offset], "assetPath", assetPath)) {
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

napi_value getDebugOptions(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent binding handle");
    }

    ResolvedBinding resolved;
    if (!resolveBinding(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }

    std::uint32_t debugOptions = 0;
    if (resolved.binding->mapView) {
        debugOptions = static_cast<std::uint32_t>(resolved.binding->mapView->getDebugOptions());
    }

    napi_value result = nullptr;
    napi_create_uint32(env, debugOptions, &result);
    return result;
}

napi_value getTileCacheEnabled(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent binding handle");
    }

    ResolvedBinding resolved;
    if (!resolveBinding(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }

    const bool enabled = resolved.binding->mapView ? resolved.binding->mapView->getTileCacheEnabled() : true;
    napi_value result = nullptr;
    napi_get_boolean(env, enabled, &result);
    return result;
}

napi_value getRenderingEnabled(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent binding handle");
    }

    ResolvedBinding resolved;
    if (!resolveBinding(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }

    napi_value result = nullptr;
    napi_get_boolean(env, resolved.binding->renderingEnabled, &result);
    return result;
}

napi_value getPixelRatio(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent binding handle");
    }

    ResolvedBinding resolved;
    if (!resolveBinding(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }

    const auto pixelRatio = resolved.binding->mapView ? resolved.binding->mapView->getPixelRatio()
                                                      : resolved.binding->pixelRatio;
    napi_value result = nullptr;
    napi_create_double(env, pixelRatio, &result);
    return result;
}

napi_value getFrameRateRange(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent binding handle");
    }

    ResolvedBinding resolved;
    if (!resolveBinding(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }

    if (!resolved.binding->frameRateRange) {
        return getUndefined(env);
    }
    return createFrameRateRangeObject(env, *resolved.binding->frameRateRange);
}

napi_value getClientOptions(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent binding handle");
    }

    ResolvedBinding resolved;
    if (!resolveBinding(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }

    const auto& mapView = ensureMapView(*resolved.binding);
    return createClientOptionsObject(env, mapView.getClientName(), mapView.getClientVersion());
}

napi_value getResourceOptions(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent binding handle");
    }

    ResolvedBinding resolved;
    if (!resolveBinding(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }

    return createResourceOptionsObject(env, ensureMapView(*resolved.binding).getResourceOptions());
}

napi_value getStyleUrl(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent binding handle");
    }

    ResolvedBinding resolved;
    if (!resolveBinding(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }

    if (resolved.binding->styleKind != StyleKind::URL) {
        return getUndefined(env);
    }
    return createStringValue(env, resolved.binding->style);
}

napi_value getStyleJson(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent binding handle");
    }

    ResolvedBinding resolved;
    if (!resolveBinding(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }

    if (resolved.binding->styleKind != StyleKind::JSON) {
        return getUndefined(env);
    }
    return createStringValue(env, resolved.binding->style);
}

napi_value getSurfaceState(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent binding handle");
    }

    ResolvedBinding resolved;
    if (!resolveBinding(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }

    return createSurfaceStateObject(env, *resolved.binding);
}

napi_value getBounds(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent binding handle");
    }

    ResolvedBinding resolved;
    if (!resolveBinding(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }

    return createBoundOptionsObject(env, ensureMapView(*resolved.binding).getBounds());
}

napi_value getCameraOptions(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent binding handle");
    }

    ResolvedBinding resolved;
    if (!resolveBinding(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }

    return createCameraOptionsObject(env, ensureMapView(*resolved.binding).getCameraOptions());
}

napi_value getFreeCameraOptions(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a native XComponent binding handle");
    }

    ResolvedBinding resolved;
    if (!resolveBinding(env, thisArg, argc, argv, resolved)) {
        return getUndefined(env);
    }

    return createFreeCameraOptionsObject(env, ensureMapView(*resolved.binding).getFreeCameraOptions());
}

napi_value Init(napi_env env, napi_value exports) {
    registerLegacyExportedXComponent(env, exports);

    napi_value debugOptions = createDebugOptionsObject(env);
    napi_property_descriptor properties[] = {
        {"DebugOptions", nullptr, nullptr, nullptr, nullptr, debugOptions, napi_default, nullptr},
        {"cameraForBounds", nullptr, cameraForBounds, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"destroy", nullptr, destroy, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"easeTo", nullptr, easeTo, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"fitBounds", nullptr, fitBounds, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"flyTo", nullptr, flyTo, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getBounds", nullptr, getBounds, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getCameraOptions", nullptr, getCameraOptions, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getClientOptions", nullptr, getClientOptions, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getDebugOptions", nullptr, getDebugOptions, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getFreeCameraOptions", nullptr, getFreeCameraOptions, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getFrameRateRange", nullptr, getFrameRateRange, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getPixelRatio", nullptr, getPixelRatio, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getRenderingEnabled", nullptr, getRenderingEnabled, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getResourceOptions", nullptr, getResourceOptions, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getStyleJson", nullptr, getStyleJson, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getStyleUrl", nullptr, getStyleUrl, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getSurfaceState", nullptr, getSurfaceState, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"logSurfaceState", nullptr, logSurfaceState, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getTileCacheEnabled", nullptr, getTileCacheEnabled, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"jumpTo", nullptr, jumpTo, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"registerXComponentNode", nullptr, registerXComponentNode, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"reduceMemoryUse", nullptr, reduceMemoryUse, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"renderFrame", nullptr, renderFrame, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"runLoopOnce", nullptr, runLoopOnce, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setCameraOptions", nullptr, setCameraOptions, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setBounds", nullptr, setBounds, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setClientOptions", nullptr, setClientOptions, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setDebugOptions", nullptr, setDebugOptions, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setFrameRateRange", nullptr, setFrameRateRange, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setFreeCameraOptions", nullptr, setFreeCameraOptions, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setPixelRatio", nullptr, setPixelRatio, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setRenderingEnabled", nullptr, setRenderingEnabled, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setResourceOptions", nullptr, setResourceOptions, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setTileCacheEnabled", nullptr, setTileCacheEnabled, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setStyleUrl", nullptr, setStyleUrl, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setStyleJson", nullptr, setStyleJson, nullptr, nullptr, nullptr, napi_default, nullptr},
    };

    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    return exports;
}

} // namespace

NAPI_MODULE(maplibre_native_ohos, Init)
