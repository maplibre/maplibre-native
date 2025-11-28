// swift-tools-version: 5.9

import PackageDescription

// dummy package to fetch dependencies via bazel `rules_swift_package_manager``
// https://github.com/cgrindel/rules_swift_package_manager?tab=readme-ov-file#4-run-swift-package-update

let package = Package(
    name: "maplibre-swift",
    defaultLocalization: "en",
    platforms: [.iOS(.v12)],
    dependencies: [
        .package(url: "https://github.com/raphaelmor/Polyline", from: "5.1.0"),
        .package(url: "https://github.com/getsentry/sentry-cocoa", from: "8.54.0"),
    ]
)
