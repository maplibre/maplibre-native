# Developing - MapLibre Native for Android

These instructions are for developers interested in making code-level contributions to MapLibre Native for Android.

## Getting the source

Clone the git repository and pull in submodules:

```bash
git clone git@github.com:maplibre/maplibre-native.git
cd maplibre-native
git submodule update --init --recursive
cd platform/android
```

## Requirements

[Android Studio](https://developer.android.com/studio) needs to be installed.

Open the `platform/android` directory to get started.

## Setting an API Key

_The test application (used for development purposes) uses MapTiler vector tiles, which require a MapTiler account and API key._

With the first Gradle invocation, Gradle will take the value of the `MLN_API_KEY` environment variable and save it to `MapLibreAndroidTestApp/src/main/res/values/developer-config.xml`. If the environment variable wasn't set, you can edit `developer-config.xml` manually and add your API key to the `api_key` resource.  

## Running the TestApp

Run the configuration for the `MapLibreAndroidTestApp` module and select a device or emulator to deploy on.

<p align="left">
  <img src="https://github.com/maplibre/maplibre-native/assets/649392/5494925e-8cbb-4d5d-8033-8a2f141ede3c" alt="Android TestApp menu" width="15%">   <img src="https://github.com/maplibre/maplibre-native/assets/649392/f169db51-615d-4fca-b297-ac6197bec674" alt="Android TestApp showing Demotiles" width="15%">
</p>

## Render Tests

To run the render tests for Android, run the configuration for the `androidRenderTest.app` module.

More information on working on the render tests can be found [in the wiki](https://github.com/maplibre/maplibre-native/wiki/Working-on-Android-Render-Tests).

## Instrumentation Tests

To run the instrumentation tests, choose the "Instrumentation Tests" run configuration.

Your device needs remain unlocked for the duration of the tests.

### AWS Device Farm

The instrumentation tests are running on AWS Device Farm. To see the results and the logs, go to:

https://us-west-2.console.aws.amazon.com/devicefarm/home?region=us-east-1#/mobile/projects/20687d72-0e46-403e-8f03-0941850665bc/runs

You can log in with the `maplibre` alias, with `maplibre` as username and `maplibre` as password (this is a read-only account).

## Kotlin

All new code should be written in [Kotlin](https://kotlinlang.org/).

## Style Checking

To check Kotlin style, we use [ktlint](https://pinterest.github.io/ktlint/). This linter is based on the [official Kotlin coding conventions](https://kotlinlang.org/docs/coding-conventions.html). We intergrate with it using the [kotlinder](https://github.com/jeremymailen/kotlinter-gradle) Gradle plugin.

To check the style of all Kotlin source files, use:

```
$ ./gradlew checkStyle
```

To format all Kotlin source files, use:

```
$ ./gradlew formatKotlin
```

## Benchmarks in Pull Request

To run the benchmarks (for Android) include the following line on a PR comment:

```
!benchmark android
```
