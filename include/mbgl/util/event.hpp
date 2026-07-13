#pragma once

#include <cstdint>

namespace mbgl {

enum class EventSeverity : uint8_t {
    Debug,
    Info,
    Warning,
    Error,
    SeverityCount
};

enum class Event : uint8_t {
    General,
    Setup,
    Shader,
    ParseStyle,
    ParseTile,
    Render,
    Style,
    Database,
    HttpRequest,
    Sprite,
    Image,
    OpenGL,
    JNI,
    Android,
    Crash,
    Glyph,
    Timing
};

struct EventPermutation {
    const EventSeverity severity;
    const Event event;

    constexpr bool operator==(const EventPermutation &rhs) const {
        return severity == rhs.severity && event == rhs.event;
    }
};

constexpr EventSeverity disabledEventSeverities[] = {
#ifndef NDEBUG
    EventSeverity(-1) // Avoid zero size array
#else
    EventSeverity::Debug
#endif
};

constexpr Event disabledEvents[] = {
    Event(-1) // Avoid zero size array
};

constexpr EventPermutation disabledEventPermutations[] = {{.severity = EventSeverity::Debug, .event = Event::Shader}};

} // namespace mbgl
