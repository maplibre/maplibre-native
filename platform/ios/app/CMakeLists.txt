cmake_minimum_required(VERSION 3.20)
project(app LANGUAGES C CXX OBJC OBJCXX)

set(CMAKE_OSX_ARCHITECTURES "arm64")
set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0")

file(GLOB APP_SOURCES CONFIGURE_DEPENDS
    "*.m" "*.mm"
)

set(APP_RESOURCES
    Assets.xcassets
    Base.lproj
    LaunchScreen.storyboard
    Main.storyboard
    Settings.bundle
)

add_executable(app
    ${APP_SOURCES}
    ${APP_RESOURCES}
)

target_include_directories(
  app PRIVATE
  "${PROJECT_SOURCE_DIR}"
)

set_target_properties(app PROPERTIES
    MACOSX_BUNDLE YES
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/Info.plist"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "org.maplibre.app"
    XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "YES"
    XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${DEVELOPMENT_TEAM_ID}"
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES"
)

add_custom_command(TARGET app POST_BUILD
    COMMAND plutil -insert CFBundleShortVersionString -string "1.0.0" "$<TARGET_BUNDLE_DIR:app>/Info.plist"
    COMMENT "Injecting CFBundleShortVersionString..."
)

add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "$<TARGET_BUNDLE_DIR:ios-sdk-static>/Mapbox.bundle"
        "$<TARGET_BUNDLE_DIR:app>/Mapbox.bundle"
    COMMAND ls "$<TARGET_BUNDLE_DIR:app>"
)

target_link_libraries(
  app
  custom-layer-examples  # ios-sdk-static transitive dependency
)

foreach(resource_file ${APP_RESOURCES})
    set_source_files_properties(
        ${resource_file}
        PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
    )
endforeach()
