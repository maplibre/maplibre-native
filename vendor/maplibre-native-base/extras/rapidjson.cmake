maplibre_native_base_extras_add_library(rapidjson ${CMAKE_CURRENT_LIST_DIR}/rapidjson/include)

target_compile_definitions(maplibre-native-base-extras-rapidjson INTERFACE
    RAPIDJSON_HAS_STDSTRING=1
)

if(WIN32)
    target_compile_definitions(maplibre-native-base-extras-rapidjson INTERFACE
        RAPIDJSON_HAS_CXX11_RVALUE_REFS
    )
endif()
