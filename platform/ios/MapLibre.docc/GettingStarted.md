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

## SwiftUI

To use MapLibre with SwiftUI we need to create a wrapper for the UIKit view that MapLibre provides (using UIViewRepresentable. The simplest way to implement this protocol is as follows:

<!-- include-example(SimpleMap) -->

```swift
struct SimpleMap: UIViewRepresentable {
    func makeUIView(context _: Context) -> MLNMapView {
        let mapView = MLNMapView()
        return mapView
    }

    func updateUIView(_: MLNMapView, context _: Context) {}
}
```

You can use this view directly in a SwiftUI View hierarchy, for example:

```swift
struct MyApp: App {
    var body: some Scene {
        WindowGroup {
            SimpleMap().edgesIgnoringSafeArea(.all)
        }
    }
}
```

When running your app in the simulator you should be greeted with the default [Demotiles](https://demotiles.maplibre.org/) style:

![](DemotilesScreenshot.png)

## UIKit

You can use the following `UIViewController` to get started with MapLibre Native iOS with UIKit.

```swift
class SimpleMap: UIViewController, MLNMapViewDelegate {
    var mapView: MLNMapView!

    override func viewDidLoad() {
        super.viewDidLoad()

        mapView = MLNMapView(frame: view.bounds)
        mapView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        view.addSubview(mapView)

        mapView.delegate = self
    }

    // MLNMapViewDelegate method called when map has finished loading
    func mapView(_: MLNMapView, didFinishLoading _: MLNStyle) {
    }
}
```
