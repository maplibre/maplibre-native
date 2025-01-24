if (MLN_USE_TRACY)
    add_definitions(-DTRACY_ENABLE)
    add_definitions(-DMLN_TRACY_ENABLE)
else()
    return()
endif()

include(FetchContent)

FetchContent_Declare(
    tracy
    GIT_REPOSITORY "https://github.com/wolfpld/tracy.git"
    GIT_TAG master
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(tracy)

set_target_properties(
    TracyClient
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "Tracy profiler"
        INTERFACE_MAPLIBRE_URL "https://github.com/wolfpld/tracy.git"
        INTERFACE_MAPLIBRE_AUTHOR "Bartosz Taudul"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/tracy/LICENSE
)
