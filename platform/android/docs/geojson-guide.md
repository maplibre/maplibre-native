# Using a GeoJSON Source

This guide will teach you how to use [`GeoJsonSource`](https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.style.sources/-geo-json-source/index.html) by deep diving into [GeoJSON](https://geojson.org/) file format.

## Goals

After finishing  this documentation you should be able to:

1. Understand how `Style`, `Layer`, and `Source` interact with each other.
2. Explore building blocks of GeoJSON data.
3. Use GeoJSON files in constructing `GeoJsonSource`s.
4. Update data at runtime.

## 1. Styles, Layers, and Data source

- A style defines the visual representation of the map such as colors and appearance.
- Layers control how data should be presented to the user.
- Data sources hold actual data and provides layers with it.

Styles consist of collections of layers and a data source. Layers reference data sources. Hence, they require a unique source ID when you construct them.
It would be meaningless if we don't have any data to show, so we need know how to supply data through a data source.

Firstly, we need to understand how to store data and pass it into a data source; therefore, we will discuss GeoJSON in the next session.

## 2. GeoJSON

[GeoJSON](https://geojson.org/) is a JSON file for encoding various geographical data structures.
It defines several JSON objects to represent geospatial information. Typicalle the`.geojson` extension is used for GeoJSON files.
We define the most fundamental objects:

- `Geometry` refers to a single geometric shape that contains one or more coordinates. These shapes are visual objects displayed on a map. A geometry can be one of the following six types:
    - Point
    - MultiPoint
    - LineString
    - MultilineString
    - Polygon
    - MultiPolygon
- `Feautue` is a compound object that combines a single geometry with user-defined attributes, such as name, color.
- `FeatureCollection` is set of features stored in an array. It is a root object that introduces all other features.

A typical GeoJSON structure might look like:

```json
{
  "type": "Feature",
  "geometry": {
    "type": "Point",
    "coordinates": [125.6, 10.1]
  },
  "properties": {
    "name": "Dinagat Islands"
  }
}
```

So far we learned describing geospatial data in GeoJSON files. We will start applying this knowledge into our map applications.

## 3. GeoJsonSource

As we discussed before, map requires some sort data to be rendered. We use different sources such as Vector, Raster and GeoJSON.
We will focus exclusively on `GeoJsonSource` and will not address other sources.

`GeoJsonSource` is a type of source that has a unique `String` ID and GeoJSON data.

There are several ways to construct a `GeoJsonSource`:

- Locally stored files such as assets and raw folders
- Remote services
- Raw string  parsed into FeatureCollections objects
- Geometry, Feature, and FeatureCollection objects that map to GeoJSON Base builders

A sample `GeoJsonSource`:

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/CollectionUpdateOnStyleChange.kt:setupLayer"
```

Note that you can not simply show data on a map. Layers must reference them. Therefore, you create a layer that gives visual appearance to it.

### Creating GeoJSON sources

There are various ways you can create a `GeoJSONSource`. Some of the options are shown below.

```kotlin title="Loading from local files with assets folder file"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/NoStyleActivity.kt:setup"
```

```kotlin title="Loading with raw folder file"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/RuntimeStyleActivity.kt:source"
```

```kotlin title="Parsing inline JSON"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/utils/ResourceUtils.kt:readRawResource"
```

```kotlin title="Loading from remote services"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/HeatmapLayerActivity.kt:createEarthquakeSource"
```

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/HeatmapLayerActivity.kt:constants"
```

```kotlin title="Parsing string with the fromJson method of FeatureCollection"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/turf/MapSnapshotterWithinExpression.kt:fromJson"
```

```kotlin title="Creating Geometry, Feature, and FeatureCollections from scratch"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/feature/QuerySourceFeaturesActivity.kt:JsonObject"
```

Note that the GeoJSON objects we discussed earlier have classes defined in the MapLibre SDK.
Therefore, we can either map JSON objects to regular Java/Kotlin objects or build them directly.

## 4. Updating data at runtime

The key feature of `GeoJsonSource`s is that once we add one, we can set another set of data.
We achieve this using `setGeoJson()` method. For instance, we create a source variable and check if we have not assigned it, then we create a new source object and add it to style; otherwise, we set a different data source:

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/ZoomFunctionSymbolLayerActivity.kt:createFeatureCollection"
```

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/ZoomFunctionSymbolLayerActivity.kt:updateSource"
```

See [this guide](styling/animated-symbol-layer.md) for an advanced example that showcases random cars and a passenger on a map updating their positions with smooth animation.

## Summary

GeoJsonSources have their pros and cons. They are most effective when you want to add additional data to your style or provide features like animating objects on your map.

However, working with large datasets can be challenging if you need to manipulate and store data within the app; in such cases, it’s better to use a remote data source.
