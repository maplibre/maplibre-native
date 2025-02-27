set(MLT_WITH_JSON OFF CACHE BOOL "No JSON support" FORCE)
set(MLT_WITH_PROTOZERO OFF CACHE BOOL "No protozero" FORCE)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.25")
    add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/maplibre-tile-spec/cpp SYSTEM)
else()
    add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/maplibre-tile-spec/cpp)
endif()
