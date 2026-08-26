#pragma once

#include <cstdint>

namespace mln {
namespace gfx {

/// Which projection prelude a shader is compiled against.
enum class ProjectionVariant : uint8_t {
    Mercator,
    Globe,
};

} // namespace gfx
} // namespace mln
