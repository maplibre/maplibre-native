# Annotation: Marker

This guide will show you how to add Markers in the map.

`Annotation` is an overlay on top of a Map. In package
`org.maplibre.android.annotations`, it has the following subclasses:

1. [Marker]
2. [Polyline]
3. [Polygon]

A Marker shows an icon image at a geographical location. By default, marker uses
a [provided image] as its icon.

![marker image]

Or, the icon can be customized using [IconFactory] to generate an
[Icon] using a provided image.

For more customization, please read the documentation about [MarkerOptions].

In this showcase, we continue the code from the [Quickstart],
rename Activity into `JsonApiActivity`,
and pull the GeoJSON data from a free and public API.
Then add markers to the map with GeoJSON:

1. In your module Gradle file (usually `<project>/<app-module>/build.gradle`), add `okhttp` to simplify code for making HTTP requests.
  ```gradle
  dependencies {
      ...
      implementation 'com.squareup.okhttp3:okhttp:4.10.0'
      ...
  }
  ```

2. Sync your Android project the with Gradle files.

3. In `JsonApiActivity` we add a new variable for `MapLibreMap`.
   It is used to add annotations to the map instance.
  ```kotlin
  --8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/annotation/JsonApiActivity.kt:top"
  ```

4. Call `mapview.getMapSync()` in order to get a `MapLibreMap` object.
   After `maplibreMap` is assigned, call the `getEarthQuakeDataFromUSGS()` method
   to make a HTTP request and transform data into the map annotations.
  ```kotlin
  --8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/annotation/JsonApiActivity.kt:mapAsync"
  ```

5. Define a function `getEarthQuakeDataFromUSGS()` to fetch GeoJSON data from a public API.
   If we successfully get the response, call `addMarkersToMap()` on the UI thread.
  ```kotlin
  --8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/annotation/JsonApiActivity.kt:getEarthquakes"
  ```

6. Now it is time to add markers into the map.
   - In the `addMarkersToMap()` method, we define two types of bitmap for the marker icon.
   - For each feature in the GeoJSON, add a marker with a snippet about earthquake details.
   - If the magnitude of an earthquake is bigger than 6.0, we use the red icon. Otherwise, we use the blue one.
   - Finally, move the camera to the bounds of the newly added markers
  ```kotlin
  --8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/annotation/JsonApiActivity.kt:addMarkers"
  ```

7. Here is the final result. For the full contents of `JsonApiActivity`, please visit source code of our [Test App].

<div style="align: center">
  <img src="https://github.com/maplibre/maplibre-native/assets/19887090/00446249-9b19-4a48-8a46-00d4c5a2f981" alt="Screenshot with the map in demotile style">
</div>

[Marker]: https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.annotations/-marker/index.html
[provided image]: https://github.com/maplibre/maplibre-native/blob/main/platform/android/MapLibreAndroid/src/main/res/drawable-xxxhdpi/maplibre_marker_icon_default.png
[Polyline]: https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.annotations/-polyline/index.html
[Polygon]: https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.annotations/-polygon/index.html
[marker image]: https://raw.githubusercontent.com/maplibre/maplibre-native/main/test/fixtures/sprites/default_marker.png
[IconFactory]: https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.annotations/-icon-factory/index.html
[Icon]: https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.annotations/-icon/index.html
[Quickstart]: ../getting-started.md
[mvn]: https://mvnrepository.com/artifact/org.maplibre.gl/android-plugin-annotation-v9
[Android Developer Documentation]: https://developer.android.com/topic/libraries/architecture/coroutines
[MarkerOptions]: https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.annotations/-marker-options/index.html
[Test App]: https://github.com/maplibre/maplibre-native/tree/main/platform/android/MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/annotation/JsonApiActivity.kt
