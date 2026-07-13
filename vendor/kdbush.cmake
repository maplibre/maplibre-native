if(TARGET mbgl-vendor-kdbush)
    return()
endif()

add_library(
    mbgl-vendor-kdbush INTERFACE
)

target_include_directories(
    mbgl-vendor-kdbush SYSTEM
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/kdbush.hpp/include
)

set_target_properties(
    mbgl-vendor-kdbush
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "kdbush.hpp"
        INTERFACE_MAPLIBRE_URL "https://github.com/mourner/kdbush.hpp"
        INTERFACE_MAPLIBRE_AUTHOR "Vladimir Agafonkin"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/kdbush.hpp/LICENSE
)
