// swift-tools-version: 5.9

import PackageDescription

// dummy package to fetch dependencies via bazel `rules_swift_package_manager``

let package = Package(
    name: "maplibre-swift",
    defaultLocalization: "en",
    platforms: [.iOS(.v12)],
    dependencies: [
        .package(url: "https://github.com/flitsmeister/mapbox-directions-swift", exact: "0.23.3"),
        .package(url: "https://github.com/mapbox/turf-swift.git", from: "2.8.0"),
        .package(url: "https://github.com/ceeK/Solar.git", exact: "3.0.1"),
        .package(url: "https://github.com/nicklockwood/SwiftFormat.git", from: "0.53.6"),
        // .package(url: "https://github.com/maplibre/maplibre-gl-native-distribution.git", from: "6.0.0"),
    ]
)
