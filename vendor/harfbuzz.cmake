if(TARGET mbgl-harfbuzz)
    return()
endif()
if (MLN_TEXT_SHAPING_HARFBUZZ)
    if (MLN_WEBGPU_IMPL_FFI)
		add_library(mbgl-harfbuzz SHARED
			${CMAKE_CURRENT_LIST_DIR}/harfbuzz/src/harfbuzz.cc
		)
        # This target is consumed as a shared dependency by other DSOs.
        # Keep symbols visible so runtime resolution (e.g. hb_ft_font_create) succeeds on Android.
        set_target_properties(mbgl-harfbuzz PROPERTIES CXX_VISIBILITY_PRESET default VISIBILITY_INLINES_HIDDEN OFF)
	else()
		add_library(mbgl-harfbuzz STATIC
			${CMAKE_CURRENT_LIST_DIR}/harfbuzz/src/harfbuzz.cc
		)
    endif()

    target_include_directories(mbgl-harfbuzz
        PUBLIC
            ${CMAKE_CURRENT_LIST_DIR}/harfbuzz/src
    )

    target_link_libraries(mbgl-harfbuzz PRIVATE mbgl-freetype)
    target_include_directories(mbgl-harfbuzz PRIVATE ${CMAKE_CURRENT_LIST_DIR}/freetype/include)
    target_compile_definitions(mbgl-harfbuzz PRIVATE -DHAVE_FREETYPE)

    if(APPLE)
        # Keep bundled HarfBuzz symbols private so they do not interpose symbols from
        # app dependencies. See https://github.com/maplibre/maplibre-native/issues/3777.
        target_compile_options(mbgl-harfbuzz PRIVATE -fvisibility=hidden)
    endif()

    set_target_properties(
        mbgl-harfbuzz
        PROPERTIES
            INTERFACE_MAPLIBRE_NAME "harfbuzz"
            INTERFACE_MAPLIBRE_URL "https://github.com/harfbuzz/harfbuzz"
            INTERFACE_MAPLIBRE_AUTHOR ${CMAKE_CURRENT_LIST_DIR}/harfbuzz/AUTHORS
            INTERFACE_MAPLIBRE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/harfbuzz/COPYING
    )
endif()
