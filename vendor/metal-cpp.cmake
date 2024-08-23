if(TARGET mbgl-vendor-metal-cpp)
    return()
endif()

add_library(
    mbgl-vendor-metal-cpp INTERFACE
)

target_include_directories(
    mbgl-vendor-metal-cpp SYSTEM
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/metal-cpp
)

set_target_properties(
    mbgl-vendor-metal-cpp
    PROPERTIES
        INTERFACE_MAPBOX_NAME "metal-cpp"
        INTERFACE_MAPBOX_URL "https://developer.apple.com/metal/cpp/"
        INTERFACE_MAPBOX_AUTHOR "Apple"
        INTERFACE_MAPBOX_LICENSE ${CMAKE_CURRENT_LIST_DIR}/metal-cpp/LICENSE.txt
)
