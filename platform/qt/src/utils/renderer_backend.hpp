// Simplified backend selection: we only support Metal in this Qt build
#pragma once

#include <mbgl/gfx/renderable.hpp>

#if defined(MLN_RENDER_BACKEND_OPENGL)
#include "opengl_renderer_backend.hpp"
#elif defined(MLN_RENDER_BACKEND_VULKAN)
#include "vulkan_renderer_backend.hpp"
#elif defined(MLN_RENDER_BACKEND_METAL)
#include "metal_renderer_backend.hpp"
#endif

namespace QMapLibre {

#if defined(MLN_RENDER_BACKEND_OPENGL)
using RendererBackend = OpenGLRendererBackend;
#elif defined(MLN_RENDER_BACKEND_VULKAN)
using RendererBackend = VulkanRendererBackend;
#elif defined(MLN_RENDER_BACKEND_METAL)
using RendererBackend = MetalRendererBackend;
#endif

} // namespace QMapLibre
