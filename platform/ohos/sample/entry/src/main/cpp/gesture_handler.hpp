#pragma once

#include <chrono>
#include <cstdint>
#include <vector>

namespace mbgl {
namespace ohos {

class MapView;

struct GestureState {
    bool touchActive = false;
    int32_t touchId = 0;
    float touchX = 0.0f;
    float touchY = 0.0f;
    bool tapCandidate = false;
    float tapStartX = 0.0f;
    float tapStartY = 0.0f;
    std::chrono::steady_clock::time_point tapStartTime;
    std::chrono::steady_clock::time_point touchSampleTime;
    double touchVelocityX = 0.0;
    double touchVelocityY = 0.0;
    bool lastTapActive = false;
    float lastTapX = 0.0f;
    float lastTapY = 0.0f;
    std::chrono::steady_clock::time_point lastTapTime;
    bool pinchActive = false;
    bool shoveActive = false;
    float pinchStartCenterX = 0.0f;
    float pinchStartCenterY = 0.0f;
    double pinchStartDistance = 0.0;
    double pinchStartAngle = 0.0;
    float pinchCenterX = 0.0f;
    float pinchCenterY = 0.0f;
    double pinchDistance = 0.0;
    double pinchAngle = 0.0;
};

enum class TouchAction {
    Down,
    Move,
    Up,
    Cancel,
    Unknown,
};

struct TouchPoint {
    int32_t id = 0;
    float x = 0.0f;
    float y = 0.0f;
};

struct TouchEvent {
    TouchAction action = TouchAction::Unknown;
    int32_t id = 0;
    float x = 0.0f;
    float y = 0.0f;
    std::vector<TouchPoint> points;
};

bool hasActiveGesture(const GestureState&);
void resetGestureState(GestureState&);
bool handleTouchEvent(GestureState&, MapView*, const TouchEvent&);

} // namespace ohos
} // namespace mbgl
