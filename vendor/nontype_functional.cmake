if(TARGET mbgl-vendor-nontype_functional)
    return()
endif()

add_library(
    mbgl-vendor-nontype_functional INTERFACE
)

target_include_directories(
    mbgl-vendor-nontype_functional SYSTEM
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/nontype_functional/include
)

set_target_properties(
    mbgl-vendor-nontype_functional
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "nontype_functional"
        INTERFACE_MAPLIBRE_URL "https://github.com/zhihaoy/nontype_functional"
        INTERFACE_MAPLIBRE_AUTHOR "zhihaoy"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/nontype_functional/LICENSE
)
