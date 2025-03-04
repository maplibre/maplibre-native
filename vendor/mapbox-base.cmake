# License helper for mapbox-base, should be upstreamed.

if(NOT TARGET mapbox-base)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/mapbox-base)
endif()

set_target_properties(
    mapbox-base-extras-kdbush.hpp
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "kdbush.hpp"
        INTERFACE_MAPLIBRE_URL "https://github.com/mourner/kdbush.hpp"
        INTERFACE_MAPLIBRE_AUTHOR "Vladimir Agafonkin"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/mapbox-base/extras/kdbush.hpp/LICENSE
)

set_target_properties(
    mapbox-base-extras-expected-lite
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "expected-lite"
        INTERFACE_MAPLIBRE_URL "https://github.com/martinmoene/expected-lite"
        INTERFACE_MAPLIBRE_AUTHOR "Martin Moene"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/mapbox-base/extras/expected-lite/LICENSE.txt
)

set_target_properties(
    mapbox-base-supercluster.hpp
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "supercluster.hpp"
        INTERFACE_MAPLIBRE_URL "https://github.com/mapbox/supercluster.hpp"
        INTERFACE_MAPLIBRE_AUTHOR "Mapbox"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/mapbox-base/deps/supercluster.hpp/LICENSE
)

set_target_properties(
    mapbox-base-shelf-pack-cpp
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "shelf-pack-cpp"
        INTERFACE_MAPLIBRE_URL "https://github.com/mapbox/shelf-pack-cpp"
        INTERFACE_MAPLIBRE_AUTHOR "Mapbox"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/mapbox-base/deps/shelf-pack-cpp/LICENSE.md
)

set_target_properties(
    mapbox-base-geojson-vt-cpp
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "geojson-vt-cpp"
        INTERFACE_MAPLIBRE_URL "https://github.com/mapbox/geojson-vt-cpp"
        INTERFACE_MAPLIBRE_AUTHOR "Mapbox"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/mapbox-base/deps/geojson-vt-cpp/LICENSE
)

set_target_properties(
    mapbox-base-extras-rapidjson
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "RapidJSON"
        INTERFACE_MAPLIBRE_URL "https://rapidjson.org"
        INTERFACE_MAPLIBRE_AUTHOR "THL A29 Limited, a Tencent company, and Milo Yip"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/mapbox-base/extras/rapidjson/license.txt
)

set_target_properties(
    mapbox-base-geojson.hpp
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "geojson.hpp"
        INTERFACE_MAPLIBRE_URL "https://github.com/mapbox/geojson-cpp"
        INTERFACE_MAPLIBRE_AUTHOR "Mapbox"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/mapbox-base/deps/geojson.hpp/LICENSE
)

set_target_properties(
    mapbox-base-geometry.hpp
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "geometry.hpp"
        INTERFACE_MAPLIBRE_URL "https://github.com/mapbox/geometry.hpp"
        INTERFACE_MAPLIBRE_AUTHOR "Mapbox"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/mapbox-base/deps/geometry.hpp/LICENSE
)

set_target_properties(
    mapbox-base
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "mapbox-base"
        INTERFACE_MAPLIBRE_URL "https://github.com/mapbox/mapbox-base"
        INTERFACE_MAPLIBRE_AUTHOR "Mapbox"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/mapbox-base/LICENSE
)

set_target_properties(
    mapbox-base-variant
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "variant"
        INTERFACE_MAPLIBRE_URL "https://github.com/mapbox/variant"
        INTERFACE_MAPLIBRE_AUTHOR "Mapbox"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/mapbox-base/deps/variant/LICENSE
)

set_target_properties(
    mapbox-base-cheap-ruler-cpp
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "cheap-ruler-cpp"
        INTERFACE_MAPLIBRE_URL "https://github.com/mapbox/cheap-ruler-cpp"
        INTERFACE_MAPLIBRE_AUTHOR "Mapbox"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/mapbox-base/deps/cheap-ruler-cpp/LICENSE
)
