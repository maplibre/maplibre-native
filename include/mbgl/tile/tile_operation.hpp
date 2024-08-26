#pragma once
#include <cstdint>

namespace mbgl {
enum class TileOperation : uint8_t {
    Requested,
    LoadFromNetwork,
    LoadFromCache,
    StartParse,
    EndParse,
    Error,
    Cancelled,
};
} // namespace mbgl
