#pragma once

#include <mln/style/types.hpp>

#include <cstdint>

namespace mln {

// Runtime form of the style-spec `icon-overlap` / `text-overlap` property.
enum class OverlapMode : std::uint8_t {
    Never,
    Always,
    Cooperative
};

// `Undefined` means the property was not set: fall back to the deprecated
// `-allow-overlap` boolean (false = never, true = always).
inline OverlapMode getOverlapMode(style::IconOverlapType overlap, bool allowOverlap) {
    switch (overlap) {
        case style::IconOverlapType::Never:
            return OverlapMode::Never;
        case style::IconOverlapType::Always:
            return OverlapMode::Always;
        case style::IconOverlapType::Cooperative:
            return OverlapMode::Cooperative;
        case style::IconOverlapType::Undefined:
            break;
    }
    return allowOverlap ? OverlapMode::Always : OverlapMode::Never;
}

} // namespace mln
