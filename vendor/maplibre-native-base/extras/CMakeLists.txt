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

maplibre_native_base_extras_add_library(expected-lite ${CMAKE_CURRENT_LIST_DIR}/expected-lite/include)
maplibre_native_base_extras_add_library(kdbush.hpp ${CMAKE_CURRENT_LIST_DIR}/kdbush.hpp/include)
