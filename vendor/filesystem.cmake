if(TARGET mbgl-vendor-filesystem)
	return()
endif()

add_library(
	mbgl-vendor-filesystem INTERFACE
)

target_include_directories(
	mbgl-vendor-filesystem SYSTEM
	INTERFACE ${CMAKE_CURRENT_LIST_DIR}/filesystem/include
)

set_target_properties(
	mbgl-vendor-filesystem
	PROPERTIES
		INTERFACE_MAPLIBRE_NAME "filesystem"
		INTERFACE_MAPLIBRE_URL "https://github.com/gulrak/filesystem"
		INTERFACE_MAPLIBRE_AUTHOR "Steffen Schümann"
		INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/filesystem/LICENSE
)
