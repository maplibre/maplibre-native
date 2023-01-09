# MapLibre Maps SDK for Android

The MapLibre Maps SDK for Android is a library based on MapLibre GL Native for embedding interactive map views with scalable, customizable vector maps onto Android devices.

## Getting Started

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

## Docs

Visit [https://maplibre.org/maplibre-gl-native/android/api/](https://maplibre.org/maplibre-gl-native/android/api/) to view the current API reference Javadoc files for MapLibre Maps SDK for Android.

## Contributing

See [CONTRIBUTING.md](./CONTRIBUTING.md).