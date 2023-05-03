if(TARGET mbgl-vendor-nunicode)
    return()
endif()

if(MLN_WITH_QT AND ${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12.0")
    add_library(mbgl-vendor-nunicode OBJECT)
else()
    add_library(mbgl-vendor-nunicode STATIC)
endif()

target_sources(
    mbgl-vendor-nunicode PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/nunicode/src/libnu/ducet.c
    ${CMAKE_CURRENT_LIST_DIR}/nunicode/src/libnu/strcoll.c
    ${CMAKE_CURRENT_LIST_DIR}/nunicode/src/libnu/strings.c
    ${CMAKE_CURRENT_LIST_DIR}/nunicode/src/libnu/tolower.c
    ${CMAKE_CURRENT_LIST_DIR}/nunicode/src/libnu/tounaccent.c
    ${CMAKE_CURRENT_LIST_DIR}/nunicode/src/libnu/toupper.c
    ${CMAKE_CURRENT_LIST_DIR}/nunicode/src/libnu/utf8.c
)

target_compile_definitions(
    mbgl-vendor-nunicode
    PRIVATE NU_BUILD_STATIC
)

target_link_libraries(
    mbgl-vendor-nunicode
    PRIVATE mbgl-compiler-options
)

if(MSVC)
    target_compile_options(mbgl-vendor-nunicode PRIVATE /wd4146)
else()
    target_compile_options(mbgl-vendor-nunicode PRIVATE -Wno-error)
endif()

target_include_directories(
    mbgl-vendor-nunicode SYSTEM
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/nunicode/include
)

export(TARGETS
    mbgl-vendor-nunicode
    APPEND FILE MapboxCoreTargets.cmake
)
