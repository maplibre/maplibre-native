#pragma once
#include <cstdint>

namespace mbgl {
enum class TileOperation : uint8_t {
    RequestedFromCache,
    RequestedFromNetwork,
    LoadFromNetwork,
    LoadFromCache,
    StartParse,
    EndParse,
    Error,
    Cancelled,
    NullOp,
};
} // namespace mbgl
