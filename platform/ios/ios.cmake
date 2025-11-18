target_include_directories(
    mbgl-core
    PRIVATE
        ${PROJECT_SOURCE_DIR}/platform/ios/src
)

target_link_libraries(
    mbgl-core
    PRIVATE
        mbgl-vendor-filesystem
)

file(GLOB_RECURSE IOS_SDK_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/platform/darwin/src/*.m"
    "${PROJECT_SOURCE_DIR}/platform/darwin/src/*.mm"
    "${CMAKE_CURRENT_LIST_DIR}/src/*.m"
    "${CMAKE_CURRENT_LIST_DIR}/src/*.mm"
)

file(GLOB_RECURSE IOS_SDK_RESOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/resources/*"
)


if(NOT MLN_WITH_OPENGL)
    list(FILTER IOS_SDK_SOURCE_FILES EXCLUDE REGEX ".*(OpenGL|gl).*")
elseif(NOT MLN_WITH_METAL)
    list(FILTER IOS_SDK_SOURCE_FILES EXCLUDE REGEX ".*Metal.*")
endif()

set(FRAMEWORK_BUNDLE_DIR ${CMAKE_CURRENT_BINARY_DIR}/Mapbox.bundle)

find_program(ACTOOL_EXECUTABLE actool
    HINTS
        /Applications/Xcode.app/Contents/Developer/usr/bin
    DOC "Xcode Asset Catalog Compiler (actool)"
)
if(NOT ACTOOL_EXECUTABLE)
    message(FATAL_ERROR "Could not find actool. Ensure Xcode is installed.")
endif()

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Mapbox.bundle/Assets.car
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/Mapbox.bundle
    COMMAND ${ACTOOL_EXECUTABLE}
            --compile ${FRAMEWORK_BUNDLE_DIR}
            --platform iphoneos
            --minimum-deployment-target 12.0
            ${CMAKE_CURRENT_LIST_DIR}/resources/Images.xcassets
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_LIST_DIR}/framework/Info-static.plist
            ${CMAKE_CURRENT_BINARY_DIR}/Mapbox.bundle/Info.plist
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/resources/Images.xcassets
    COMMENT "Compiling Images.xcassets into Mapbox.bundle/Assets.car"
)

list(APPEND IOS_SDK_RESOURCE_FILES ${FRAMEWORK_BUNDLE_DIR})

add_custom_target(create-framework-bundle
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Mapbox.bundle/Assets.car
)

add_library(
    ios-sdk-static
    STATIC ${IOS_SDK_SOURCE_FILES} ${MLN_GENERATED_DARWIN_STYLE_SOURCE}
    ${IOS_SDK_RESOURCE_FILES}
)

set_source_files_properties(
    ${FRAMEWORK_BUNDLE_DIR}
    PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
    GENERATED TRUE
)

add_dependencies(ios-sdk-static create-framework-bundle)

set_target_properties(ios-sdk-static PROPERTIES
    FRAMEWORK TRUE
    # This makes the .framework contain a static library instead of a dynamic one:
    XCODE_ATTRIBUTE_MACH_O_TYPE "staticlib"

    # Provide a unique bundle identifier
    MACOSX_FRAMEWORK_IDENTIFIER "org.maplibre.ios"

    # FIXME: versioning
    MACOSX_FRAMEWORK_BUNDLE_VERSION "1.0"
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING "1.0"
)

if(MLN_WITH_METAL)
    message(STATUS "Configuring Metal renderer backend")
endif()

target_include_directories(
    ios-sdk-static
    PUBLIC ${MLN_GENERATED_DARWIN_CODE_DIR} ${CMAKE_CURRENT_LIST_DIR}/src ${PROJECT_SOURCE_DIR}/platform/darwin/src
    PRIVATE ${PROJECT_SOURCE_DIR}/src
)

include("${CMAKE_CURRENT_LIST_DIR}/vendor/calloutview.cmake")

target_link_libraries(
    ios-sdk-static
        PUBLIC mbgl-core
        PRIVATE mbgl-compiler-options mbgl-vendor-calloutview mbgl-vendor-metal-cpp mbgl-vendor-polylabel
        "-framework CoreText"
        "-framework CoreImage"
        "-framework CoreGraphics"
        "-framework QuartzCore"
        "-framework UIKit"
        "-framework MetalKit"
        "-framework ImageIO"
)

target_link_options(ios-sdk-static INTERFACE -ObjC)

add_dependencies(ios-sdk-static mbgl-darwin-style-code)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/app")

set(DEVELOPMENT_TEAM_ID "" CACHE STRING "Apple Development Team ID")

if(NOT DEVELOPMENT_TEAM_ID)
    message(STATUS "No Apple Developer Team ID set (-DDEVELOPMENT_TEAM_ID=YOUR_TEAM_ID). Configuring build without signing.")
    set(CMAKE_OSX_SYSROOT iphonesimulator)
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO")
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO")
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")
endif()
