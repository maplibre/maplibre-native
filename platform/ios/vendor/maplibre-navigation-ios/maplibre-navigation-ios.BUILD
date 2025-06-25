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

swift_library(
    #name = "MapboxNavigation",
    name = "maplibre-navigation-ios",
    srcs = glob(["MapboxNavigation/*.swift"]),
    module_name = "MapboxNavigation",
    visibility = ["//visibility:public"],
    #data = glob(["MapboxNavigation/Resources/**"]),
    deps = [
        ":MapboxCoreNavigation",
        ":MapboxNavigationObjC",
        "@swiftpkg_solar//:Solar",
    ],
)
