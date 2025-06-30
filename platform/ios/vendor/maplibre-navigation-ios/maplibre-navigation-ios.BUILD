load("@rules_apple//apple:resources.bzl", "apple_resource_bundle")
load("@rules_cc//cc:defs.bzl", "objc_library")
load("@rules_swift//swift:swift.bzl", "swift_library")

objc_library(
    name = "MapboxCoreNavigationObjC",
    srcs = glob(["MapboxCoreNavigationObjC/**"]),
    hdrs = glob(["MapboxCoreNavigationObjC/include/**"]),
    module_name = "MapboxCoreNavigationObjC",
)

swift_library(
    name = "MapboxCoreNavigation",
    srcs = glob(["MapboxCoreNavigation/*.swift"]),
    module_name = "MapboxCoreNavigation",
    visibility = ["//visibility:public"],
    #data = glob(["MapboxCoreNavigation/Resources/**"]),
    deps = [
        ":MapboxCoreNavigationObjC",
        "@swiftpkg_mapbox_directions_swift//:MapboxDirections",
        "@swiftpkg_turf_swift//:Turf",
    ],
)

genrule(
    name = "copy_maplibre_header",
    srcs = ["@maplibre-native//platform/ios:ios_public_hdrs"],
    outs = ["MapboxNavigationObjC/include/MapLibre/Mapbox.h"],
    cmd = "cp -r $(SRCS) $(OUTS)",
)

objc_library(
    name = "MapboxNavigationObjC",
    srcs = glob(["MapboxNavigationObjC/**"]),
    hdrs = glob(["MapboxNavigationObjC/include/**"]) + [":copy_maplibre_header"],
    includes = ["./MapboxNavigationObjC/include"],
    module_name = "MapboxNavigationObjC",
    deps = ["@maplibre-native//platform:ios-sdk"],
    #deps = [ "@swiftpkg_maplibre_gl_native_distribution//:MapLibre" ],
)

genrule(
    name = "navigation_info_plist",
    outs = ["MapboxNavigation/Info.plist"],
    cmd = """
    echo `
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleIdentifier</key>
    <string>com.maplibre.mapbox_navigation</string>
    <key>CFBundleShortVersionString</key>
	<string>1.0</string>
	<key>CFBundleVersion</key>
	<string>1</string>
    <key>CFBundleName</key>
    <string>MapboxNavigation</string>
</dict>
</plist>` > $@
    """,
)

apple_resource_bundle(
    name = "MapboxNavigationBundle",
    bundle_id = "com.maplibre.mapbox_navigation",
    bundle_name = "MapboxNavigation",
    infoplists = ["navigation_info_plist"],
    resources = glob(["MapboxNavigation/Resources/**"]),
)

swift_library(
    #name = "MapboxNavigation",
    name = "maplibre-navigation-ios",
    srcs = glob(["MapboxNavigation/*.swift"]),
    module_name = "MapboxNavigation",
    visibility = ["//visibility:public"],
    #data = glob(["MapboxNavigation/Resources/**"]),
    deps = [
        ":MapboxCoreNavigation",
        ":MapboxNavigationBundle",
        ":MapboxNavigationObjC",
        "@swiftpkg_solar//:Solar",
    ],
)
