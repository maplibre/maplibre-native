if(TARGET mbgl-vendor-rapidjson)
    return()
endif()

add_library(mbgl-vendor-rapidjson INTERFACE)

target_compile_definitions(mbgl-vendor-rapidjson INTERFACE
    RAPIDJSON_HAS_STDSTRING=1
)

if(WIN32)
    target_compile_definitions(mbgl-vendor-rapidjson INTERFACE
        RAPIDJSON_HAS_CXX11_RVALUE_REFS
    )
endif()

target_include_directories(
    mbgl-vendor-rapidjson SYSTEM
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/rapidjson/include
)

set_target_properties(
    mbgl-vendor-rapidjson
    PROPERTIES
        INTERFACE_MAPLIBRE_NAME "RapidJSON"
        INTERFACE_MAPLIBRE_URL "https://rapidjson.org"
        INTERFACE_MAPLIBRE_AUTHOR "THL A29 Limited, a Tencent company, and Milo Yip"
        INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/rapidjson/license.txt
)
