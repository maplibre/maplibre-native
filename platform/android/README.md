# MapLibre Maps SDK for Android

The MapLibre Maps SDK for Android is a library based on MapLibre GL Native for embedding interactive map views with scalable, customizable vector maps onto Android devices.

## Getting Started

This particular README is for people who are interested in working on and improving the Maps SDK for Android. If you're looking for more general information and instructions on the Maps SDK:

Visit [https://docs.mapbox.com/android/maps/overview](https://docs.mapbox.com/android/maps/overview/) to see current documentation on the Maps SDK for Android.

Visit [https://docs.mapbox.com/android/api/map-sdk](https://docs.mapbox.com/android/api/map-sdk/) to view the Maps SDK's current API reference Javadoc files.

See [the Mapbox website's Android install flow](https://www.mapbox.com/install/android/) to install and use the Mapbox Maps SDK for Android in an application.

### Setup environment

These instructions are for developers interested in making code-level contributions to the SDK itself. If you instead want to use the SDK in your app, see above.

#### Getting the source

Clone the git repository and pull in submodules:

```bash
git clone git@github.com:mapbox/mapbox-gl-native-android.git && cd mapbox-gl-native-android
git submodule update --init --recursive
```

#### Installing dependencies

These dependencies are required for all operating systems and all platform targets.

- Latest stable [Android Studio](https://developer.android.com/studio/index.html)
- Update the Mapbox Maps SDK for Android with the latest
  - Android SDK Build-Tools
  - Android Platform-Tools
  - Android SDK Tools
  - CMake
  - NDK
  - LLDB
- Modern C++ compiler that supports `-std=c++14`\*
  - clang++ 3.5 or later or
  - g++-4.9 or later
- [Node.js](https://nodejs.org/)
  - make sure [npm](https://www.npmjs.com) is installed as well
- [ccache](https://ccache.samba.org/) (optional)

**Note**: We partially support C++14 because GCC 4.9 does not fully implement the
final draft of the C++14 standard. More information in [DEVELOPING.md](../../DEVELOPING.md).

**Note**: On macOS you can install clang with installing the [Apple command line developer tools](https://developer.apple.com/download/).

### Opening the project

#### macOS

* Environment:  Android Studio + NDK
  * `JAVA_HOME=/Applications/Android Studio.app/Contents/jre/Contents/Home`
  * `ANDROID_SDK_ROOT=~/Library/Android/sdk`
  * See NDK version discussion on issue: [maplibre/maplibre-gl-native/#xxx](https://github.com/maplibre/maplibre-gl-native/issues/xxx)  

```
cd platform/android

# generate the required build files and open the project with Android Studio
make aproj

# Build package
BUILDTYPE=Debug make apackage
# BUILDTYPE=Release make apackage

# Build release Test App
make android

# Generate javadoc
make android-javadoc
```

#### linux

run `make android-configuration` in the root folder of the project and open in Android Studio.

If you are using Arch Linux, install [ncurses5-compat-libs](https://aur.archlinux.org/packages/ncurses5-compat-libs).

### Project configuration

#### Setup Checkstyle

Mapbox uses specific IDE settings related to code and check style.
See [checkstyle guide](https://github.com/mapbox/mapbox-gl-native-android/wiki/Setting-up-Mapbox-checkstyle) for configuration details.

#### Resolving duplicate file entries
With buck build support, Android Studio can complain about duplicate source files. To remove this warning, open `MapboxGLAndroidSDK.iml` find the list of `excludeFolder` entries and add `<excludeFolder url="file://$MODULE_DIR$/../../../misc/" />` line.

##### Setting API Key

_The test application (used for development purposes) uses MapTiler vector tiles, which require a MapTiler account and API key._

With the first gradle invocation, gradle will take the value of the `MGL_API_KEY` environment variable and save it to `MapboxGLAndroidSDKTestApp/src/main/res/values/developer-config.xml`. If the environment variable wasn't set, you can edit `developer-config.xml` manually and add your api key to the `api_key` resource.  

### Running project

Run the configuration for the `MapboxGLAndroidSDKTestApp` module and select a device or emulator to deploy on. Based on the selected device, the c++ code will be compiled for the related processor architecture. You can see the project compiling in the `View > Tool Windows > Gradle Console`.

More information about building and distributing this project in [DISTRIBUTE.md](DISTRIBUTE.md).

### Additional resources

#### Using the SDK snapshot

Instead of using the latest stable release of the Maps SDK for Android, you can use a "snapshot" or the beta version if there is one available. Our snapshots are built every time a Github pull request adds code to this repository's `master` branch. If you'd like to use a snapshot build, your Android project's gradle file should have -SNAPSHOT appended to the SDK version number:

```java
// Mapbox SDK dependency
implementation 'com.mapbox.mapboxsdk:mapbox-android-sdk:9.1.0-SNAPSHOT'
```

You also need to have the section below in your build.gradle root folder to be able to resolve the SNAPSHOT dependencies:
```
allprojects {
    repositories {
        jcenter()
        maven { url 'https://oss.jfrog.org/artifactory/oss-snapshot-local/' }
    }
}
```


#### Symbolicating native crashes

When hitting native crashes you can use ndk-stack to symbolicate crashes.
More information in [this](https://github.com/mapbox/mapbox-gl-native-android/wiki/Getting-line-numbers-from-an-Android-crash-with-ndk-stack) guide.
