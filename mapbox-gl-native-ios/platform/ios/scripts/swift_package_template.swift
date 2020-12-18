// swift-tools-version:5.3
import PackageDescription

let package = Package(
    name: "maplibre-gl-native",
    products: [
        .library(
            name: "Mapbox",
            targets: ["Mapbox"]),
        .library(
            name: "MapboxMobileEvents",
            targets: ["MapboxMobileEvents"])            
    ],
    dependencies: [
    ],    
    targets: [
        .binaryTarget(
            name: "Mapbox",
            url: "MAPBOX_PACKAGE_URL",
            checksum: "MAPBOX_PACKAGE_CHECKSUM"),
        .binaryTarget(
            name: "MapboxMobileEvents",
            url: "MAPBOX_EVENTS_PACKAGE_URL",
            checksum: "MAPBOX_EVENTS_PACKAGE_CHECKSUM")            
    ]
)