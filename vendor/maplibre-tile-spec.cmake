set(MLT_WITH_JSON OFF CACHE BOOL "No JSON support" FORCE)
set(MLT_WITH_PROTOZERO OFF CACHE BOOL "No protozero" FORCE)
set(MLT_WITH_TESTS OFF CACHE BOOL "Google Test conflicts with Dawn")
set(MLT_WITH_TOOLS OFF CACHE BOOL "Not used")
if(MLN_WITH_QT)
    set(MLT_AS_OBJECT_LIBRARY ON CACHE BOOL "Build as object library" FORCE)
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/maplibre-tile-spec/cpp SYSTEM)
