// swift-tools-version:5.3
import PackageDescription

let package = Package(
    name: "MapLibre GL Native",
    products: [
        .library(
            name: "Mapbox",
            targets: ["Mapbox"]),
        .library(
            name: "MetalANGLE",
            targets: ["MetalANGLE"])            
    ],
    dependencies: [
    ],    
    targets: [
        .binaryTarget(
            name: "Mapbox",
            url: "MAPBOX_PACKAGE_URL",
            checksum: "MAPBOX_PACKAGE_CHECKSUM"),
        .binaryTarget(
            name: "MetalANGLE",
            url: "METAL_ANGLE_PACKAGE_URL",
            checksum: "METAL_ANGLE_PACKAGE_CHECKSUM")            
    ]
)