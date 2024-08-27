if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.25")
    add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/unordered_dense SYSTEM)
else()
    add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/unordered_dense)
endif()

set_target_properties(
    unordered_dense
    PROPERTIES
        INTERFACE_MAPBOX_NAME "unordered_dense"
        INTERFACE_MAPBOX_URL "https://github.com/martinus/unordered_dense"
        INTERFACE_MAPBOX_AUTHOR "Martin Leitner-Ankerl"
        INTERFACE_MAPBOX_LICENSE ${PROJECT_SOURCE_DIR}/vendor/unordered_dense/LICENSE
)