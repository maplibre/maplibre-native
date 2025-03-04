# Add live realtime data

{{ activity_source_note("RealTimeGeoJsonActivity.kt") }}

In this example you will learn how to add a live GeoJSON source. We have set up a [lambda function](https://m6rgfvqjp34nnwqcdm4cmmy3cm0dtupu.lambda-url.us-east-1.on.aws/) that returns a new GeoJSON point every time it is called.

<figure markdown="span">
  <video controls width="250" poster="{{ s3_url("live_realtime_data_thumbnail.jpg") }}" >
    <source src="{{ s3_url("live_realtime_data.mp4") }}" />
  </video>
</figure>

First we will create a `GeoJSONSource`.

```kotlin title="Adding GeoJSON source"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/RealTimeGeoJsonActivity.kt:addSource"
```

Next we will create a `SymbolLayer` that uses the source.

```kotlin title="Adding a SymbolLayer source"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/RealTimeGeoJsonActivity.kt:addLayer"
```

We use define a `Runnable` and use `android.os.Handler` with a `android.os.Looper` to update the GeoJSON source every 2 seconds.

```kotlin title="Defining a Runnable for updating the GeoJSON source"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/RealTimeGeoJsonActivity.kt:Runnable"
```

## Bonus: set icon rotation

You can set the icon rotation of the icon when ever the point is updated based on the last two points.

```kotlin title="Defining a Runnable for updating the GeoJSON source"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/RealTimeGeoJsonActivity.kt:setIconRotation"
```
