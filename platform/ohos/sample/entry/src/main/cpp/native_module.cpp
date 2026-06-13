#include "map_view.hpp"

#include "gesture_handler.hpp"
#include "native_values.hpp"

#include <mbgl/storage/resource_options.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/size.hpp>

#include <algorithm>
#include <cmath>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <ace/xcomponent/native_interface_xcomponent.h>
#include <arkui/native_interface.h>
#include <arkui/native_node.h>
#include <arkui/native_node_napi.h>
#include <arkui/ui_input_event.h>
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

class SurfaceController;

std::unordered_map<ArkUI_NodeHandle, std::weak_ptr<SurfaceController>>& controllersByNode() {
    static std::unordered_map<ArkUI_NodeHandle, std::weak_ptr<SurfaceController>> controllers;
    return controllers;
}

std::unordered_map<OH_ArkUI_SurfaceHolder*, std::weak_ptr<SurfaceController>>& controllersByHolder() {
    static std::unordered_map<OH_ArkUI_SurfaceHolder*, std::weak_ptr<SurfaceController>> controllers;
    return controllers;
}

std::mutex& controllerRegistryMutex() {
    static std::mutex mutex;
    return mutex;
}

void registerController(ArkUI_NodeHandle node,
                        OH_ArkUI_SurfaceHolder* holder,
                        const std::shared_ptr<SurfaceController>& controller) {
    std::lock_guard<std::mutex> lock(controllerRegistryMutex());
    controllersByNode()[node] = controller;
    controllersByHolder()[holder] = controller;
}

void unregisterController(ArkUI_NodeHandle node, OH_ArkUI_SurfaceHolder* holder, const SurfaceController* controller) {
    std::lock_guard<std::mutex> lock(controllerRegistryMutex());

    if (auto it = controllersByNode().find(node); it != controllersByNode().end()) {
        const auto existing = it->second.lock();
        if (!existing || existing.get() == controller) {
            controllersByNode().erase(it);
        }
    }

    if (auto it = controllersByHolder().find(holder); it != controllersByHolder().end()) {
        const auto existing = it->second.lock();
        if (!existing || existing.get() == controller) {
            controllersByHolder().erase(it);
        }
    }
}

std::shared_ptr<SurfaceController> findController(ArkUI_NodeHandle node) {
    std::lock_guard<std::mutex> lock(controllerRegistryMutex());
    const auto it = controllersByNode().find(node);
    if (it == controllersByNode().end()) {
        return nullptr;
    }
    auto controller = it->second.lock();
    if (!controller) {
        controllersByNode().erase(it);
    }
    return controller;
}

std::shared_ptr<SurfaceController> findController(OH_ArkUI_SurfaceHolder* holder) {
    std::lock_guard<std::mutex> lock(controllerRegistryMutex());
    const auto it = controllersByHolder().find(holder);
    if (it == controllersByHolder().end()) {
        return nullptr;
    }
    auto controller = it->second.lock();
    if (!controller) {
        controllersByHolder().erase(it);
    }
    return controller;
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

ArkUI_NativeNodeAPI_1* nativeNodeApi() {
    static auto* api = reinterpret_cast<ArkUI_NativeNodeAPI_1*>(
        OH_ArkUI_QueryModuleInterfaceByName(ARKUI_NATIVE_NODE, "ArkUI_NativeNodeAPI_1"));
    return api;
}

ArkUI_NativeNodeAPI_1& requireNativeNodeApi() {
    auto* api = nativeNodeApi();
    if (api == nullptr) {
        throw std::runtime_error("Could not load ArkUI_NativeNodeAPI_1");
    }
    return *api;
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
    return lhs.apiKey() == rhs.apiKey() && lhs.cachePath() == rhs.cachePath() && lhs.assetPath() == rhs.assetPath();
}

bool isValidFrameRateRange(const OH_NativeXComponent_ExpectedRateRange& range) {
    return range.min > 0 && range.max >= range.min && range.expected >= range.min && range.expected <= range.max;
}

bool parseFrameRateRange(napi_env env, napi_value value, OH_NativeXComponent_ExpectedRateRange& range) {
    return isObject(env, value) && getRequiredInt32Property(env, value, "min", range.min) &&
           getRequiredInt32Property(env, value, "max", range.max) &&
           getRequiredInt32Property(env, value, "expected", range.expected) && isValidFrameRateRange(range);
}

void setFloatAttribute(ArkUI_NodeHandle node,
                       ArkUI_NodeAttributeType attribute,
                       float value,
                       const char* errorMessage) {
    ArkUI_NumberValue numberValue{};
    numberValue.f32 = value;

    ArkUI_AttributeItem item{};
    item.value = &numberValue;
    item.size = 1;

    if (requireNativeNodeApi().setAttribute(node, attribute, &item) != ARKUI_ERROR_CODE_NO_ERROR) {
        throw std::runtime_error(errorMessage);
    }
}

ArkUI_NodeHandle createNativeXComponentNode() {
    auto& api = requireNativeNodeApi();
    ArkUI_NodeHandle node = api.createNode(ARKUI_NODE_XCOMPONENT);
    if (node == nullptr) {
        throw std::runtime_error("Could not create native XComponent node");
    }

    try {
        setFloatAttribute(node, NODE_WIDTH_PERCENT, 1.0f, "Could not set native XComponent width");
        setFloatAttribute(node, NODE_HEIGHT_PERCENT, 1.0f, "Could not set native XComponent height");
    } catch (...) {
        api.disposeNode(node);
        throw;
    }

    return node;
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

mbgl::ohos::TouchAction toTouchAction(int32_t action) {
    switch (action) {
        case UI_TOUCH_EVENT_ACTION_DOWN:
            return mbgl::ohos::TouchAction::Down;
        case UI_TOUCH_EVENT_ACTION_MOVE:
            return mbgl::ohos::TouchAction::Move;
        case UI_TOUCH_EVENT_ACTION_UP:
            return mbgl::ohos::TouchAction::Up;
        case UI_TOUCH_EVENT_ACTION_CANCEL:
            return mbgl::ohos::TouchAction::Cancel;
        default:
            return mbgl::ohos::TouchAction::Unknown;
    }
}

class SurfaceController final : public std::enable_shared_from_this<SurfaceController> {
public:
    static std::shared_ptr<SurfaceController> create(ArkUI_NodeHandle node_) {
        auto controller = std::shared_ptr<SurfaceController>(new SurfaceController(node_, nullptr, false));
        try {
            controller->initialize();
        } catch (...) {
            controller->close();
            throw;
        }
        return controller;
    }

    static std::shared_ptr<SurfaceController> create(ArkUI_NodeContentHandle content_) {
        if (content_ == nullptr) {
            throw std::invalid_argument("Expected a NodeContent host");
        }

        auto controller = std::shared_ptr<SurfaceController>(
            new SurfaceController(createNativeXComponentNode(), content_, true));
        try {
            controller->initialize();
            controller->addNodeToContent();
        } catch (...) {
            controller->close();
            throw;
        }
        return controller;
    }

    ~SurfaceController() { close(); }

    SurfaceController(const SurfaceController&) = delete;
    SurfaceController& operator=(const SurfaceController&) = delete;

    bool isClosed() const { return closed; }

    void close() {
        if (closed) {
            return;
        }

        closed = true;

        if (frameCallbackRegistered && node != nullptr &&
            OH_ArkUI_XComponent_UnregisterOnFrameCallback(node) != ARKUI_ERROR_CODE_NO_ERROR) {
            mbgl::Log::Warning(mbgl::Event::Setup, "Could not unregister XComponent frame callback");
        }
        frameCallbackRegistered = false;

        if (auto* api = nativeNodeApi()) {
            if (touchEventRegistered && node != nullptr) {
                api->unregisterNodeEvent(node, NODE_TOUCH_EVENT);
            }
            touchEventRegistered = false;

            if (nodeEventReceiverRegistered && node != nullptr) {
                if (api->removeNodeEventReceiver(node, onNodeEvent) != ARKUI_ERROR_CODE_NO_ERROR) {
                    mbgl::Log::Warning(mbgl::Event::Setup, "Could not remove XComponent touch event receiver");
                }
            }
            nodeEventReceiverRegistered = false;
        }

        if (surfaceCallbackRegistered && holder != nullptr && surfaceCallback != nullptr &&
            OH_ArkUI_SurfaceHolder_RemoveSurfaceCallback(holder, surfaceCallback) != ARKUI_ERROR_CODE_NO_ERROR) {
            mbgl::Log::Warning(mbgl::Event::Setup, "Could not remove XComponent surface callback");
        }
        surfaceCallbackRegistered = false;

        unregisterController(node, holder, this);
        clearMapState();

        if (nodeAddedToContent && content != nullptr && node != nullptr &&
            OH_ArkUI_NodeContent_RemoveNode(content, node) != ARKUI_ERROR_CODE_NO_ERROR) {
            mbgl::Log::Warning(mbgl::Event::Setup, "Could not remove native XComponent node from NodeContent");
        }
        nodeAddedToContent = false;
        content = nullptr;

        if (surfaceCallback != nullptr) {
            OH_ArkUI_SurfaceCallback_Dispose(surfaceCallback);
            surfaceCallback = nullptr;
        }

        if (holder != nullptr) {
            OH_ArkUI_SurfaceHolder_Dispose(holder);
            holder = nullptr;
        }

        if (ownsNode && node != nullptr) {
            if (auto* api = nativeNodeApi()) {
                api->disposeNode(node);
            }
        }

        node = nullptr;
    }

    void onSurfaceCreated(OH_ArkUI_SurfaceHolder* eventHolder) {
        if (closed || eventHolder != holder) {
            return;
        }

        window = OH_ArkUI_XComponent_GetNativeWindow(holder);
        surfaceVisible = window != nullptr;
        if (window == nullptr) {
            lastSurfaceError = "XComponent surface did not provide a native window";
            return;
        }

        updateSurface();
    }

    void onSurfaceChanged(OH_ArkUI_SurfaceHolder* eventHolder, std::uint64_t width_, std::uint64_t height_) {
        if (closed || eventHolder != holder) {
            return;
        }

        window = OH_ArkUI_XComponent_GetNativeWindow(holder);
        width = width_;
        height = height_;
        surfaceVisible = window != nullptr;
        updateSurface();
    }

    void onSurfaceDestroyed(OH_ArkUI_SurfaceHolder* eventHolder) {
        if (closed || eventHolder != holder) {
            return;
        }

        surfaceVisible = false;
        clearSurface();
        window = nullptr;
        width = 0;
        height = 0;
    }

    void onFrame() {
        if (closed) {
            return;
        }

        ++frameCallbackCount;
        renderFrame();
    }

    void handleNodeEvent(ArkUI_NodeEvent* event) {
        if (closed || event == nullptr || OH_ArkUI_NodeEvent_GetEventType(event) != NODE_TOUCH_EVENT) {
            return;
        }

        auto* inputEvent = OH_ArkUI_NodeEvent_GetInputEvent(event);
        if (inputEvent == nullptr || OH_ArkUI_UIInputEvent_GetType(inputEvent) != ARKUI_UIINPUTEVENT_TYPE_TOUCH) {
            return;
        }

        auto touchEvent = createTouchEvent(inputEvent);
        if (mbgl::ohos::handleTouchEvent(gesture, mapView.get(), touchEvent)) {
            renderFrame();
        }
    }

    void renderFrame() {
        if (!closed && renderingEnabled && surfaceVisible && mapView) {
            mapView->renderFrame();
        }
    }

    void reduceMemoryUse() {
        if (!closed && mapView) {
            mapView->reduceMemoryUse();
        }
    }

    void setStyleUrl(std::string style_) {
        if (closed) {
            return;
        }

        style = std::move(style_);
        ++styleGeneration;
        applyDesiredStyle();
    }

    void jumpTo(mbgl::CameraOptions cameraOptions) {
        if (closed) {
            return;
        }

        ensureMapView().jumpTo(std::move(cameraOptions));
        renderFrame();
    }

    void setPixelRatio(float newPixelRatio) {
        if (closed) {
            return;
        }

        if (!mapView) {
            if (pixelRatio == newPixelRatio) {
                return;
            }
            mapView = std::make_unique<mbgl::ohos::MapView>(newPixelRatio);
        } else if (mapView->getPixelRatio() == newPixelRatio) {
            return;
        } else {
            mapView->setPixelRatio(newPixelRatio);
        }

        pixelRatio = newPixelRatio;
        appliedStyleGeneration = 0;
        updateSurface();
    }

    void setBounds(mbgl::BoundOptions boundOptions) {
        if (closed) {
            return;
        }

        ensureMapView().setBounds(std::move(boundOptions));
        renderFrame();
    }

    void setRenderingEnabled(bool enabled) {
        if (closed) {
            return;
        }

        renderingEnabled = enabled;
        if (enabled) {
            renderFrame();
        }
    }

    bool setFrameRateRange(const OH_NativeXComponent_ExpectedRateRange& range) {
        if (closed || node == nullptr) {
            return false;
        }

        if (OH_ArkUI_XComponent_SetExpectedFrameRateRange(node, range) != ARKUI_ERROR_CODE_NO_ERROR) {
            return false;
        }

        frameRateRange = range;
        return true;
    }

    void setTileCacheEnabled(bool enabled) {
        if (closed) {
            return;
        }

        ensureMapView().setTileCacheEnabled(enabled);
    }

    void setClientOptions(std::string clientName, std::string clientVersion) {
        if (closed) {
            return;
        }

        if (!mapView && clientName.empty() && clientVersion.empty()) {
            return;
        }
        if (mapView && mapView->getClientName() == clientName && mapView->getClientVersion() == clientVersion) {
            return;
        }

        ensureMapView().setClientOptions(std::move(clientName), std::move(clientVersion));
        appliedStyleGeneration = 0;
        applyDesiredStyle();
        renderFrame();
    }

    void setResourceOptions(const std::optional<std::string>& apiKey,
                            const std::optional<std::string>& cachePath,
                            const std::optional<std::string>& assetPath) {
        if (closed) {
            return;
        }
        if (!mapView && !apiKey && !cachePath && !assetPath) {
            return;
        }

        auto& currentMapView = ensureMapView();
        auto resourceOptions = currentMapView.getResourceOptions().clone();
        if (apiKey) {
            resourceOptions.withApiKey(*apiKey);
        }
        if (cachePath) {
            resourceOptions.withCachePath(*cachePath);
        }
        if (assetPath) {
            resourceOptions.withAssetPath(*assetPath);
        }

        if (exposedResourceOptionsEqual(resourceOptions, currentMapView.getResourceOptions())) {
            return;
        }

        currentMapView.setResourceOptions(std::move(resourceOptions));
        appliedStyleGeneration = 0;
        applyDesiredStyle();
        renderFrame();
    }

    float getPixelRatio() const { return mapView ? mapView->getPixelRatio() : pixelRatio; }

    std::vector<std::string> getStyleAttributions() {
        if (closed) {
            return {};
        }

        return ensureMapView().getStyleAttributions();
    }

    mbgl::CameraOptions getCameraOptions() {
        if (closed) {
            return {};
        }

        return ensureMapView().getCameraOptions();
    }

    napi_value createSurfaceStateObject(napi_env env) {
        updateFrameRates();

        const bool hasWindow = window != nullptr;
        const bool hasSurface = hasWindow && width > 0 && height > 0;
        const bool hasMap = mapView && mapView->hasMap();
        const bool needsRender = mapView && mapView->hasPendingRender();
        const bool styleLoaded = mapView && mapView->hasLoadedStyle();
        const bool mapLoaded = mapView && mapView->hasLoadedMap();
        const bool fullyLoaded = mapView && mapView->isFullyLoaded();
        std::string backendLabel;
        if (mapView && mapView->getGlesContextClientVersion() > 0) {
            backendLabel = std::string{"OpenGL ES "} + std::to_string(mapView->getGlesContextClientVersion());
        } else if (mapView && !mapView->getRendererDiagnostic().empty()) {
            backendLabel = "Vulkan";
        }

        napi_value object = nullptr;
        napi_create_object(env, &object);
        setSizeProperty(env, object, "width", width);
        setSizeProperty(env, object, "height", height);
        setBoolProperty(env, object, "hasWindow", hasWindow);
        setBoolProperty(env, object, "hasSurface", hasSurface);
        setBoolProperty(env, object, "hasMap", hasMap);
        setBoolProperty(env, object, "needsRender", needsRender);
        setBoolProperty(env, object, "styleLoaded", styleLoaded);
        setBoolProperty(env, object, "mapLoaded", mapLoaded);
        setBoolProperty(env, object, "fullyLoaded", fullyLoaded);
        setDoubleProperty(env, object, "renderedFrameRate", renderedFrameRate);
        setDoubleProperty(env, object, "frameCallbackRate", frameCallbackRate);
        if (!backendLabel.empty()) {
            setStringProperty(env, object, "backend", backendLabel);
        }
        setBoolProperty(env, object, "surfaceVisible", surfaceVisible);
        if (!lastSurfaceError.empty()) {
            setStringProperty(env, object, "lastSurfaceError", lastSurfaceError);
        }
        if (mapView && !mapView->getLastMapLoadError().empty()) {
            setStringProperty(env, object, "lastMapLoadError", mapView->getLastMapLoadError());
        }
        if (mapView && !mapView->getLastRenderError().empty()) {
            setStringProperty(env, object, "lastRenderError", mapView->getLastRenderError());
        }
        if (mapView && !mapView->getLastStyleImageMissing().empty()) {
            setStringProperty(env, object, "lastStyleImageMissing", mapView->getLastStyleImageMissing());
        }
        if (mapView && !mapView->getLastGlyphsError().empty()) {
            setStringProperty(env, object, "lastGlyphsError", mapView->getLastGlyphsError());
        }
        if (mapView && !mapView->getLastSpritesError().empty()) {
            setStringProperty(env, object, "lastSpritesError", mapView->getLastSpritesError());
        }
        return object;
    }

private:
    SurfaceController(ArkUI_NodeHandle node_, ArkUI_NodeContentHandle content_, bool ownsNode_)
        : node(node_),
          content(content_),
          ownsNode(ownsNode_) {}

    void initialize() {
        if (node == nullptr) {
            throw std::invalid_argument("Expected an XComponent node");
        }

        holder = OH_ArkUI_SurfaceHolder_Create(node);
        if (holder == nullptr) {
            throw std::runtime_error("Could not create XComponent surface holder");
        }

        surfaceCallback = OH_ArkUI_SurfaceCallback_Create();
        if (surfaceCallback == nullptr) {
            throw std::runtime_error("Could not create XComponent surface callback");
        }

        OH_ArkUI_SurfaceCallback_SetSurfaceCreatedEvent(surfaceCallback, handleSurfaceCreated);
        OH_ArkUI_SurfaceCallback_SetSurfaceChangedEvent(surfaceCallback, handleSurfaceChanged);
        OH_ArkUI_SurfaceCallback_SetSurfaceDestroyedEvent(surfaceCallback, handleSurfaceDestroyed);

        registerController(node, holder, shared_from_this());

        if (OH_ArkUI_SurfaceHolder_AddSurfaceCallback(holder, surfaceCallback) != ARKUI_ERROR_CODE_NO_ERROR) {
            throw std::runtime_error("Could not add XComponent surface callback");
        }
        surfaceCallbackRegistered = true;

        if (OH_ArkUI_XComponent_RegisterOnFrameCallback(node, onFrame) != ARKUI_ERROR_CODE_NO_ERROR) {
            throw std::runtime_error("Could not register XComponent frame callback");
        }
        frameCallbackRegistered = true;

        auto& api = requireNativeNodeApi();
        if (api.addNodeEventReceiver(node, onNodeEvent) != ARKUI_ERROR_CODE_NO_ERROR) {
            throw std::runtime_error("Could not add XComponent touch event receiver");
        }
        nodeEventReceiverRegistered = true;

        if (api.registerNodeEvent(node, NODE_TOUCH_EVENT, 0, nullptr) != ARKUI_ERROR_CODE_NO_ERROR) {
            throw std::runtime_error("Could not register XComponent touch event");
        }
        touchEventRegistered = true;
    }

    void addNodeToContent() {
        if (content == nullptr || node == nullptr) {
            throw std::invalid_argument("Expected a NodeContent host");
        }
        if (OH_ArkUI_NodeContent_AddNode(content, node) != ARKUI_ERROR_CODE_NO_ERROR) {
            throw std::runtime_error("Could not add native XComponent node to NodeContent");
        }
        nodeAddedToContent = true;
    }

    static void handleSurfaceCreated(OH_ArkUI_SurfaceHolder* holder) {
        if (auto controller = findController(holder)) {
            controller->onSurfaceCreated(holder);
        }
    }

    static void handleSurfaceChanged(OH_ArkUI_SurfaceHolder* holder, std::uint64_t width, std::uint64_t height) {
        if (auto controller = findController(holder)) {
            controller->onSurfaceChanged(holder, width, height);
        }
    }

    static void handleSurfaceDestroyed(OH_ArkUI_SurfaceHolder* holder) {
        if (auto controller = findController(holder)) {
            controller->onSurfaceDestroyed(holder);
        }
    }

    static void onFrame(ArkUI_NodeHandle node, std::uint64_t, std::uint64_t) {
        if (auto controller = findController(node)) {
            controller->onFrame();
        }
    }

    static void onNodeEvent(ArkUI_NodeEvent* event) {
        if (event == nullptr) {
            return;
        }
        if (auto controller = findController(OH_ArkUI_NodeEvent_GetNodeHandle(event))) {
            controller->handleNodeEvent(event);
        }
    }

    float coordinateScale() const { return std::isfinite(pixelRatio) && pixelRatio > 0.0f ? pixelRatio : 1.0f; }

    mbgl::ohos::TouchPoint touchPointAt(const ArkUI_UIInputEvent* event, std::uint32_t pointerIndex) const {
        const auto scale = coordinateScale();
        return {
            OH_ArkUI_PointerEvent_GetPointerId(event, pointerIndex),
            OH_ArkUI_PointerEvent_GetXByIndex(event, pointerIndex) * scale,
            OH_ArkUI_PointerEvent_GetYByIndex(event, pointerIndex) * scale,
        };
    }

    mbgl::ohos::TouchEvent createTouchEvent(const ArkUI_UIInputEvent* event) const {
        mbgl::ohos::TouchEvent touchEvent;
        touchEvent.action = toTouchAction(OH_ArkUI_UIInputEvent_GetAction(event));

        const auto pointerCount = OH_ArkUI_PointerEvent_GetPointerCount(event);
        touchEvent.points.reserve(pointerCount);
        for (std::uint32_t i = 0; i < pointerCount; ++i) {
            touchEvent.points.push_back(touchPointAt(event, i));
        }

        std::uint32_t changedPointerId = 0;
        const bool hasChangedPointerId = OH_ArkUI_PointerEvent_GetChangedPointerId(event, &changedPointerId) ==
                                         ARKUI_ERROR_CODE_NO_ERROR;

        std::optional<std::uint32_t> changedIndex;
        if (hasChangedPointerId) {
            touchEvent.id = static_cast<int32_t>(changedPointerId);
            for (std::uint32_t i = 0; i < pointerCount; ++i) {
                if (touchEvent.points[i].id == touchEvent.id) {
                    changedIndex = i;
                    break;
                }
            }
        } else if (!touchEvent.points.empty()) {
            changedIndex = 0;
        }

        if (changedIndex) {
            const auto& changedPoint = touchEvent.points[*changedIndex];
            touchEvent.id = changedPoint.id;
            touchEvent.x = changedPoint.x;
            touchEvent.y = changedPoint.y;
        } else {
            const auto scale = coordinateScale();
            touchEvent.x = OH_ArkUI_PointerEvent_GetX(event) * scale;
            touchEvent.y = OH_ArkUI_PointerEvent_GetY(event) * scale;
        }

        return touchEvent;
    }

    void clearSurface() {
        if (mapView) {
            if (mbgl::ohos::hasActiveGesture(gesture)) {
                mapView->setGestureInProgress(false);
            }
            mapView->clearSurface();
        }
        appliedStyleGeneration = 0;
        mbgl::ohos::resetGestureState(gesture);
    }

    void clearMapState() {
        clearSurface();
        window = nullptr;
        width = 0;
        height = 0;
        surfaceVisible = false;
        mapView.reset();
    }

    void applyDesiredStyle() {
        if (closed || !mapView || !mapView->hasMap() || style.empty() || appliedStyleGeneration == styleGeneration) {
            return;
        }

        mapView->setStyleURL(style);
        appliedStyleGeneration = styleGeneration;
    }

    void updateSurface() {
        if (closed) {
            return;
        }

        auto& currentMapView = ensureMapView();
        if (window == nullptr || width == 0 || height == 0) {
            clearSurface();
            return;
        }

        try {
            const bool needsStyleReapply = !currentMapView.hasMap() || currentMapView.getNativeWindow() != window;
            currentMapView.setSurface(window, toSize(width, height));
            if (needsStyleReapply) {
                appliedStyleGeneration = 0;
            }
            applyDesiredStyle();
            renderFrame();
            lastSurfaceError.clear();
        } catch (const std::exception& exception) {
            lastSurfaceError = exception.what();
            clearSurface();
            mbgl::Log::Error(mbgl::Event::Render, exception.what());
        }
    }

    mbgl::ohos::MapView& ensureMapView() {
        if (!mapView) {
            mapView = std::make_unique<mbgl::ohos::MapView>(pixelRatio);
        }
        return *mapView;
    }

    void updateFrameRates() {
        const auto now = std::chrono::steady_clock::now();
        const auto renderedFrames = mapView ? mapView->getRenderedFrameCount() : 0;
        const auto frameCallbacks = frameCallbackCount;

        if (frameRateSampleTime.time_since_epoch().count() == 0) {
            frameRateSampleTime = now;
            frameRateRenderedFrames = renderedFrames;
            frameRateCallbacks = frameCallbacks;
            return;
        }

        const auto elapsed = std::chrono::duration<double>(now - frameRateSampleTime).count();
        if (elapsed <= 0.0) {
            return;
        }

        renderedFrameRate = static_cast<double>(renderedFrames - frameRateRenderedFrames) / elapsed;
        frameCallbackRate = static_cast<double>(frameCallbacks - frameRateCallbacks) / elapsed;
        frameRateSampleTime = now;
        frameRateRenderedFrames = renderedFrames;
        frameRateCallbacks = frameCallbacks;
    }

    ArkUI_NodeHandle node = nullptr;
    ArkUI_NodeContentHandle content = nullptr;
    OH_ArkUI_SurfaceHolder* holder = nullptr;
    OH_ArkUI_SurfaceCallback* surfaceCallback = nullptr;
    OHNativeWindow* window = nullptr;
    std::unique_ptr<mbgl::ohos::MapView> mapView;
    std::string style;
    std::uint64_t styleGeneration = 0;
    std::uint64_t appliedStyleGeneration = 0;
    float pixelRatio = 1.0f;
    std::uint64_t width = 0;
    std::uint64_t height = 0;
    bool renderingEnabled = true;
    bool frameCallbackRegistered = false;
    bool nodeEventReceiverRegistered = false;
    bool touchEventRegistered = false;
    bool surfaceCallbackRegistered = false;
    bool ownsNode = false;
    bool nodeAddedToContent = false;
    bool closed = false;
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

using ControllerHandle = std::shared_ptr<SurfaceController>;

napi_value destroy(napi_env env, napi_callback_info info);
napi_value renderFrame(napi_env env, napi_callback_info info);
napi_value reduceMemoryUse(napi_env env, napi_callback_info info);
napi_value setStyleUrl(napi_env env, napi_callback_info info);
napi_value jumpTo(napi_env env, napi_callback_info info);
napi_value setPixelRatio(napi_env env, napi_callback_info info);
napi_value setBounds(napi_env env, napi_callback_info info);
napi_value setRenderingEnabled(napi_env env, napi_callback_info info);
napi_value setFrameRateRange(napi_env env, napi_callback_info info);
napi_value setTileCacheEnabled(napi_env env, napi_callback_info info);
napi_value setClientOptions(napi_env env, napi_callback_info info);
napi_value setResourceOptions(napi_env env, napi_callback_info info);
napi_value getPixelRatio(napi_env env, napi_callback_info info);
napi_value getStyleAttributions(napi_env env, napi_callback_info info);
napi_value getSurfaceState(napi_env env, napi_callback_info info);
napi_value getCameraOptions(napi_env env, napi_callback_info info);

ControllerHandle resolveController(napi_env env, napi_value thisArg) {
    void* nativeController = nullptr;
    if (napi_unwrap(env, thisArg, &nativeController) != napi_ok || nativeController == nullptr) {
        throwError(env, "Expected a MapLibre XComponent context");
        return nullptr;
    }

    auto* controller = static_cast<ControllerHandle*>(nativeController);
    if (controller == nullptr || !*controller) {
        throwError(env, "Expected a MapLibre XComponent context");
        return nullptr;
    }

    return *controller;
}

void finalizeController(napi_env, void* data, void*) {
    auto* controller = static_cast<ControllerHandle*>(data);
    if (controller != nullptr) {
        if (*controller) {
            (*controller)->close();
        }
        delete controller;
    }
}

const napi_property_descriptor* contextProperties(std::size_t& count) {
    static napi_property_descriptor properties[] = {
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
    count = sizeof(properties) / sizeof(properties[0]);
    return properties;
}

napi_value createContextObject(napi_env env, ControllerHandle controller) {
    napi_value object = nullptr;
    if (napi_create_object(env, &object) != napi_ok || object == nullptr) {
        return throwError(env, "Could not create MapLibre XComponent context");
    }

    std::size_t propertyCount = 0;
    const auto* properties = contextProperties(propertyCount);
    if (napi_define_properties(env, object, propertyCount, properties) != napi_ok) {
        return throwError(env, "Could not define MapLibre XComponent context");
    }

    auto* boxedController = new ControllerHandle(std::move(controller));
    if (napi_wrap(env, object, boxedController, finalizeController, nullptr, nullptr) != napi_ok) {
        delete boxedController;
        return throwError(env, "Could not wrap MapLibre XComponent context");
    }

    return object;
}

ControllerHandle createController(ArkUI_NodeContentHandle content) {
    return SurfaceController::create(content);
}

napi_value createMap(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    if (napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected a NodeContent host");
    }

    ArkUI_NodeContentHandle content = nullptr;
    if (OH_ArkUI_GetNodeContentFromNapiValue(env, argv[0], &content) != ARKUI_ERROR_CODE_NO_ERROR ||
        content == nullptr) {
        return throwError(env, "Expected a NodeContent host");
    }

    try {
        return createContextObject(env, createController(content));
    } catch (const std::exception& exception) {
        return throwError(env, exception.what());
    }
}

napi_value destroy(napi_env env, napi_callback_info info) {
    std::size_t argc = 0;
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, nullptr, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a MapLibre XComponent context");
    }

    if (auto controller = resolveController(env, thisArg)) {
        controller->close();
    }
    return getUndefined(env);
}

napi_value renderFrame(napi_env env, napi_callback_info info) {
    std::size_t argc = 0;
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, nullptr, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a MapLibre XComponent context");
    }

    if (auto controller = resolveController(env, thisArg)) {
        controller->renderFrame();
    }
    return getUndefined(env);
}

napi_value reduceMemoryUse(napi_env env, napi_callback_info info) {
    std::size_t argc = 0;
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, nullptr, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a MapLibre XComponent context");
    }

    if (auto controller = resolveController(env, thisArg)) {
        controller->reduceMemoryUse();
    }
    return getUndefined(env);
}

napi_value setStyleUrl(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected style URL");
    }

    auto controller = resolveController(env, thisArg);
    if (!controller) {
        return getUndefined(env);
    }

    std::string style;
    if (!getString(env, argv[0], style)) {
        return throwError(env, "Expected style URL string");
    }

    controller->setStyleUrl(std::move(style));
    return getUndefined(env);
}

napi_value jumpTo(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected camera options");
    }

    auto controller = resolveController(env, thisArg);
    if (!controller) {
        return getUndefined(env);
    }

    mbgl::CameraOptions cameraOptions;
    if (!getCameraOptionsObject(env, argv[0], cameraOptions)) {
        return throwError(env, "Expected valid camera options object");
    }

    try {
        controller->jumpTo(std::move(cameraOptions));
    } catch (const std::exception& exception) {
        return throwError(env, exception.what());
    }
    return getUndefined(env);
}

napi_value setPixelRatio(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected a pixel ratio");
    }

    auto controller = resolveController(env, thisArg);
    if (!controller) {
        return getUndefined(env);
    }

    double pixelRatio = 0.0;
    if (!getDouble(env, argv[0], pixelRatio) || !std::isfinite(pixelRatio) || pixelRatio <= 0.0) {
        return throwError(env, "Expected a finite positive pixel ratio");
    }

    try {
        controller->setPixelRatio(static_cast<float>(pixelRatio));
    } catch (const std::exception& exception) {
        return throwError(env, exception.what());
    }
    return getUndefined(env);
}

napi_value setBounds(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected bounds options");
    }

    auto controller = resolveController(env, thisArg);
    if (!controller) {
        return getUndefined(env);
    }

    mbgl::BoundOptions boundOptions;
    if (!getBoundOptions(env, argv[0], boundOptions)) {
        return throwError(env, "Expected valid bounds options object");
    }

    try {
        controller->setBounds(std::move(boundOptions));
    } catch (const std::exception& exception) {
        return throwError(env, exception.what());
    }
    return getUndefined(env);
}

napi_value setRenderingEnabled(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected rendering enabled boolean");
    }

    auto controller = resolveController(env, thisArg);
    if (!controller) {
        return getUndefined(env);
    }

    bool enabled = false;
    if (!getBool(env, argv[0], enabled)) {
        return throwError(env, "Expected rendering enabled boolean");
    }

    controller->setRenderingEnabled(enabled);
    return getUndefined(env);
}

napi_value setFrameRateRange(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected frame-rate range");
    }

    auto controller = resolveController(env, thisArg);
    if (!controller) {
        return getUndefined(env);
    }

    OH_NativeXComponent_ExpectedRateRange range{};
    if (!parseFrameRateRange(env, argv[0], range)) {
        return throwError(env, "Expected frame-rate range object with positive min <= expected <= max");
    }

    if (!controller->setFrameRateRange(range)) {
        return throwError(env, "Could not apply XComponent frame-rate range");
    }
    return getUndefined(env);
}

napi_value setTileCacheEnabled(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected tile cache enabled boolean");
    }

    auto controller = resolveController(env, thisArg);
    if (!controller) {
        return getUndefined(env);
    }

    bool enabled = false;
    if (!getBool(env, argv[0], enabled)) {
        return throwError(env, "Expected tile cache enabled boolean");
    }

    controller->setTileCacheEnabled(enabled);
    return getUndefined(env);
}

napi_value setClientOptions(napi_env env, napi_callback_info info) {
    std::size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected client name and optional client version");
    }

    auto controller = resolveController(env, thisArg);
    if (!controller) {
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
        controller->setClientOptions(std::move(clientName), std::move(clientVersion));
    } catch (const std::exception& exception) {
        return throwError(env, exception.what());
    }
    return getUndefined(env);
}

napi_value setResourceOptions(napi_env env, napi_callback_info info) {
    std::size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr) != napi_ok || argc < 1 || argv[0] == nullptr) {
        return throwError(env, "Expected resource options");
    }

    auto controller = resolveController(env, thisArg);
    if (!controller) {
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
        controller->setResourceOptions(apiKey, cachePath, assetPath);
    } catch (const std::exception& exception) {
        return throwError(env, exception.what());
    }
    return getUndefined(env);
}

napi_value getPixelRatio(napi_env env, napi_callback_info info) {
    std::size_t argc = 0;
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, nullptr, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a MapLibre XComponent context");
    }

    auto controller = resolveController(env, thisArg);
    if (!controller) {
        return getUndefined(env);
    }

    napi_value result = nullptr;
    napi_create_double(env, controller->getPixelRatio(), &result);
    return result;
}

napi_value getStyleAttributions(napi_env env, napi_callback_info info) {
    std::size_t argc = 0;
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, nullptr, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a MapLibre XComponent context");
    }

    auto controller = resolveController(env, thisArg);
    if (!controller) {
        return getUndefined(env);
    }

    return createStringArray(env, controller->getStyleAttributions());
}

napi_value getSurfaceState(napi_env env, napi_callback_info info) {
    std::size_t argc = 0;
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, nullptr, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a MapLibre XComponent context");
    }

    auto controller = resolveController(env, thisArg);
    if (!controller) {
        return getUndefined(env);
    }

    return controller->createSurfaceStateObject(env);
}

napi_value getCameraOptions(napi_env env, napi_callback_info info) {
    std::size_t argc = 0;
    napi_value thisArg = nullptr;
    if (napi_get_cb_info(env, info, &argc, nullptr, &thisArg, nullptr) != napi_ok) {
        return throwError(env, "Expected a MapLibre XComponent context");
    }

    auto controller = resolveController(env, thisArg);
    if (!controller) {
        return getUndefined(env);
    }

    return createCameraOptionsObject(env, controller->getCameraOptions());
}

napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor properties[] = {
        {"createMap", nullptr, createMap, nullptr, nullptr, nullptr, napi_default, nullptr},
    };

    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    return exports;
}

} // namespace

NAPI_MODULE(maplibre_native_ohos, Init)
