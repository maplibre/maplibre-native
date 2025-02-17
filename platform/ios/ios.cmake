target_include_directories(
    mbgl-core
    PRIVATE
        ${PROJECT_SOURCE_DIR}/platform/ios/src
)

file(GLOB_RECURSE IOS_SDK_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/platform/darwin/src/*.m"
    "${CMAKE_CURRENT_LIST_DIR}/src/*.m"
    "${CMAKE_CURRENT_LIST_DIR}/src/*.mm"
)

if(NOT MLN_WITH_OPENGL)
    list(FILTER IOS_SDK_SOURCE_FILES EXCLUDE REGEX ".*OpenGL.*")
elseif(NOT MLN_WITH_METAL)
    list(FILTER IOS_SDK_SOURCE_FILES EXCLUDE REGEX ".*Metal.*")
endif()

add_library(
    ios-sdk-static
    STATIC ${IOS_SDK_SOURCE_FILES} ${MLN_GENERATED_DARWIN_STYLE_SOURCE}
)

if(MLN_WITH_METAL)
    message(STATUS "Configuring Metal renderer backend")
    target_compile_definitions(
        ios-sdk-static
        PRIVATE MLN_RENDER_BACKEND_METAL=1
    )
endif()

target_include_directories(
    ios-sdk-static
    PUBLIC ${MLN_GENERATED_DARWIN_CODE_DIR} ${CMAKE_CURRENT_LIST_DIR}/src
    PRIVATE
        ${PROJECT_SOURCE_DIR}/platform/darwin/src
)

include("${CMAKE_CURRENT_LIST_DIR}/vendor/calloutview.cmake")

target_link_libraries(
    ios-sdk-static
        PUBLIC mbgl-core
        PRIVATE mbgl-vendor-calloutview mbgl-vendor-metal-cpp
)

add_dependencies(ios-sdk-static mbgl-darwin-style-code)
