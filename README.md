# MapLibre GL Native - Open-Source Mapbox GL Native

SDK for iOS, Android and other platforms

MapLibre GL Native is a community led fork derived from [mapbox-gl-native](https://github.com/mapbox/mapbox-gl-native) prior to their switch to a non-OSS license. The fork also includes Maps SDK for iOS and MacOS (forked from [mapbox-gl-native-ios](https://github.com/mapbox/mapbox-gl-native-ios)) and Android SDK (forked from [mapbox-gl-native-android](https://github.com/mapbox/mapbox-gl-native-android)). These platform-specific SDKs were merged under platform directory and they reference mapbox-gl-native directly, not as a submodule.

Beside merging in platform specific SDKs, the following changes were made compared to original mapbox projects:

* The code was upgraded so that it can be built using latest clang compiler / Xcode 12.
* CI/CD was migrated from CircleCI to GitHub Actions. 
* Along with GitHub releases, binaries are distributed as follows:
    * The iOS binaries distribution was upgraded from fat packages to Swift package containing XCFramework.
    * The Android binaries are distributed to GitHub maven package repository.

> The mapbox-gl-native was forked from [d60fd30 - mgbl 1.6.0](https://github.com/mapbox/mapbox-gl-native/tree/d60fd302b1f6563e7d16952f8855122fdcc85f73), mapbox-gl-native-ios from [a139216](https://github.com/mapbox/mapbox-gl-native-ios/commit/a139216) and mapbox-gl-native-android from [4c12fb2](https://github.com/mapbox/mapbox-gl-native-android/commit/4c12fb2c)
## Build Status

| SDK                                                           | Build   | Build status                                                                                                                                                                                  |
|---------------------------------------------------------------|---------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [Maps SDK for iOS](platform/ios/) | CI      | [![GitHub Action build status](https://github.com/maplibre/maplibre-gl-native/workflows/ios-ci/badge.svg)](https://github.com/maplibre/maplibre-gl-native/workflows/ios-ci)                   |
| [Maps SDK for iOS](platform/ios/) | Release | [![GitHub Action build status](https://github.com/maplibre/maplibre-gl-native/workflows/ios-release/badge.svg)](https://github.com/maplibre/maplibre-gl-native/workflows/ios-release)         |
| [Maps SDK for Android](platform/android/)      | CI      | [![GitHub Action build status](https://github.com/maplibre/maplibre-gl-native/workflows/android-ci/badge.svg)](https://github.com/maplibre/maplibre-gl-native/workflows/android-ci)           |
| [Maps SDK for Android](platform/android/)     | Release | [![GitHub Action build status](https://github.com/maplibre/maplibre-gl-native/workflows/android-release/badge.svg)](https://github.com/maplibre/maplibre-gl-native/workflows/android-release) |

## Installation

### Android

1. Add bintray maven repositories to your build.gradle at project level so that you can access MapLibre packages for Android:

    ```gradle
        allprojects {
            repositories {
                ...
                mavenCentral()                
            }
        }
    ```

   > Note: [Bintray was turn off May 1st, 2021](https://jfrog.com/blog/into-the-sunset-bintray-jcenter-gocenter-and-chartcenter/) so we migrated all packages to maven central.

2. Add the library as a dependency into your module build.gradle

    ```gradle
        dependencies {
            ...
            implementation 'org.maplibre.gl:android-sdk:9.4.0'
            ...
        }
    ```

3. Sync gradle and rebuild your app

### iOS

1. To add a package dependency to your Xcode project, select File > Swift Packages > Add Package Dependency and enter its repository URL. You can also navigate to your target’s General pane, and in the “Frameworks, Libraries, and Embedded Content” section, click the + button, select Add Other, and choose Add Package Dependency.

2. Either add MapLibre GitHub distribution URL (https://github.com/maplibre/maplibre-gl-native-distribution) or search for `maplibre-gl-native` package.

3. Choose "next". Xcode should clone the distribution repository and download the binaries.

## Alternative installation

You can also download pre-build from releases in this repository.

## How to create your own build

### Source code checkout

```bash
git clone --recurse-submodules https://github.com/maplibre/maplibre-gl-native.git
```

### Build

MapLibre uses tags for its Android & iOS releases based on [SemVer](https://semver.org) versioning.  This is useful for checking out a particular released version for feature enhancments or debugging.

You can list available tags by issuing the command `git tag`, then use the result

```bash
# 1. Obtain a list of tags, which matches to release versions
git tag

# 2.  Set a convenience variable with the desired TAG
# TAG=android-v9.2.1
# TAG=android-v9.4.2
TAG=ios-v5.11.0
# TAG=ios-v5.12.0-pre.1

# 3.  Check out a particular TAG
git checkout tags/$TAG -b $TAG

# 4. build, debug or enhance features based on the tag
# clean, if you need to troubleshoot build dependencies by using `make clean`
```

#### Android

> Make sure you have set Android SDK path in platform/android/local.properties, variable sdk.dir

```bash
cd platform/android
BUILDTYPE=Release make apackage
```

Binaries are produced in `platform/android/MapboxGLAndroidSDK/build/outputs/aar/MapboxGLAndroidSDK-release.aar`
Please refer to [Mapbox Maps SDK for Android](platform/android/) for detailed instructions.

#### iOS

You can run automated test on a Simulator or Device by changing to the Scheme `iosapp` and choosing `Product` > `Test` (or use `⌘-U`).  Use `⌘-9` to navigate to `Reports` to see results and browse through screenshots.  This method of testing should work well with CI tools such as GitHub Actions, Xcode Server Bots, & AWS Device Farm.

```bash
cd platform/ios

# make and open the Xcode workspace
make iproj

# make Xcode workspace, but run in headless mode
make iproj CI=1

# Make Frameworks
make xcframework BUILDTYPE=Release

# test
make ios-test

# UITests
#   You can review uitest results:  $(IOS_OUTPUT_PATH)/Logs/Test
 make ios-uitest
```

The packaging script will produce a `Mapbox.xcframework` in the  `platform/ios/build/ios/pkg/dynamic` folder.
Please refer to [Mapbox Maps SDK for iOS](platform/ios/platform/ios/) for detailed instructions.


#### MacOS

```bash
cd platform/ios
make xpackage
```

This produces a `Mapbox.framework` in the `platform/ios/build/macos/pkg/` folder.
Please refer to [Mapbox Maps SDK for macos](platform/ios/platform/macos/) for detailed instructions.
