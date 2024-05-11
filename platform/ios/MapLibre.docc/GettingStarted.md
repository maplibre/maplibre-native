# Getting Started

Setting up an Xcode project that uses MapLibre Native for iOS.

## Create a new iOS project

Create a new (SwiftUI) iOS project with Xcode. Go to *File > New > Project...*.

## Add MapLibre Native as a dependency

MapLibre Native for iOS is available on [Cocoapods](https://cocoapods.org) and on the [Swift Package Index](https://swiftpackageindex.com/maplibre/maplibre-gl-native-distribution) (for use with the Swift Package Manager). However, for this guide we will add the MapLibre Native as a package dependency directly.

In Xcode, right click your project and select "Add Package Dependencies...":

![](AddPackageDependencies.png)

Paste the following URL and click Add Package:

```
https://github.com/maplibre/maplibre-gl-native-distribution
```

> Note: The [maplibre-gl-native-distributon](https://github.com/maplibre/maplibre-gl-native-distribution) repository only exists for distributing the iOS package of MapLibre Native. To report issues and ask questions, use the [maplibre-native](https://github.com/maplibre/maplibre-native) repository.

Verify you can import MapLibre in your app:

```swift
import MapLibre
```

To use MapLibre with SwiftUI we need to create a wrapper for the UIKit view that MapLibre provides (using UIViewRepresentable. The simplest way to implement this protocol is as follows:

```swift
struct MapLibreMapView: UIViewRepresentable {
    private let mapView = MLNMapView()
    
    func makeUIView(context: Context) -> some UIView {
        return mapView
    }
    
    func updateUIView(_ uiView: UIViewType, context: Context) {
    }
}
```

You can use this view directly in a SwiftUI View hierarcy, for example:

```swift
struct ContentView: View {
    var body: some View {
        MapLibreMapView()
    }
}
```

When running your app in the simulator you should be greeted with the default [Demotiles](https://demotiles.maplibre.org/) style:

![](DemotilesScreenshot.png)
