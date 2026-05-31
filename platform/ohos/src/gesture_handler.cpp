#include "gesture_handler.hpp"

#include "map_view.hpp"

#include <mbgl/map/camera.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>

namespace mbgl {
namespace ohos {
namespace {

struct TouchPoint {
    int32_t id = 0;
    float x = 0.0f;
    float y = 0.0f;
};

struct PinchState {
    float centerX = 0.0f;
    float centerY = 0.0f;
    double distance = 0.0;
    double angle = 0.0;
};

enum class TouchAction {
    Down,
    Move,
    Up,
    Cancel,
    Unknown,
};

constexpr auto MaxTapDuration = std::chrono::milliseconds(300);
constexpr auto MaxDoubleTapInterval = std::chrono::milliseconds(300);
constexpr double MaxTapMovement = 24.0;
constexpr double MaxDoubleTapDistance = 48.0;
constexpr double DoubleTapZoomScale = 2.0;
constexpr auto FlingAnimationDuration = std::chrono::milliseconds(450);
constexpr double FlingDistanceSeconds = 0.25;
constexpr double MinFlingVelocity = 250.0;
constexpr double MaxFlingDistance = 1200.0;
constexpr double Pi = 3.14159265358979323846;
constexpr double TwoPi = 2.0 * Pi;
constexpr double ShoveStartMovement = 16.0;
constexpr double ShoveVerticalRatio = 1.5;
constexpr double ShoveMaxDistanceScaleDelta = 0.06;
constexpr double ShoveMaxAngleDelta = Pi / 18.0;
constexpr double ShovePixelChangeFactor = 0.1;

double distanceBetween(float firstX, float firstY, float secondX, float secondY) {
    return std::hypot(static_cast<double>(secondX - firstX), static_cast<double>(secondY - firstY));
}

double normalizedAngleDelta(double firstAngle, double secondAngle) {
    const double delta = std::remainder(secondAngle - firstAngle, TwoPi);
    return std::abs(delta);
}

double clampFlingDistance(double distance) {
    if (!std::isfinite(distance)) {
        return 0.0;
    }
    return std::clamp(distance, -MaxFlingDistance, MaxFlingDistance);
}

std::optional<TouchPoint> touchPointForEvent(const OH_NativeXComponent_TouchEvent& event,
                                             std::optional<int32_t> activeId) {
    if (event.numPoints == 0) {
        if (activeId && event.id != *activeId) {
            return std::nullopt;
        }
        return TouchPoint{event.id, event.x, event.y};
    }

    for (std::uint32_t i = 0; i < event.numPoints; ++i) {
        const auto& point = event.touchPoints[i];
        if (!activeId || point.id == *activeId) {
            return TouchPoint{point.id, point.x, point.y};
        }
    }

    return std::nullopt;
}

std::optional<std::array<TouchPoint, 2>> touchPairForEvent(const OH_NativeXComponent_TouchEvent& event) {
    if (event.numPoints < 2) {
        return std::nullopt;
    }

    return std::array<TouchPoint, 2>{
        TouchPoint{event.touchPoints[0].id, event.touchPoints[0].x, event.touchPoints[0].y},
        TouchPoint{event.touchPoints[1].id, event.touchPoints[1].x, event.touchPoints[1].y},
    };
}

std::optional<TouchPoint> touchPointForInputEvent(const ArkUI_UIInputEvent* event, std::optional<int32_t> activeId) {
    if (event == nullptr) {
        return std::nullopt;
    }

    const auto pointerCount = OH_ArkUI_PointerEvent_GetPointerCount(event);
    if (pointerCount == 0) {
        return TouchPoint{activeId.value_or(0), OH_ArkUI_PointerEvent_GetX(event), OH_ArkUI_PointerEvent_GetY(event)};
    }

    for (std::uint32_t i = 0; i < pointerCount; ++i) {
        const auto id = OH_ArkUI_PointerEvent_GetPointerId(event, i);
        if (!activeId || id == *activeId) {
            return TouchPoint{
                id, OH_ArkUI_PointerEvent_GetXByIndex(event, i), OH_ArkUI_PointerEvent_GetYByIndex(event, i)};
        }
    }

    return std::nullopt;
}

std::optional<std::array<TouchPoint, 2>> touchPairForInputEvent(const ArkUI_UIInputEvent* event) {
    if (event == nullptr || OH_ArkUI_PointerEvent_GetPointerCount(event) < 2) {
        return std::nullopt;
    }

    return std::array<TouchPoint, 2>{
        TouchPoint{
            OH_ArkUI_PointerEvent_GetPointerId(event, 0),
            OH_ArkUI_PointerEvent_GetXByIndex(event, 0),
            OH_ArkUI_PointerEvent_GetYByIndex(event, 0),
        },
        TouchPoint{
            OH_ArkUI_PointerEvent_GetPointerId(event, 1),
            OH_ArkUI_PointerEvent_GetXByIndex(event, 1),
            OH_ArkUI_PointerEvent_GetYByIndex(event, 1),
        },
    };
}

std::optional<PinchState> pinchStateForPoints(const std::optional<std::array<TouchPoint, 2>>& points) {
    if (!points) {
        return std::nullopt;
    }

    const auto& first = (*points)[0];
    const auto& second = (*points)[1];
    const auto distance = distanceBetween(first.x, first.y, second.x, second.y);
    if (!std::isfinite(distance) || distance <= 0.0) {
        return std::nullopt;
    }

    return PinchState{
        (first.x + second.x) * 0.5f,
        (first.y + second.y) * 0.5f,
        distance,
        std::atan2(static_cast<double>(second.y - first.y), static_cast<double>(second.x - first.x)),
    };
}

TouchAction toTouchAction(OH_NativeXComponent_TouchEventType type) {
    switch (type) {
        case OH_NATIVEXCOMPONENT_DOWN:
            return TouchAction::Down;
        case OH_NATIVEXCOMPONENT_MOVE:
            return TouchAction::Move;
        case OH_NATIVEXCOMPONENT_UP:
            return TouchAction::Up;
        case OH_NATIVEXCOMPONENT_CANCEL:
            return TouchAction::Cancel;
        case OH_NATIVEXCOMPONENT_UNKNOWN:
            return TouchAction::Unknown;
    }
    return TouchAction::Unknown;
}

TouchAction toTouchAction(int32_t action) {
    switch (action) {
        case UI_TOUCH_EVENT_ACTION_DOWN:
            return TouchAction::Down;
        case UI_TOUCH_EVENT_ACTION_MOVE:
            return TouchAction::Move;
        case UI_TOUCH_EVENT_ACTION_UP:
            return TouchAction::Up;
        case UI_TOUCH_EVENT_ACTION_CANCEL:
            return TouchAction::Cancel;
        default:
            return TouchAction::Unknown;
    }
}

bool handleTouchAction(GestureState& state,
                       MapView* mapView,
                       TouchAction action,
                       std::optional<TouchPoint> point,
                       std::optional<PinchState> pinch,
                       bool hasMultiplePoints) {
    bool needsRender = false;

    auto handleTap = [&](const TouchPoint& tap) {
        if (!state.tapCandidate) {
            return false;
        }

        const auto now = std::chrono::steady_clock::now();
        const auto tapDuration = now - state.tapStartTime;
        state.tapCandidate = false;
        if (tapDuration > MaxTapDuration ||
            distanceBetween(tap.x, tap.y, state.tapStartX, state.tapStartY) > MaxTapMovement) {
            state.lastTapActive = false;
            return false;
        }

        const bool doubleTap = state.lastTapActive && now - state.lastTapTime <= MaxDoubleTapInterval &&
                               distanceBetween(tap.x, tap.y, state.lastTapX, state.lastTapY) <= MaxDoubleTapDistance;
        if (doubleTap) {
            state.lastTapActive = false;
            if (mapView) {
                mapView->scaleBy(DoubleTapZoomScale, tap.x, tap.y);
                needsRender = true;
            }
            return true;
        }

        state.lastTapActive = true;
        state.lastTapX = tap.x;
        state.lastTapY = tap.y;
        state.lastTapTime = now;
        return false;
    };

    auto beginPinch = [&] {
        if (!pinch) {
            return;
        }
        state.touchActive = false;
        state.tapCandidate = false;
        state.lastTapActive = false;
        state.touchVelocityX = 0.0;
        state.touchVelocityY = 0.0;
        state.pinchActive = true;
        state.shoveActive = false;
        state.pinchStartCenterX = pinch->centerX;
        state.pinchStartCenterY = pinch->centerY;
        state.pinchStartDistance = pinch->distance;
        state.pinchStartAngle = pinch->angle;
        state.pinchCenterX = pinch->centerX;
        state.pinchCenterY = pinch->centerY;
        state.pinchDistance = pinch->distance;
        state.pinchAngle = pinch->angle;
        if (mapView) {
            mapView->setGestureInProgress(true);
        }
    };

    switch (action) {
        case TouchAction::Down: {
            if (pinch) {
                beginPinch();
                return false;
            }
            if (!point) {
                return false;
            }
            state.touchActive = true;
            state.pinchActive = false;
            state.shoveActive = false;
            state.touchId = point->id;
            state.touchX = point->x;
            state.touchY = point->y;
            state.tapCandidate = true;
            state.tapStartX = point->x;
            state.tapStartY = point->y;
            state.tapStartTime = std::chrono::steady_clock::now();
            state.touchSampleTime = state.tapStartTime;
            state.touchVelocityX = 0.0;
            state.touchVelocityY = 0.0;
            if (mapView) {
                mapView->setGestureInProgress(true);
            }
            break;
        }
        case TouchAction::Move: {
            if (pinch) {
                if (!state.pinchActive) {
                    beginPinch();
                    return false;
                }
                if (!mapView) {
                    return false;
                }
                const double totalDx = pinch->centerX - state.pinchStartCenterX;
                const double totalDy = pinch->centerY - state.pinchStartCenterY;
                const double totalVerticalMovement = std::abs(totalDy);
                const double totalHorizontalMovement = std::abs(totalDx);
                const double distanceScaleDelta = state.pinchStartDistance > 0.0
                                                      ? std::abs((pinch->distance / state.pinchStartDistance) - 1.0)
                                                      : 0.0;
                const double angleDelta = normalizedAngleDelta(state.pinchStartAngle, pinch->angle);
                if (!state.shoveActive && totalVerticalMovement >= ShoveStartMovement &&
                    totalVerticalMovement >= totalHorizontalMovement * ShoveVerticalRatio &&
                    distanceScaleDelta <= ShoveMaxDistanceScaleDelta && angleDelta <= ShoveMaxAngleDelta) {
                    state.shoveActive = true;
                }

                const double scale = pinch->distance / state.pinchDistance;
                const double dx = pinch->centerX - state.pinchCenterX;
                const double dy = pinch->centerY - state.pinchCenterY;
                const double previousAngle = state.pinchAngle;
                state.pinchCenterX = pinch->centerX;
                state.pinchCenterY = pinch->centerY;
                state.pinchDistance = pinch->distance;
                state.pinchAngle = pinch->angle;
                if (state.shoveActive) {
                    mapView->pitchBy(-dy * ShovePixelChangeFactor);
                    return true;
                }

                mapView->scaleBy(scale, pinch->centerX, pinch->centerY);
                mapView->rotateBy(previousAngle, pinch->angle, pinch->centerX, pinch->centerY);
                mapView->moveBy(dx, dy);
                return true;
            }
            if (state.pinchActive) {
                return false;
            }
            if (!state.touchActive || hasMultiplePoints) {
                return false;
            }
            if (!point || !mapView) {
                return false;
            }
            const auto now = std::chrono::steady_clock::now();
            const double dx = point->x - state.touchX;
            const double dy = point->y - state.touchY;
            state.touchX = point->x;
            state.touchY = point->y;
            const auto elapsed = std::chrono::duration<double>(now - state.touchSampleTime).count();
            if (elapsed > 0.0) {
                state.touchVelocityX = dx / elapsed;
                state.touchVelocityY = dy / elapsed;
                state.touchSampleTime = now;
            }
            if (distanceBetween(point->x, point->y, state.tapStartX, state.tapStartY) > MaxTapMovement) {
                state.tapCandidate = false;
            }
            mapView->moveBy(dx, dy);
            needsRender = true;
            break;
        }
        case TouchAction::Up:
        case TouchAction::Cancel: {
            const bool canHandleTap = action == TouchAction::Up && !state.pinchActive && point;
            const double velocity = std::hypot(state.touchVelocityX, state.touchVelocityY);
            const bool shouldFling = action == TouchAction::Up && !state.tapCandidate && !state.pinchActive &&
                                     velocity >= MinFlingVelocity;
            state.touchActive = false;
            state.pinchActive = false;
            state.shoveActive = false;
            if (mapView) {
                mapView->setGestureInProgress(false);
                if (!canHandleTap || !handleTap(*point)) {
                    if (shouldFling) {
                        mapView->moveBy(clampFlingDistance(state.touchVelocityX * FlingDistanceSeconds),
                                        clampFlingDistance(state.touchVelocityY * FlingDistanceSeconds),
                                        mbgl::AnimationOptions{FlingAnimationDuration});
                    }
                    needsRender = true;
                }
            }
            state.tapCandidate = false;
            state.touchVelocityX = 0.0;
            state.touchVelocityY = 0.0;
            break;
        }
        case TouchAction::Unknown:
            break;
    }

    return needsRender;
}

} // namespace

bool hasActiveGesture(const GestureState& state) {
    return state.touchActive || state.pinchActive;
}

void resetGestureState(GestureState& state) {
    state = GestureState();
}

bool handleTouchEvent(GestureState& state, MapView* mapView, const OH_NativeXComponent_TouchEvent& event) {
    const auto action = toTouchAction(event.type);
    const auto point = action == TouchAction::Move ? touchPointForEvent(event, state.touchId)
                                                   : touchPointForEvent(event, std::nullopt);
    const auto pinch = pinchStateForPoints(touchPairForEvent(event));
    return handleTouchAction(state, mapView, action, point, pinch, event.numPoints > 1);
}

bool handleInputEvent(GestureState& state, MapView* mapView, const ArkUI_UIInputEvent* event) {
    if (event == nullptr || OH_ArkUI_UIInputEvent_GetType(event) != ARKUI_UIINPUTEVENT_TYPE_TOUCH) {
        return false;
    }

    const auto action = toTouchAction(OH_ArkUI_UIInputEvent_GetAction(event));
    const auto point = action == TouchAction::Move ? touchPointForInputEvent(event, state.touchId)
                                                   : touchPointForInputEvent(event, std::nullopt);
    const auto pointerCount = OH_ArkUI_PointerEvent_GetPointerCount(event);
    const auto pinch = pinchStateForPoints(touchPairForInputEvent(event));
    return handleTouchAction(state, mapView, action, point, pinch, pointerCount > 1);
}

} // namespace ohos
} // namespace mbgl
