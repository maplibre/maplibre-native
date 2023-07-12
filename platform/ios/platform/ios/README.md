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

The following are instructions on how to build MapLibre Native for development purposes. 

### Downloading Source

Download the source and install all submodules if you have not already, by running the following from the root of the repository. 

```
git clone --recurse-submodules git@github.com:maplibre/maplibre-native.git
cd maplibre-native
```

Next run the following to generate some files that are needed:

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

#### Creating `config.bzl`

You need to configure Bazel. Copy the example config from `platform/ios`.

```
cp bazel/example_config.bzl bazel/config.bzl
```

You need to set your `BUNDLE_ID_PREFIX` to be unique (ideally use a domain that you own in reverse domain name notation).

```
APPLE_MOBILE_PROVISIONING_PROFILE_TEAM_ID = "FFNX6834U8"
APPLE_MOBILE_PROVISIONING_PROFILE_NAME = "iOS Team Provisioning Profile: *"
BUNDLE_ID_PREFIX = "com.firstnamelastname"
```

You can keep leave the `APPLE_MOBILE_PROVISIONING_PROFILE_NAME` alone.

Set the Team ID to the Team ID of your Apple Developer Account (paid or unpaid both work). If you do not know your Team ID, enter the following command in the terminal:

```
cd ~/Library/MobileDevice/Provisioning\ Profiles && open .
```

Then select one of the profiles and click spacebar. Your Team ID is the string between parentheses in the value of "Team".

If there are no provisioning profiles available, continue this guide and let Xcode generate a provisioning profile for you. You will need to update the Team ID after this happened.

#### Create the Xcode Project

_These instructions are for XCode 14.3.1_

From `platform/ios`:

```
bazel run xcodeproj
xed MapLibre.xcodeproj
```

Then once in Xcode, click on "MapLibre" on the left, then "App" under Targets, then "Signing & Capabilities" in the tabbed menu. 
Confirm that no errors are shown:

<img width="921" alt="image" src="https://github.com/polvi/maplibre-native/assets/649392/a1ef30cb-97fc-429a-acee-194436f3fb8a">

Try to run the example App in the simulator and on a device to confirm your setup works.

#### Using Bazel from the Command Line

It is also possible to build and run the test application in a simulator from the command line without opening Xcode.

```
bazel run //platform/ios:App

# run the iOS tests
bazel test //platform/ios/platform/ios/test:ios_test --test_output=errors
```

### CMake (deprecated)

The original build tool generator supported by MapLibre Native is CMake. There is a `Makefile` which calls CMake and `xcodebuild` under the hood to complete various development tasks, including building with various kinds of configurations and running tests. This `Makefile` can also can set up an Xcode project for MapLibre Native development by copying a `.xcodeproj` file part of the source tree and combining that with the output of CMake.

To set up an Xcode project and open Xcode, use the following command from `platform/ios`:

```
make iproj
```


## Documentation

- [MapLibre Native for iOS API Reference](https://maplibre.org/maplibre-native/ios/api/)
