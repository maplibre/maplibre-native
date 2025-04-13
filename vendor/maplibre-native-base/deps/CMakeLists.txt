function(maplibre_native_base_add_library name include_path)
    if(TARGET maplibre-native-base-${name})
        return()
    endif()

    if(NOT EXISTS ${include_path} OR NOT IS_DIRECTORY ${include_path})
        get_filename_component(dir_name ${include_path} NAME)
        message(FATAL_ERROR "Dependency '${dir_name}' not found")
    endif()

    add_library(maplibre-native-base-${name} INTERFACE)
    add_library(MapLibreNative::Base::${name} ALIAS maplibre-native-base-${name})

    target_include_directories(maplibre-native-base-${name} SYSTEM INTERFACE
        ${include_path}
    )

    target_link_libraries(maplibre-native-base INTERFACE maplibre-native-base-${name})
endfunction()

maplibre_native_base_add_library(geojson-vt-cpp ${PROJECT_SOURCE_DIR}/deps/geojson-vt-cpp/include)
maplibre_native_base_add_library(geojson.hpp ${PROJECT_SOURCE_DIR}/deps/geojson.hpp/include)
maplibre_native_base_add_library(geometry.hpp ${PROJECT_SOURCE_DIR}/deps/geometry.hpp/include)
maplibre_native_base_add_library(jni.hpp ${PROJECT_SOURCE_DIR}/deps/jni.hpp/include)
maplibre_native_base_add_library(pixelmatch-cpp ${PROJECT_SOURCE_DIR}/deps/pixelmatch-cpp/include)
maplibre_native_base_add_library(shelf-pack-cpp ${PROJECT_SOURCE_DIR}/deps/shelf-pack-cpp/include)
maplibre_native_base_add_library(supercluster.hpp ${PROJECT_SOURCE_DIR}/deps/supercluster.hpp/include)
maplibre_native_base_add_library(variant ${PROJECT_SOURCE_DIR}/deps/variant/include)
maplibre_native_base_add_library(cheap-ruler-cpp ${PROJECT_SOURCE_DIR}/deps/cheap-ruler-cpp/include)
