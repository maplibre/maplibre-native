if(TARGET mbgl-vendor-expected-lite)
	return()
endif()

add_library(
	mbgl-vendor-expected-lite INTERFACE
)

target_include_directories(
	mbgl-vendor-expected-lite SYSTEM
	INTERFACE ${CMAKE_CURRENT_LIST_DIR}/expected-lite/include
)

set_target_properties(
	mbgl-vendor-expected-lite
	PROPERTIES
		INTERFACE_MAPLIBRE_NAME "expected-lite"
		INTERFACE_MAPLIBRE_URL "https://github.com/martinmoene/expected-lite"
		INTERFACE_MAPLIBRE_AUTHOR "Martin Moene"
		INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/expected-lite/LICENSE.txt
)
