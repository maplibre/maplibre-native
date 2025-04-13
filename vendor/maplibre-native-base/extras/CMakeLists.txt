if(TARGET maplibre-native-base-extras)
    return()
endif()

function(maplibre_native_base_extras_add_library name include_path)
    if(TARGET maplibre-native-base-extras-${name})
        return()
    endif()

    add_library(maplibre-native-base-extras-${name} INTERFACE)
    add_library(MapLibreNative::Base::Extras::${name} ALIAS maplibre-native-base-extras-${name})

    target_include_directories(maplibre-native-base-extras-${name} SYSTEM INTERFACE
        ${include_path}
    )

    target_link_libraries(maplibre-native-base-extras INTERFACE maplibre-native-base-extras-${name})
endfunction()

add_library(maplibre-native-base-extras INTERFACE)
add_library(MapLibreNative::Base::Extras ALIAS maplibre-native-base-extras)

maplibre_native_base_extras_add_library(args ${PROJECT_SOURCE_DIR}/extras/args)
maplibre_native_base_extras_add_library(expected-lite ${PROJECT_SOURCE_DIR}/extras/expected-lite/include)
maplibre_native_base_extras_add_library(filesystem ${PROJECT_SOURCE_DIR}/extras/filesystem/include)
maplibre_native_base_extras_add_library(kdbush.hpp ${PROJECT_SOURCE_DIR}/extras/kdbush.hpp/include)

include(${PROJECT_SOURCE_DIR}/extras/rapidjson.cmake)
