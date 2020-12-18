# MapBox GL Native SDK Open-Source Fork

For iOS and Android 

## Build Status

| SDK                                                           | Build   | Build status                                                                                                                                                                                  |
|---------------------------------------------------------------|---------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [Mapbox Maps SDK for iOS](mapbox-gl-native-ios/platform/ios/) | CI      | [![Github Action build status](https://github.com/maptiler/maplibre-gl-native/workflows/ios-ci/badge.svg)](https://github.com/maptiler/maplibre-gl-native/workflows/ios-ci)                   |
| [Mapbox Maps SDK for iOS](mapbox-gl-native-ios/platform/ios/) | Release | [![Github Action build status](https://github.com/maptiler/maplibre-gl-native/workflows/ios-release/badge.svg)](https://github.com/maptiler/maplibre-gl-native/workflows/ios-release)         |
| [Mapbox Maps SDK for Android](mapbox-gl-native-android/)      | CI      | [![Github Action build status](https://github.com/maptiler/maplibre-gl-native/workflows/android-ci/badge.svg)](https://github.com/maptiler/maplibre-gl-native/workflows/android-ci)           |
| [Mapbox Maps SDK for Android](mapbox-gl-native-iandroid/)     | Release | [![Github Action build status](https://github.com/maptiler/maplibre-gl-native/workflows/android-release/badge.svg)](https://github.com/maptiler/maplibre-gl-native/workflows/android-release) |

## Installation

### Android

1. Add github maven repositories to your build.gradle at project level so that you can access MapTiler packages for Android:

    ```gradle
        allprojects {
            repositories {
                ...
                maven {
                    url = "https://maven.pkg.github.com/maptiler/mapbox-gl-mobile"
                }
            }
        }
    ```

2. Add the library as a dependency into your module build.gradle

    ```gradle
        dependencies {
            ...
            implementation 'com.maptiler.gl:android-sdk:9.2.1'
            ...
        }
    ```

3. Sync gradle and rebuild your app

### iOS

1. To add a package dependency to your Xcode project, select File > Swift Packages > Add Package Dependency and enter its repository URL. You can also navigate to your target’s General pane, and in the “Frameworks, Libraries, and Embedded Content” section, click the + button, select Add Other, and choose Add Package Dependency.

2. Either add MapTiler GitHub distribution URL (https://github.com/maptiler/maplibre-gl-native-distribution) or search for `maplibre-gl-native` package.

3. Choose "next". Xcode should clone the distribution repository and download the binaries.

## Alternative installation

You can also download pre-build from releases in this repository.

## How to create your own build

### Source code checkout

```bash
git clone --recurse-submodules https://github.com/maptiler/mapbox-gl-mobile.git
```

### Build

#### Android

> Make sure you have set Android SDK path in mapbox-gl-native-android/local.properties, variable sdk.dir

```bash
cd mapbox-gl-native-android
BUILDTYPE=Release make apackage
```

Binaries are produced in `mapbox-gl-native-android/MapboxGLAndroidSDK/build/outputs/aar/MapboxGLAndroidSDK-release.aar`
Please refer to [Mapbox Maps SDK for Android](mapbox-gl-native-android/) for detailed instructions.

#### iOS

```bash
cd mapbox-gl-native-ios
make xcframework BUILDTYPE=Release
```

The packaging script will produce a `build/ios/pkg/dynamic`
Please refer to [Mapbox Maps SDK for iOS](mapbox-gl-native-ios/platform/ios/) for detailed instructions.


#### MacOS

```bash
cd mapbox-gl-native-ios
make xpackage
```

This produces a `Mapbox.framework` in the `build/macos/pkg/` folder.
Please refer to [Mapbox Maps SDK for macos](mapbox-gl-native-ios/platform/macos/) for detailed instructions.