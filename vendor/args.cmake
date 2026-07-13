if(TARGET mbgl-vendor-args)
    return()
endif()

add_library(
    mbgl-vendor-args INTERFACE
)

target_include_directories(
    mbgl-vendor-args SYSTEM
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/args
)

set_target_properties(
    mbgl-vendor-args
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "args"
        INTERFACE_MAPLIBRE_URL "https://github.com/Taywee/args"
        INTERFACE_MAPLIBRE_AUTHOR "Taylor C. Richberger"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/args/LICENSE
)
