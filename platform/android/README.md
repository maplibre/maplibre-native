# MapLibre GL Native for Android

[![GitHub Action build status](https://github.com/maplibre/maplibre-gl-native/workflows/android-ci/badge.svg)](https://github.com/maplibre/maplibre-gl-native/actions/workflows/android-ci.yml) [![GitHub Action build status](https://github.com/maplibre/maplibre-gl-native/workflows/android-release/badge.svg)](https://github.com/maplibre/maplibre-gl-native/actions/workflows/android-release.yml)

MapLibre GL Native for Android is a library for embedding interactive map views with scalable, customizable vector maps onto Android devices.

## Getting Started

1. Add bintray Maven repositories to your `build.gradle` at project level so that you can access MapLibre packages for Android:

    ```gradle
        allprojects {
            repositories {
                ...
                mavenCentral()                
            }
        }
    ```

2. Add the library as a dependency into your module build.gradle

    ```gradle
        dependencies {
            ...
            implementation 'org.maplibre.gl:android-sdk:<version>'
            ...
        }
    ```

3. Sync Gradle and rebuild your app.

## Documentation

Visit [https://maplibre.org/maplibre-gl-native/android/api/](https://maplibre.org/maplibre-gl-native/android/api/) to view the current API reference Javadoc files for MapLibre GL Native for Android.

## Contributing

See [`DEVELOPING.md`](./DEVELOPING.md) for instructions on how to get started working on the codebase.
