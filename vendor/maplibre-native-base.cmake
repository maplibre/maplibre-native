if(NOT TARGET maplibre-native)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/maplibre-native-base)
endif()

set_target_properties(
    maplibre-native-base-shelf-pack-cpp
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "shelf-pack-cpp"
        INTERFACE_MAPLIBRE_URL "https://github.com/mapbox/shelf-pack-cpp"
        INTERFACE_MAPLIBRE_AUTHOR "Mapbox"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/maplibre-native-base/deps/shelf-pack-cpp/LICENSE.md
)

set_target_properties(
    maplibre-native-base-geojson-vt-cpp
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "geojson-vt-cpp"
        INTERFACE_MAPLIBRE_URL "https://github.com/mapbox/geojson-vt-cpp"
        INTERFACE_MAPLIBRE_AUTHOR "Mapbox"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/maplibre-native-base/deps/geojson-vt-cpp/LICENSE
)



set_target_properties(
    maplibre-native-base-geojson.hpp
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "geojson.hpp"
        INTERFACE_MAPLIBRE_URL "https://github.com/mapbox/geojson-cpp"
        INTERFACE_MAPLIBRE_AUTHOR "Mapbox"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/maplibre-native-base/deps/geojson.hpp/LICENSE
)

set_target_properties(
    maplibre-native-base-geometry.hpp
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "geometry.hpp"
        INTERFACE_MAPLIBRE_URL "https://github.com/mapbox/geometry.hpp"
        INTERFACE_MAPLIBRE_AUTHOR "Mapbox"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/maplibre-native-base/deps/geometry.hpp/LICENSE
)

set_target_properties(
    maplibre-native-base
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "mapbox-base"
        INTERFACE_MAPLIBRE_URL "https://github.com/mapbox/mapbox-base"
        INTERFACE_MAPLIBRE_AUTHOR "Mapbox"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/maplibre-native-base/LICENSE
)

set_target_properties(
    maplibre-native-base-variant
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "variant"
        INTERFACE_MAPLIBRE_URL "https://github.com/mapbox/variant"
        INTERFACE_MAPLIBRE_AUTHOR "Mapbox"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/maplibre-native-base/deps/variant/LICENSE
)

set_target_properties(
    maplibre-native-base-cheap-ruler-cpp
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "cheap-ruler-cpp"
        INTERFACE_MAPLIBRE_URL "https://github.com/mapbox/cheap-ruler-cpp"
        INTERFACE_MAPLIBRE_AUTHOR "Mapbox"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/maplibre-native-base/deps/cheap-ruler-cpp/LICENSE
)
