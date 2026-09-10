// swift-tools-version: 5.9
import PackageDescription

// The plugin contract only. Applications must also link the matching MapLibre SDK.
let package = Package(
    name: "MapLibrePluginApi",
    products: [.library(name: "MapLibrePluginApi", targets: ["MapLibrePluginApi"])],
    targets: [
        .target(
            name: "MapLibrePluginApi",
            path: "include",
            sources: ["plugin_api_anchor.c"],
            publicHeadersPath: "."
        ),
    ]
)
