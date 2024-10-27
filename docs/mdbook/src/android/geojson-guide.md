# Introduction

This guide will teach you how to use [`GeoJsonDataSource`]("https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.style.sources/-geo-json-source/index.html") by deep diving into [`GeoJson`]("https://geojson.org/") file format.

You will start with fundamentals of how map renders data under the hood and why [`JSON`]("https://en.wikipedia.org/wiki/JSON") format is preferred in storing geospatial data

# Goals
After finishing  this documentation you should be able to

1. understand how [`Style`]("https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.maps/-style/index.html?query=open%20class%20Style"), [`Layer`]("https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.style.layers/-layer/index.html?query=abstract%20class%20Layer"), and [`DataSource`]("https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.style.sources/-source/index.html?query=abstract%20class%20Source") interact with each other
2. describe `JSON` files
3. explore building blocks of `GeoJson` data
4. use `GeoJson` files in constructing [`GeoJsonDataSources`]("https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.style.sources/-source/index.html?query=abstract%20class%20Source")
5. update data at runtime

### 1. Styles, Layers, and DataSources
- `Style ` defines the visual representation of the map such as colors and appearance.

- `Layer` controls how data should be presented to the user

- `Datasource`  holds actual data and provides layers with it

Styles consist of collections of layers and datasources. Layers reference datasources. Hence they require unique source ID when they are constructed

It would be meaningless if we don't have any data to show so we need know supplying data with datasources.

Firstly, we need to understand how to store data and passing them into datasources and therefore `JSON` comes into play in the next session.

### 2. JSON
`JSON` stands for JavaScript Object Notation. It is a key-value pair text format for storing and retrieving data between client and server. 

It is popular because it is human readable and easy to parse into objects.

JSON content is treated as string and it makes easy to transmit data across clients. JSON file is saved in .json extension

A sample json file:
```json
{
  "name": "John Doe",
  "age": 30,
  "isStudent": false,
  "contact": {
    "email": "johndoe@example.com",
    "phone": "123-456-7890"
  },
  "hobbies": ["reading", "traveling", "coding"]
}

```
With its straightforward syntax, we can benefit from it by describing our geospatial data inside json

### 3. GeoJson 
[`GeoJson`]("https://geojson.org/") is a json file for encoding various geographical data structures.

It defines several json objects to represent geospatial information. GeoJson file is saved with `.geojson` extension

The following list illustrates the most fundamental objects:

- Geometry
- Feature
- FeatureCollection

`Geometry` is a single geometric shape that contains one or more coordinates.

A geometry can be one of the following six types:

- Point
- MultiPoint
- LineString
- MultilineString
- Polygon
- MultiPolygon

They are visual objects that seen on map with one or more coordinates. We can use each depending on our requirements

`Feautue` is a compound object of single geometry and user-defined attributes applied to that geometry such as name, color, etc.

`FeatureCollection` is set of features stored in an array. It is a root object that introduces all other features.

A typical GeoJson file might look like:
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
So far we learned describing Geospatial data in geojson files. We will start applying this knowledge into our map applications 

### 4. GeoJsonDataSource
As we discussed before, map requires some sort data to be rendered. We use different sources such as [`Vector`]("https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.style.sources/-vector-source/index.html?query=class%20VectorSource%20:%20Source"), [`Raster`]("https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.style.sources/-raster-source/index.html?query=class%20RasterSource%20:%20Source") and `GeoJson`.
We will not deal with other sources and only focus on `GeoJsonDataSource`.

`GeoJsonDataSource` is a type of source that has a unique `String` ID and GeoJson data. There are several ways of creating and adding `GeoJsonDataSource` objects.

A GeoJsonDataSource might be constructed with several approaches:

- Locally stored files such as assets and raw folders
- Remote services 
- Raw string  parsed into FeatureCollections objects
- Geometry, Feature, and FeatureCollection objects that map to GeoJson Base builders

A sample GeoJsonDatasource:
```kotlin
val source = GeoJsonSource("source", featureCollection)
val lineLayer = LineLayer("layer", "source")
    .withProperties(
        PropertyFactory.lineColor(Color.RED),
        PropertyFactory.lineWidth(10f)
    )

style.addSource(source)
style.addLayer(lineLayer)
```

Note that dataSources can not be simply shown. Layers must reference them. Therefore, you create a layer that gives visual appearance to it.

- Loading from local files

with assets folder file
```kotlin
 binding.mapView.getMapAsync { map ->
            map.moveCamera(CameraUpdateFactory.newLatLngZoom(cameraTarget, cameraZoom))
            map.setStyle(
                Style.Builder()
                    .withImage(imageId, imageIcon)
                    .withSource(GeoJsonSource(sourceId, URI("asset://points-sf.geojson")))
                    .withLayer(SymbolLayer(layerId, sourceId).withProperties(iconImage(imageId)))
            )
        }
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
 
Note that GeoJson objects as we discussed above have classes defined in MapLibre SDK.
So we can map json objects into regular Java/Kotlin objects or build straightaway.
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

### 5. Updating data at runtime
The most important feature of GeoJsonDataSources, once we add one of them, we can set another set of data.
We achieve this with `setGeoJson()` method.

For example we create a source variable and check if it is not assigned, then we create new source object and add it to style, otherwise we set another data:
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

Probably the most advanced use case might be animating random objects such as cars. 

```kotlin
{{#include ../../../../platform/android/MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/AnimatedSymbolLayerActivity.kt}}
```

# Summary
GeoJsonDataSources come with their pros and cons. They work best if you want to add additional data to your style , or provide features like animating objects on your map.

It would be difficult if you have large datasets and you want to manipulate and store inside app. In that case you should prefer remote datasources.

Lastly, you may check [`Official Maplibre Android Documentation`]("https://maplibre.org/maplibre-native/android/api/index.html") for full guide and  [`Test App`]("https://github.com/maplibre/maplibre-native/tree/main/platform/android/MapLibreAndroidTestApp") in order learn best practises to adopt in your apps.