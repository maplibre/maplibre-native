function(maplibre_native_base_add_library name include_path)
    if(TARGET maplibre-native-base-${name})
        return()
    endif()

    if(NOT EXISTS ${include_path} OR NOT IS_DIRECTORY ${include_path})
        message(FATAL_ERROR "Dependency with include path '${include_path}' not found")
    endif()

    add_library(maplibre-native-base-${name} INTERFACE)
    add_library(MapLibreNative::Base::${name} ALIAS maplibre-native-base-${name})

    target_include_directories(maplibre-native-base-${name} SYSTEM INTERFACE
        ${include_path}
    )

    target_link_libraries(maplibre-native-base INTERFACE maplibre-native-base-${name})
endfunction()

maplibre_native_base_add_library(cheap-ruler-cpp ${CMAKE_CURRENT_LIST_DIR}/cheap-ruler-cpp/include)
maplibre_native_base_add_library(geojson-vt-cpp ${CMAKE_CURRENT_LIST_DIR}/geojson-vt-cpp/include)
maplibre_native_base_add_library(geojson.hpp ${CMAKE_CURRENT_LIST_DIR}/geojson.hpp/include)
maplibre_native_base_add_library(geometry.hpp ${CMAKE_CURRENT_LIST_DIR}/geometry.hpp/include)
maplibre_native_base_add_library(jni.hpp ${CMAKE_CURRENT_LIST_DIR}/jni.hpp/include)
maplibre_native_base_add_library(pixelmatch-cpp ${CMAKE_CURRENT_LIST_DIR}/pixelmatch-cpp/include)
maplibre_native_base_add_library(shelf-pack-cpp ${CMAKE_CURRENT_LIST_DIR}/shelf-pack-cpp/include)
maplibre_native_base_add_library(variant ${CMAKE_CURRENT_LIST_DIR}/variant/include)
