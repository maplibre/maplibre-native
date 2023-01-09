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

**Call for Bounties💰** If you have ideas for new features in MapLibre, you can now nominate them for the MapLibre Bounty Program at https://maplibre.org/news/2022-10-16-call-for-bounties/
## Documentation
MapLibre GL Native documentation is a work in progress. To know about the current state of MapLibre and it's path forward, 
read the Markdown book. To know about API references for Android and iOS, please follow the API Reference links.

1. [MapLibre GL Native Markdown Book](https://maplibre.org/maplibre-gl-native/docs/book/)
2. [MapLibre GL Native iOS API Reference](https://maplibre.org/maplibre-gl-native/ios/api/)
2. [MapLibre GL Native Android API Reference](https://maplibre.org/maplibre-gl-native/android/api/)

## Build Status

| SDK                                                           | Build   | Build status                                                                                                                                                                                  |
|---------------------------------------------------------------|---------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [Maps SDK for iOS](platform/ios/) | CI      | [![GitHub Action build status](https://github.com/maplibre/maplibre-gl-native/workflows/ios-ci/badge.svg)](https://github.com/maplibre/maplibre-gl-native/workflows/ios-ci)                   |
| [Maps SDK for iOS](platform/ios/) | Release | [![GitHub Action build status](https://github.com/maplibre/maplibre-gl-native/workflows/ios-release/badge.svg)](https://github.com/maplibre/maplibre-gl-native/workflows/ios-release)         |
| [Maps SDK for Android](platform/android/)      | CI      | [![GitHub Action build status](https://github.com/maplibre/maplibre-gl-native/workflows/android-ci/badge.svg)](https://github.com/maplibre/maplibre-gl-native/workflows/android-ci)           |
| [Maps SDK for Android](platform/android/)     | Release | [![GitHub Action build status](https://github.com/maplibre/maplibre-gl-native/workflows/android-release/badge.svg)](https://github.com/maplibre/maplibre-gl-native/workflows/android-release) |


## Sponsors

We thank everyone who supported us financially in the past and special thanks to the people and organizations who support us with recurring donations!

Read more about the MapLibre Sponsorship Program at [https://maplibre.org/sponsors/](https://maplibre.org/sponsors/).

Platinum:

<img src="https://maplibre.org/img/aws-logo.svg" alt="Logo AWS" width="25%"/>


Silver:

<img src="https://maplibre.org/img/meta-logo.svg" alt="Logo Meta" width="50%"/>

Stone:

[MIERUNE Inc.](https://www.mierune.co.jp/?lang=en)

Backers and Supporters:

<a href="https://opencollective.com/maplibre/backer/0/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/0/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/1/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/1/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/2/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/2/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/3/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/3/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/4/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/4/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/5/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/5/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/6/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/6/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/7/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/7/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/8/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/8/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/9/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/9/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/10/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/10/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/11/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/11/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/12/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/12/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/13/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/13/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/14/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/14/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/15/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/15/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/16/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/16/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/17/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/17/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/18/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/18/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/19/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/19/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/20/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/20/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/21/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/21/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/22/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/22/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/23/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/23/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/24/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/24/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/25/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/25/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/26/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/26/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/27/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/27/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/28/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/28/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/29/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/29/avatar.svg?requireActive=false"></a>
<a href="https://opencollective.com/maplibre/backer/30/website?requireActive=false" target="_blank"><img src="https://opencollective.com/maplibre/backer/30/avatar.svg?requireActive=false"></a>

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
            implementation 'org.maplibre.gl:android-sdk:<version>'
            ...
        }
    ```

3. Sync gradle and rebuild your app

*Note: MapLibre by default ships with the proprietary Google Play Location Services. If you want to avoid pulling proprietary dependencies into your project, you can exclude Google Play Location Services as follows:*
```gradle
    implementation ('org.maplibre.gl:android-sdk:<version>') {
        exclude group: 'com.google.android.gms'
    }
```

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

## Build

MapLibre uses tags for its Android & iOS releases based on [SemVer](https://semver.org) versioning.  This is useful for checking out a particular released version for feature enhancements or debugging.

You can list available tags by issuing the command `git tag`, then use the result

```bash
# 1. Obtain a list of tags, which matches to release versions
git tag

# 2.  Set a convenience variable with the desired TAG
# TAG=android-v9.4.2
# TAG=android-v9.5.2
TAG=ios-v5.12.0
# TAG=ios-v5.12.0-pre.1

# 3.  Check out a particular TAG
git checkout tags/$TAG -b $TAG

# 4. build, debug or enhance features based on the tag
# clean, if you need to troubleshoot build dependencies by using `make clean`
```


### Build using Bazel

[Bazel](https://bazel.build) is also supported as a build option for getting a packaged release of the xcframework compiled for either static or dynamic linking.

Firstly you will have to ensure that Bazel is installed

`brew install baselisk`

From there you can use the script in platform/ios/platform/ios/scripts/package-bazel.sh

#### There are 4 options:

`cd platform/ios/platform/ios/scripts`

Static xcframework compiled for release (this is default if no parameters are provided):
`./bazel-package.sh --static --release`

Static xcframework compiled for debug:
`./bazel-package.sh --static --debug`

Dynamic xcframework compiled for release:
`./bazel-package.sh --dynamic --release`

Dynamic xcframework compiled for debug:
`./bazel-package.sh --dynamic --debug`

All compiled frameworks will end up in the `bazel-bin/platform/ios/` path from the root of the repo.

Also you can use the link option to ensure that the framework is able to link.

`./bazel-package.sh --link`

#### Bazel build files are placed in a few places throughout the project:

`BUILD.bazel`
- Covering the base cpp in the root `src` directory.

`vendor/BUILD.bazel`
- Covering the submodule dependencies of Maplibre.

`platform/default/BUILD.bazel`
- Covering the cpp dependencies in default.

`platform/darwin/BUILD.bazel`
- Covering the cpp source in platform/default.

`platform/ios/platform/ios/vendor/`
- Covering the iOS specific dependencies.

`platform/ios/BUILD.bazel`
- Covering the source in `platform/ios/platform/ios/src` and `platform/ios/platform/darwin/src` as well as defining all the other BUILD.bazel files and defining the xcframework targets.

#### There are also some other areas that make bazel work:

`WORKSPACE`
- Defines the "repo" and the different modules that are loaded in order to compile for Apple.

`.bazelversion`
- Defines the version of bazel used, important for specific support for Apple targets.

`bazel/flags.bzl`
- Defines some compilation flags that are used between the different build files. 

### Android

---

<details open><summary>macOS Build Environment:  Android Studio + NDK</summary>
<ul>
<li>Environment:  Android Studio + NDK<ul>
<li><code style="font-family: Menlo, Consolas, &quot;DejaVu Sans Mono&quot;, monospace;">JAVA_HOME=/Applications/Android Studio.app/Contents/jre/Contents/Home</code></li>
<li><code style="font-family: Menlo, Consolas, &quot;DejaVu Sans Mono&quot;, monospace;">ANDROID_SDK_ROOT=~/Library/Android/sdk</code></li>
<li><code style="font-family: Menlo, Consolas, &quot;DejaVu Sans Mono&quot;, monospace;">~/Library/Android/sdk/tools/bin/sdkmanager --install ndk;major.minor.build</code></li>
</ul>
</details>


```bash
cd platform/android
BUILDTYPE=Debug make apackage
#BUILDTYPE=Release make apackage
```

Binaries are produced in `platform/android/MapboxGLAndroidSDK/build/outputs/aar/MapboxGLAndroidSDK-<BUILDTYPE>.aar`
Please refer to [Mapbox Maps SDK for Android](platform/android/) for detailed instructions.

### iOS

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
# open macOS project in Xcode
make xproj

# build or test from the command line
make xpackage
make macos-test
```

This produces a `Mapbox.framework` in the `platform/ios/build/macos/pkg/` folder.
Please refer to [Mapbox Maps SDK for macos](platform/ios/platform/macos/) for detailed instructions.

#### Linux

See [the Linux platform build section](platform/linux/) for instructions.

### Building MapLibre-GL-Native Core
MapLibre GL Native shares a single C++ core library with all platforms. To build it, we utilize CMake.

To build, run the following from the root directory
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DMBGL_WITH_CORE_ONLY=ON -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DMBGL_WITH_COVERAGE=ON
```

`CMAKE_BUILD_TYPE=Debug` will build debug artifacts. You can opt to omit it if that is not necessary.
`MBGL_WITH_CORE_ONLY=ON` will build only the core libraries.
Built artifacts should be available on `build` folder.
