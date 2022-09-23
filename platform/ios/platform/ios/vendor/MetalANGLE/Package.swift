// swift-tools-version:5.6
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "MetalANGLE",
    platforms: [.iOS(.v10)],
    products: [
        .library(name: "MetalANGLE", targets: ["MetalANGLE"]),
    ],
    targets: [
        .binaryTarget(
            name: "MetalANGLE",
            url: "https://github.com/username0x0a/metalangle/releases/download/gles3-0.0.7/MetalANGLE.zip",
            checksum: "848d6ebde5d474786cf82569a466c62c60f67c142f5e1119c51032ff55d9b2bb"
        ),
    ]
)
