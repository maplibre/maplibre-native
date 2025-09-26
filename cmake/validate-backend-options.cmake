if (MLN_LEGACY_RENDERER)
  message(FATAL "The legacy renderer is no longer supported")
endif()

if (MLN_DRAWABLE_RENDERER)
  message(FATAL "Do not pass MLN_DRAWABLE_RENDERER, the drawable renderer is now the default")
endif()

set(backend_count 0)
if (MLN_WITH_OPENGL)
  math(EXPR backend_count "${backend_count} + 1")
endif()
if (MLN_WITH_METAL)
  math(EXPR backend_count "${backend_count} + 1")
endif()
if (MLN_WITH_VULKAN)
  math(EXPR backend_count "${backend_count} + 1")
endif()
if (MLN_WITH_WEBGPU)
  math(EXPR backend_count "${backend_count} + 1")
endif()

if (backend_count EQUAL 0)
  message(FATAL_ERROR
    "You need to set a rendering backend. "
    "Set exactly one of: MLN_WITH_OPENGL, MLN_WITH_METAL, MLN_WITH_VULKAN, or MLN_WITH_WEBGPU.")
elseif (backend_count GREATER 1)
  message(FATAL_ERROR
    "Multiple rendering backends selected. "
    "Please enable only one of: MLN_WITH_OPENGL, MLN_WITH_METAL, MLN_WITH_VULKAN, or MLN_WITH_WEBGPU.")
endif()
