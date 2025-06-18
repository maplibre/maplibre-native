// Simplified backend selection: we only support Metal in this Qt build
#pragma once

#include <mbgl/gfx/renderable.hpp>
#include "metal_renderer_backend.hpp"

namespace QMapLibre {

// Alias expected by the rest of the Qt wrapper
using RendererBackend = MetalRendererBackend;

} // namespace QMapLibre
