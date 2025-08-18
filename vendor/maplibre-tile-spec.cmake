set(MLT_WITH_JSON OFF CACHE BOOL "No JSON support" FORCE)
set(MLT_WITH_PROTOZERO OFF CACHE BOOL "No protozero" FORCE)
set(MLT_WITH_FASTPFOR OFF CACHE BOOL "disabled for lack of support for ARMv7 in SIMDE and requirement for SSE on x86" FORCE)

# This is set by the platform CMakeLists.txt after vendor libraries.  If it was set earlier, we could just use it.
set(CMAKE_OSX_DEPLOYMENT_TARGET 14.3)
set(MLT_OSX_DEPLOYMENT_TARGET ${CMAKE_OSX_DEPLOYMENT_TARGET} CACHE STRING "Inherited MacOS/iOS target" FORCE)

add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/maplibre-tile-spec/cpp SYSTEM)
