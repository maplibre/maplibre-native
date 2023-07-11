# MapLibre Native for iOS

[![GitHub Action build status](https://github.com/maplibre/maplibre-native/workflows/ios-ci/badge.svg)](https://github.com/maplibre/maplibre-native/actions/workflows/ios-ci.yml) [![GitHub Action build status](https://github.com/maplibre/maplibre-native/workflows/ios-release/badge.svg)](https://github.com/maplibre/maplibre-native/actions/workflows/ios-release.yml)

A library based on [MapLibre Native](https://github.com/maplibre/maplibre-native) for embedding interactive map views with scalable, customizable vector maps into iOS Applications.

## Getting Started

MapLibre Native for iOS is distributed using the [Swift Package Index](https://swiftpackageindex.com/maplibre/maplibre-gl-native-distribution). To add it to your project, follow the steps below.

1. To add a package dependency to your Xcode project, select File > Swift Packages > Add Package Dependency and enter its repository URL. You can also navigate to your target’s General pane, and in the “Frameworks, Libraries, and Embedded Content” section, click the + button, select Add Other, and choose Add Package Dependency.

2. Either add MapLibre GitHub distribution URL `https://github.com/maplibre/maplibre-gl-native-distribution` or search for `maplibre-native` package.

3. Choose "Next". Xcode should clone the distribution repository and download the binaries.

4. To create a minimal app, update `ContentView.swift` (which should have been automatically created when you initalized the new XCode project) with the following contents:

```swift
import SwiftUI
import Mapbox

struct ContentView: View {
    var body: some View {
        MapView().edgesIgnoringSafeArea(.all)
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}

struct MapView: UIViewRepresentable {
    func makeUIView(context: Context) -> MGLMapView {
        // Build the style URL
        let styleURL = URL(string: "https://demotiles.maplibre.org/style.json")
        
        // Create the map view
        let mapView = MGLMapView(frame: .zero, styleURL: styleURL)
        
        mapView.logoView.isHidden = true
        
        mapView.setCenter(
            CLLocationCoordinate2D(
                latitude: 23.16, longitude: -109.50), animated: false)
        
        mapView.setZoomLevel(4, animated: false)
        return mapView
        
    }
    
    func updateUIView(_ mapView: MGLMapView, context: Context) {
        // Update the view if needed
    }
}
```

There is a an open bounty to extend this Getting Started guide ([#809](https://github.com/maplibre/maplibre-native/issues/809)). In the meantime, refer to one of these external guides:

- [Get Started with MapLibre Native for iOS using SwiftUI](https://docs.maptiler.com/maplibre-gl-native-ios/ios-swiftui-basic-get-started/)
- [Get Started With MapLibre Native for iOS using UIKit](https://docs.maptiler.com/maplibre-gl-native-ios/ios-uikit-basic-get-started/)

## Developing

The following are instructions on how to build `maplibre-native` for development purposes. 

### Downloading Source

Download the source and install all submodules if you have not already, by running the following from the root of the repository. 

```
git clone git@github.com:maplibre/maplibre-native.git maplibre-native
cd maplibre-native
git submodule update --init --recursive
```

Next run the following to add required dependencies:

```
cd platform/ios
make style-code
```


### Bazel

[Bazel](https://bazel.build/) together with [rules_xcodeproj](https://github.com/MobileNativeFoundation/rules_xcodeproj) is the preferred build system. Please [share your experiences](https://github.com/maplibre/maplibre-native/discussions/1145).

You need to install bazelisk, which is a wrapper around Bazel which ensures that the version specified in `.bazelversion` is used.

```
brew install bazelisk
```

#### Configure Bazel: config.bzl

You will need to edit bazel/config.bzl to match your provisioning team id. If this is your first time building this iOS application, you may not have one. 

If this is a fresh install, the easiest thing to do is to build the xcode project, then open XCode, sign in, and create a new provisioning ID. To get started, from `platform/ios`:

```
cp bazel/example_config.bzl bazel/config.bzl
```

Unless you already know your provisioning team ID, skip editing it for now, and move on to creating the XCode project. 

### Create the XCode Project

_These instructions are for XCode 14.3.1_

From `platform/ios`:

```
bazel run //platform/ios:xcodeproj
open MapLibre.xcodeproj
```

Then once in XCode, click on "MapLibre" on the left, then "App" under Targets, then "Signing & Capabilities" in the tabbed menu. In the below screenshot, the "TC45MCF93C" is the default profile that is in `bazel/config.bzl`. To fix this add a valid team and update to a unique bundle identifier. 

<img width="1127" alt="xcode-signing-capabilities" src="https://github.com/polvi/maplibre-native/assets/46035/77b9f60c-d60e-464b-929a-590326723ac9">

Once you have done that, you should see a new profile in `~/Library/MobileDevice/Provisioning\ Profiles/`. For example:

```
~/Library/MobileDevice/Provisioning\ Profiles/5fe347ab-13b4-4669-96eb-198e0890a303.mobileprovision 
```

To set the correct team id, do the following:

```
open ~/Library/MobileDevice/Provisioning\ Profiles/
single-click the name of the profile
press spacebar
```

You should see something like the following:

<img width="787" alt="xcode-provisioning-profile" src="https://github.com/polvi/maplibre-native/assets/46035/3172165b-227f-4bce-a9e4-a71665c6074b">


Edit bazel/config.bzl:

```
APPLE_MOBILE_PROVISIONING_PROFILE_TEAM_ID = "HXK82SM8MM"
```

This is a temporary fix until #1341 is fixed, but also edit BUILD.bazel:

```
--- a/platform/ios/BUILD.bazel
+++ b/platform/ios/BUILD.bazel
@@ -508,7 +508,7 @@ genrule(
 
 local_provisioning_profile(
     name = "provisioning_profile",
-    profile_name = "iOS Team Provisioning Profile: *",
+    profile_name = "5fe347ab-13b4-4669-96eb-198e0890a303",
     team_id = APPLE_MOBILE_PROVISIONING_PROFILE_TEAM_ID,
 )
```

Once this is done, close XCode and rebuild the project with:

```
bazel run //platform/ios:xcodeproj
```

You can now open `platform/ios/MapLibre.xcodeproj` with Xcode to get started.

It is also possible to build and run the test application in a simulator from the command line without opening Xcode.

```
bazel run //platform/ios:App
```

### CMake (deprecated)

The original build tool generator supported by MapLibre Native is CMake. There is a `Makefile` which calls CMake and `xcodebuild` under the hood to complete various development tasks, including building with various kinds of configurations and running tests. This `Makefile` can also can set up an Xcode project for MapLibre Native development by copying a `.xcodeproj` file part of the source tree and combining that with the output of CMake.

To set up an Xcode project and open Xcode, use the following command from `platform/ios`:

```
make iproj
```


## Documentation

- [MapLibre Native for iOS API Reference](https://maplibre.org/maplibre-native/ios/api/)
