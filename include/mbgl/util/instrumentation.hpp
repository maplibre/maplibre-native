#pragma once

#ifdef MLN_TRACY_ENABLE

#include <tracy/Tracy.hpp>
#include <cstddef>

template <typename GpuId>
const void* castGpuIdToTracyPtr(GpuId id) {
    // Tracy functions such as TracyAllocN track resources using `const void*`
    // pointers. GPU resources may use a non-pointer handles. These handles may
    // be GLsizei, GLuint and Vulkan handles are guaranteed to be 64 bits:
    // Check VK_USE_64_BIT_PTR_DEFINES in the spec for instance.
    // For simplicity we cast to ptrdiff_t which is guaranteed to work on
    // 64bits systems but the cast may drop high bits if the handle is larger
    // than the pointer type
    // We fail to compile if this happens. A workaround would be to map
    // handles to 32 bits values if Tracy is on those systems
    static_assert(sizeof(GpuId) <= sizeof(std::ptrdiff_t), "Tracy is currently not supported on 32 bits systems");
    return reinterpret_cast<const void*>(static_cast<std::ptrdiff_t>(id));
}

#if !defined(MLN_RENDER_BACKEND_OPENGL) && !defined(MLN_RENDER_BACKEND_VULKAN)
#error \
    "MLN_RENDER_BACKEND_OPENGL/MLN_RENDER_BACKEND_VULKAN is not defined. \
    MLN_RENDER_BACKEND_OPENGL/MLN_RENDER_BACKEND_VULKAN is expected to be defined in CMake and Bazel"
#endif

#define MLN_TRACE_FUNC() ZoneScoped
#define MLN_TRACE_ZONE(label) ZoneScopedN(#label)

#define MLN_ZONE_TEXT(text, size) ZoneText(text, size)
#define MLN_ZONE_STR(str) ZoneText(str.c_str(), str.size())
#define MLN_ZONE_VALUE(n) ZoneValue(n)

constexpr const char* tracyTextureMemoryLabel = "Texture Memory";
#define MLN_TRACE_ALLOC_TEXTURE(id, size) TracyAllocN(castGpuIdToTracyPtr(id), size, tracyTextureMemoryLabel)
#define MLN_TRACE_FREE_TEXTURE(id) TracyFreeN(castGpuIdToTracyPtr(id), tracyTextureMemoryLabel)

constexpr const char* tracyRenderTargetMemoryLabel = "Render Target Memory";
#define MLN_TRACE_ALLOC_RT(id, size) TracyAllocN(castGpuIdToTracyPtr(id), size, tracyRenderTargetMemoryLabel)
#define MLN_TRACE_FREE_RT(id) TracyFreeN(castGpuIdToTracyPtr(id), tracyRenderTargetMemoryLabel)

constexpr const char* tracyVertexMemoryLabel = "Vertex Buffer Memory";
#define MLN_TRACE_ALLOC_VERTEX_BUFFER(id, size) TracyAllocN(castGpuIdToTracyPtr(id), size, tracyVertexMemoryLabel)
#define MLN_TRACE_FREE_VERTEX_BUFFER(id) TracyFreeN(castGpuIdToTracyPtr(id), tracyVertexMemoryLabel)

constexpr const char* tracyIndexMemoryLabel = "Index Buffer Memory";
#define MLN_TRACE_ALLOC_INDEX_BUFFER(id, size) TracyAllocN(castGpuIdToTracyPtr(id), size, tracyIndexMemoryLabel)
#define MLN_TRACE_FREE_INDEX_BUFFER(id) TracyFreeN(castGpuIdToTracyPtr(id), tracyIndexMemoryLabel)

constexpr const char* tracyConstMemoryLabel = "Constant Buffer Memory";
#define MLN_TRACE_ALLOC_CONST_BUFFER(id, size) TracyAllocN(castGpuIdToTracyPtr(id), size, tracyConstMemoryLabel)
#define MLN_TRACE_FREE_CONST_BUFFER(id) TracyFreeN(castGpuIdToTracyPtr(id), tracyConstMemoryLabel)

// Only OpenGL is currently considered for GPU profiling
// Metal and other APIs need to be handled separately
#if MLN_RENDER_BACKEND_OPENGL

#include <mbgl/gl/timestamp_query_extension.hpp>

// TracyOpenGL.hpp assumes OpenGL functions are in the global namespace
// Temporarily expose the functions to TracyOpenGL.hpp then undef
#define glGenQueries mbgl::gl::extension::glGenQueries
#define glGetQueryiv mbgl::gl::extension::glGetQueryiv
#define glGetQueryObjectiv mbgl::gl::extension::glGetQueryObjectiv
#define glGetInteger64v mbgl::gl::extension::glGetInteger64v
#define glQueryCounter mbgl::gl::extension::glQueryCounter
#define glGetQueryObjectui64v mbgl::gl::extension::glGetQueryObjectui64v
#define GLint mbgl::platform::GLint

#include "tracy/TracyOpenGL.hpp"

#define MLN_TRACE_GL_CONTEXT() TracyGpuContext
#define MLN_TRACE_GL_ZONE(label) TracyGpuZone(#label)
#define MLN_TRACE_FUNC_GL() TracyGpuZone(__FUNCTION__)

#define MLN_END_FRAME()  \
    do {                 \
        FrameMark;       \
        TracyGpuCollect; \
    } while (0);

#undef glGenQueries
#undef glGetQueryiv
#undef glGetQueryObjectiv
#undef glGetInteger64v
#undef glQueryCounter
#undef glGetQueryObjectui64v
#undef GLint

#elif MLN_RENDER_BACKEND_VULKAN

#define MLN_END_FRAME() \
    do {                \
        FrameMark;      \
    } while (0);

#else

#define MLN_TRACE_GL_CONTEXT() ((void)0)
#define MLN_TRACE_GL_ZONE(label) ((void)0)
#define MLN_TRACE_FUNC_GL() ((void)0)
#define MLN_END_FRAME() FrameMark

#endif

#else // MLN_TRACY_ENABLE

#define MLN_TRACE_GL_CONTEXT() ((void)0)
#define MLN_TRACE_GL_ZONE(label) ((void)0)
#define MLN_ZONE_TEXT(label) ((void)0)
#define MLN_ZONE_STR(str) ((void)0)
#define MLN_ZONE_VALUE(val) ((void)0)
#define MLN_TRACE_FUNC_GL() ((void)0)
#define MLN_END_FRAME() ((void)0)
#define MLN_TRACE_ALLOC_TEXTURE(id, size) ((void)0)
#define MLN_TRACE_FREE_TEXTURE(id) ((void)0)
#define MLN_TRACE_ALLOC_RT(id, size) ((void)0)
#define MLN_TRACE_FREE_RT(id) ((void)0)
#define MLN_TRACE_ALLOC_VERTEX_BUFFER(id, size) ((void)0)
#define MLN_TRACE_FREE_VERTEX_BUFFER(id) ((void)0)
#define MLN_TRACE_ALLOC_INDEX_BUFFER(id, size) ((void)0)
#define MLN_TRACE_FREE_INDEX_BUFFER(id) ((void)0)
#define MLN_TRACE_ALLOC_CONST_BUFFER(id, size) ((void)0)
#define MLN_TRACE_FREE_CONST_BUFFER(id) ((void)0)
#define MLN_TRACE_FUNC() ((void)0)
#define MLN_TRACE_ZONE(label) ((void)0)

#endif // MLN_TRACY_ENABLE
