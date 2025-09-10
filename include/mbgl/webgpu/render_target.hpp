#pragma once

#include <mbgl/renderer/render_target.hpp>

namespace mbgl {
namespace webgpu {

// RenderTarget is actually defined in mbgl::RenderTarget
// This file just ensures the type is known in the webgpu namespace
using RenderTarget = mbgl::RenderTarget;

} // namespace webgpu
} // namespace mbgl