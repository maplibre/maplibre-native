if(TARGET mbgl-vendor-supercluster.hpp)
    return()
endif()

add_library(mbgl-vendor-supercluster.hpp INTERFACE)

target_include_directories(
    mbgl-vendor-supercluster.hpp SYSTEM
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/supercluster/include
)

set_target_properties(
    mbgl-vendor-supercluster.hpp
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "supercluster.hpp"
        INTERFACE_MAPLIBRE_URL "https://github.com/mapbox/supercluster.hpp"
        INTERFACE_MAPLIBRE_AUTHOR "Mapbox"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/supercluster/LICENSE
)
