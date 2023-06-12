# Developing - MapLibre Native for Android

These instructions are for developers interested in making code-level contributions to the SDK itself.

## Kotlin

All new code should be written in [Kotlin](https://kotlinlang.org/).

### Style Checking

To check Kotlin style, we use [ktlint](https://pinterest.github.io/ktlint/). This linter is based on the [official Kotlin coding conventions](https://kotlinlang.org/docs/coding-conventions.html). We intergrate with it using the [kotlinder](https://github.com/jeremymailen/kotlinter-gradle) Gradle plugin.

To check the style of all Kotlin source files, use:

```
$ ./gradlew checkStyle
```

To format all Kotlin source files, use:

```
$ ./gradlew formatKotlin
```

## Getting the source

Clone the git repository and pull in submodules:

```bash
git clone git@github.com:maplibre/maplibre-native.git
git submodule update --init --recursive
cd platform/android
```

## Installing dependencies

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

## Opening the project

Run

```
make style-code
```

To generate the needed generated sources.

### macOS

* Environment
  * Android Studio
  * node
  * `JAVA_HOME="/Applications/Android Studio.app/Contents/jre/Contents/Home"`
  * `ANDROID_SDK_ROOT=~/Library/Android/sdk`
  * In Android Studio, go to Tools > [SDK Manager](https://developer.android.com/studio/projects/install-ndk#specific-version) to install CMake and the specific NDK version specified in [`dependencies.gradle`](https://github.com/maplibre/maplibre-native/blob/main/platform/android/gradle/dependencies.gradle).

Open `platform/android` with Android Studio.

### Linux

run `make android-configuration` in the root folder of the project and open in Android Studio.

If you are using Arch Linux, install [ncurses5-compat-libs](https://aur.archlinux.org/packages/ncurses5-compat-libs).

## Project configuration

### Setup Checkstyle

Mapbox uses specific IDE settings related to code and check style.
See [checkstyle guide](https://github.com/mapbox/mapbox-gl-native-android/wiki/Setting-up-Mapbox-checkstyle) for configuration details.

#### Resolving duplicate file entries
With buck build support, Android Studio can complain about duplicate source files. To remove this warning, open `MapboxGLAndroidSDK.iml` find the list of `excludeFolder` entries and add `<excludeFolder url="file://$MODULE_DIR$/../../../misc/" />` line.

### Setting API Key

_The test application (used for development purposes) uses MapTiler vector tiles, which require a MapTiler account and API key._

With the first gradle invocation, gradle will take the value of the `MLN_API_KEY` environment variable and save it to `MapboxGLAndroidSDKTestApp/src/main/res/values/developer-config.xml`. If the environment variable wasn't set, you can edit `developer-config.xml` manually and add your api key to the `api_key` resource.  

## Running project

Run the configuration for the `MapboxGLAndroidSDKTestApp` module and select a device or emulator to deploy on. Based on the selected device, the c++ code will be compiled for the related processor architecture. You can see the project compiling in the `View > Tool Windows > Gradle Console`.

More information about building and distributing this project in [DISTRIBUTE.md](DISTRIBUTE.md).

## Additional resources

### Using the SDK snapshot

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

### Symbolicating native crashes

When hitting native crashes you can use ndk-stack to symbolicate crashes.
More information in [this](https://github.com/mapbox/mapbox-gl-native-android/wiki/Getting-line-numbers-from-an-Android-crash-with-ndk-stack) guide.

## Instrumentation Tests

The results of the instrumentation tests can be accessed on the AWS Console:

https://us-west-2.console.aws.amazon.com/devicefarm/home?region=us-east-1#/mobile/projects/20687d72-0e46-403e-8f03-0941850665bc/runs

You can log with the `maplibre` alias, with `maplibre` as username and `maplibre` as password (this is a read-only account).
