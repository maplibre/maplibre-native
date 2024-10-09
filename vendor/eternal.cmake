if(TARGET mbgl-vendor-eternal)
    return()
endif()

add_library(
    mbgl-vendor-eternal INTERFACE
)

target_include_directories(
    mbgl-vendor-eternal SYSTEM
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/eternal/include
)

set_target_properties(
    mbgl-vendor-eternal
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "eternal"
        INTERFACE_MAPLIBRE_URL "https://github.com/mapbox/eternal"
        INTERFACE_MAPLIBRE_AUTHOR "Mapbox"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/eternal/LICENSE.md
)
