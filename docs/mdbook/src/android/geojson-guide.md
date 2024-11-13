# Introduction

This guide will teach you how to use [`GeoJsonSource`]("https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.style.sources/-geo-json-source/index.html") by deep diving into [`GeoJSON`]("https://geojson.org/") file format.
You will start with fundamentals of how a map renders data internally and why you should prefer [JSON]("https://en.wikipedia.org/wiki/JSON") format in storing geospatial data.

# Goals

After finishing  this documentation you should be able to:
1. Understand how [`Style`]("https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.maps/-style/index.html?query=open%20class%20Style"), [`Layer`]("https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.style.layers/-layer/index.html?query=abstract%20class%20Layer"), and [`Data source`]("https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.style.sources/-source/index.html?query=abstract%20class%20Source") interact with each other.
2. Explore building blocks of `GeoJSON` data.
3. Use `GeoJSON` files in constructing [`GeoJsonSource`]("https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.style.sources/-source/index.html?query=abstract%20class%20Source")s.
4. Update data at runtime.

### 1. Styles, Layers, and Data source

- `Style ` defines the visual representation of the map such as colors and appearance.
- `Layer` controls how data should be presented to the user.
- `Data source`  holds actual data and provides layers with it.

Styles consist of collections of layers and data source. Layers reference data source. Hence they require a unique source ID when you construct them.
It would be meaningless if we don't have any data to show, so we need know how to supply data through a data source.
Firstly, we need to understand how to store data and pass it into a data source; therefore, we will discuss JSON in the next session.

### 2. GeoJSON 

[`GeoJSON`]("https://geojson.org/") is a JSON file for encoding various geographical data structures.
It defines several JSON objects to represent geospatial information. We use the`.geojson` extension for GeoJSON files.
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

A typical GeoJSON file might look like:
```json
 {
  "type": "FeatureCollection",
  "features": [
    {
      "type": "Feature",
      "properties": {},
      "geometry": {
        "type": "Polygon",
        "coordinates": [
          [
            [
              -77.06867337226866,
              38.90467655551809
            ],
            [
              -77.06233263015747,
              38.90479344272695
            ],
            [
              -77.06234335899353,
              38.906463238984344
            ],
            [
              -77.06290125846863,
              38.907206285691615
            ],
            [
              -77.06364154815674,
              38.90684728656818
            ],
            [
              -77.06326603889465,
              38.90637140121084
            ],
            [
              -77.06321239471436,
              38.905561553883246
            ],
            [
              -77.0691454410553,
              38.905436318935635
            ],
            [
              -77.06912398338318,
              38.90466820642439
            ],
            [
              -77.06867337226866,
              38.90467655551809
            ]
          ]
        ]
      }
    }
  ]
}
```
So far we learned describing Geospatial data in GeoJSON files. We will start applying this knowledge into our map applications. 

### 3. GeoJsonSource

As we discussed before, map requires some sort data to be rendered. We use different sources such as [`Vector`]("https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.style.sources/-vector-source/index.html?query=class%20VectorSource%20:%20Source"), [`Raster`]("https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.style.sources/-raster-source/index.html?query=class%20RasterSource%20:%20Source") and `GeoJSON`.
We will focus exclusively on `GeoJsonSource` and will not address other sources.
`GeoJsonSource` is a type of source that has a unique `String` ID and GeoJSON data.

There are several ways to construct a `GeoJsonSource`:
- Locally stored files such as assets and raw folders
- Remote services 
- Raw string  parsed into FeatureCollections objects
- Geometry, Feature, and FeatureCollection objects that map to GeoJSON Base builders

A sample GeoJsonSource:
```kotlin
{{#include ../../../../platform/android/MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/CollectionUpdateOnStyleChange.kt:geojson}}
```

Note that you can not simply show data on a map. Layers must reference them.
Therefore, you create a layer that gives visual appearance to it.

- Loading from local files

with assets folder file
```kotlin
{{#include ../../../../platform/android/MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/NoStyleActivity.kt:setup}}
```
with raw folder file
```kotlin
   val source: Source = try {
            GeoJsonSource("amsterdam-spots", ResourceUtils.readRawResource(this, R.raw.amsterdam))
        } catch (ioException: IOException) {
            Toast.makeText(
                this@RuntimeStyleActivity,
                "Couldn't add source: " + ioException.message,
                Toast.LENGTH_SHORT
            ).show()
            return
        }
        maplibreMap.style!!.addSource(source)
        var layer: FillLayer? = FillLayer("parksLayer", "amsterdam-spots")
        layer!!.setProperties(
            PropertyFactory.fillColor(Color.RED),
            PropertyFactory.fillOutlineColor(Color.BLUE),
            PropertyFactory.fillOpacity(0.3f),
            PropertyFactory.fillAntialias(true)
        )
```
parsing method:
```kotlin
    fun readRawResource(context: Context?, @RawRes rawResource: Int): String {
        var json = ""
        if (context != null) {
            val writer: Writer = StringWriter()
            val buffer = CharArray(1024)
            context.resources.openRawResource(rawResource).use { `is` ->
                val reader: Reader = BufferedReader(InputStreamReader(`is`, "UTF-8"))
                var numRead: Int
                while (reader.read(buffer).also { numRead = it } != -1) {
                    writer.write(buffer, 0, numRead)
                }
            }
            json = writer.toString()
        }
        return json
    }
```
- Loading from remote services
```kotlin
 private fun createEarthquakeSource(): GeoJsonSource {
        return GeoJsonSource(EARTHQUAKE_SOURCE_ID, URI(EARTHQUAKE_SOURCE_URL))
    }
```
```kotlin
 companion object {
        private const val EARTHQUAKE_SOURCE_URL =
            "https://maplibre.org/maplibre-gl-js-docs/assets/earthquakes.geojson"
        private const val EARTHQUAKE_SOURCE_ID = "earthquakes"
        private const val HEATMAP_LAYER_ID = "earthquakes-heat"
        private const val HEATMAP_LAYER_SOURCE = "earthquakes"
        private const val CIRCLE_LAYER_ID = "earthquakes-circle"
    }
```
- Parsing string with `fromJson` method of FeatureCollection
```kotlin
 private fun bufferLineStringGeometry(): Polygon {
        // TODO replace static data by Turf#Buffer: mapbox-java/issues/987
        return FeatureCollection.fromJson(
            """
            {
              "type": "FeatureCollection",
              "features": [
                {
                  "type": "Feature",
                  "properties": {},
                  "geometry": {
                    "type": "Polygon",
                    "coordinates": [
                      [
                        [
                          -77.06867337226866,
                          38.90467655551809
                        ],
                        [
                          -77.06233263015747,
                          38.90479344272695
                        ],
                        [
                          -77.06234335899353,
                          38.906463238984344
                        ],
                        [
                          -77.06290125846863,
                          38.907206285691615
                        ],
                        [
                          -77.06364154815674,
                          38.90684728656818
                        ],
                        [
                          -77.06326603889465,
                          38.90637140121084
                        ],
                        [
                          -77.06321239471436,
                          38.905561553883246
                        ],
                        [
                          -77.0691454410553,
                          38.905436318935635
                        ],
                        [
                          -77.06912398338318,
                          38.90466820642439
                        ],
                        [
                          -77.06867337226866,
                          38.90467655551809
                        ]
                      ]
                    ]
                  }
                }
              ]
            }
            """.trimIndent()
        ).features()!![0].geometry() as Polygon
    }
```
- Creating Geometry, Feature, and FeatureCollections from scratch

Note that the GeoJSON objects we discussed earlier have classes defined in the MapLibre SDK. 
Therefore, we can either map JSON objects to regular Java/Kotlin objects or build them directly.
```kotlin
    val properties = JsonObject()
        properties.addProperty("key1", "value1")
        val source = GeoJsonSource(
            "test-source",
            FeatureCollection.fromFeatures(
                arrayOf(
                    Feature.fromGeometry(Point.fromLngLat(17.1, 51.0), properties),
                    Feature.fromGeometry(Point.fromLngLat(17.2, 51.0), properties),
                    Feature.fromGeometry(Point.fromLngLat(17.3, 51.0), properties),
                    Feature.fromGeometry(Point.fromLngLat(17.4, 51.0), properties)
                )
            )
        )
        style.addSource(source)
        val visible = Expression.eq(Expression.get("key1"), Expression.literal("value1"))
        val invisible = Expression.neq(Expression.get("key1"), Expression.literal("value1"))
        val layer = CircleLayer("test-layer", source.id)
            .withFilter(visible)
        style.addLayer(layer)
```

### 4. Updating data at runtime

The key feature of GeoJsonSources is that once we add one, we can set another set of data.
We achieve this using `setGeoJson()` method. For instance, we create a source variable and check if we have not assigned it, then we create a new source object and add it to style; otherwise, we set a different data source:
```kotlin
  private fun createFeatureCollection(): FeatureCollection {
        val point = if (isInitialPosition) {
            Point.fromLngLat(-74.01618140, 40.701745)
        } else {
            Point.fromLngLat(-73.988097, 40.749864)
        }
        val properties = JsonObject()
        properties.addProperty(KEY_PROPERTY_SELECTED, isSelected)
        val feature = Feature.fromGeometry(point, properties)
        return FeatureCollection.fromFeatures(arrayOf(feature))
    }
```

```kotlin
    private fun updateSource(style: Style?) {
        val featureCollection = createFeatureCollection()
        if (source != null) {
            source!!.setGeoJson(featureCollection)
        } else {
            source = GeoJsonSource(SOURCE_ID, featureCollection)
            style!!.addSource(source!!)
        }
    }
```


Another advanced example showcases random cars and a passenger on a map updating their positions with smooth animation. 
The example defines two sources for the cars and passenger objects and we assign these sources to appropriate layers. 
We use `ValueAnimator` objects and update positions with `onAnimationUpdate()`:

```kotlin
{{#include ../../../../platform/android/MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/AnimatedSymbolLayerActivity.kt}}
```

# Summary

GeoJsonSources have their pros and cons. They are most effective when you want to add additional data to your style or provide features like animating objects on your map. 
However, working with large datasets can be challenging if you need to manipulate and store data within the app; in such cases, it’s better to use a remote data source.
Lastly, you can refer to [`Official Maplibre Android Documentation`]("https://maplibre.org/maplibre-native/android/api/index.html") for a comprehensive guide and  [`Test App`]("https://github.com/maplibre/maplibre-native/tree/main/platform/android/MapLibreAndroidTestApp") to learn best practises for your applications.