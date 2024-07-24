# MapLibre Configuration
This guide will explain different ways to create a map.

When working with maps there are chances of providing default values for MapView to render.

There are several ways to build MapView:
1. Providing existing XML namespace tags of MapView in layout.
2. Creating `MapLibreMapOptions` and providing builder function values into MapView.
3. Creating `SupportMapFragment` with the help of `MapLibreMapOptions`.

Before explaining MapView configurations we need to know what we can do with both XML namespaces and `MapLibreMaptions`.


Common configurations can be set:
- Map base uri
- Camera
- Zoom
- Pitch
- Gestures
- Compass
- Logo
- Attribution
- Placement of above elements on the Map and more

We will see how to achieve these configurations in XML layout and programmatically in Activity code step by step.

### 1. MapView Configuration inside XML layout.
We need to use MapView namespace and provide some data in layout file.
```xml
{{#include ../../../../platform/android/MapLibreAndroidTestApp/src/main/res/layout/activity_map_options_xml.xml}}
```
```
This can be found in platform/android/MapLibreAndroidTestApp/src/main/res/layout/activity_map_options_xml.xml
```
We can give any other existing values to `maplibre` tags and  only need to create Mapview and MapLibreMap objects with simple setup in Activity.
```kotlin
{{#include ../../../../platform/android/MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/options/MapOptionsXmlActivity.kt}}
```
```
This can be found in platform/android/MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/options/MapOptionsXmlActivity.kt
```
### 2. MapView configuration with `MapLibreMapOptions`

 Here we don't have to create MapView from XML since we want to create it programmatically.
```xml
{{#include ../../../../platform/android/MapLibreAndroidTestApp/src/main/res/layout/activity_map_options_xml.xml}}
```
```
This can be found in platform/android/MapLibreAndroidTestApp/src/main/res/layout/activity_map_options_runtime.xml
```
A `MapLibreMapOptions` object must be created and passed to MapView constructor. And all setup happens in Activity code:
```kotlin
{{#include ../../../../platform/android/MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/options/MapOptionsRuntimeActivity.kt}}
```
```
This can be found in platform/android/MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/options/MapOptionsRuntimeActivity.kt}}
```
Finally we will see a result similar to this :
<div style="align: center">
  <img src="https://github.com/user-attachments/assets/dd85f496-3e6f-4788-933e-4ec3d5999935" alt="Screenshot with the MapLibreMapOptions">
</div>

For full content of `MapOptionsRuntimeActivity` and `MapOptionsXmlActivity`, please visit source code of [Test APP](https://github.com/maplibre/maplibre-native/tree/main/platform/android/MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/options).

You can read more about `MapLibreMapOptions` in the [documentation](https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.maps/-map-libre-map-options/index.html?query=open%20class%20MapLibreMapOptions%20:%20Parcelable).

### 3. `SupportMapFragment` with the help of `MapLibreMapOptions`.

 If you are using MapFragment in your project, it is also easy to give initial values to the `newInstance()` static method of `SupportMapFragment` which requires passing `MapLibreMapOptions` parameter.

Let's see how we can do in a sample activity:

```kotlin
{{#include ../../../../platform/android/MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/fragment/SupportMapFragmentActivity.kt}}
```
```
This can be found platform/android/MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/fragment/SupportMapFragmentActivity.kt}}
```
You can also find full contents of `SupportMapFragmentActivity` in the [Test App](https://github.com/maplibre/maplibre-native/tree/main/platform/android/MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/fragment/SupportMapFragmentActivity.kt).

To learn more about `SupportMapFragment`, please visit the [documentation](https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.maps/-support-map-fragment/index.html?query=open%20class%20SupportMapFragment%20:%20Fragment,%20OnMapReadyCallback).
