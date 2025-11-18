# Circle Layer (with Clustering)

{{ activity_source_note("CircleLayerActivity.kt") }}

In this example we will add a circle layer for a GeoJSON source. We also show how you can use the [cluster](https://maplibre.org/maplibre-style-spec/sources/#cluster) property of a GeoJSON source.

<figure markdown="span">
  <video controls width="400" poster="{{ s3_url("circle_layer_cluster_thumbnail.jpg") }}" >
    <source src="{{ s3_url("circle_layer_cluster.mp4") }}" />
  </video>
</figure>

Create a `GeoJsonSource` instance, pass a unique identifier for the source and the URL where the GeoJSON is available. Next add the source to the style.

```kotlin title="Setting up the GeoJSON source"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/CircleLayerActivity.kt:addBusStopSource"
```

Now you can create a `CircleLayer`, pass it a unique identifier for the layer and the source identifier of the GeoJSON source just created. You can use a `PropertyFactory` to pass [circle layer properties](https://maplibre.org/maplibre-style-spec/layers/#circle). Lastly add the layer to your style.

```kotlin title="Create circle layer a small orange circle for each bus stop"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/CircleLayerActivity.kt:addBusStopCircleLayer"
```

## Clustering

Next we will show you how you can use clustering. Create a `GeoJsonSource` as before, but with some additional options to enable clustering.

```kotlin title="Setting up the clustered GeoJSON source"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/CircleLayerActivity.kt:addClusteredSource"
```

When enabling clustering some [special attributes](https://maplibre.org/maplibre-style-spec/sources/#cluster) will be available to the points in the newly created layer. One is `cluster`, which is true if the point indicates a cluster. We want to show a bus stop for points that are **not** clustered.

```kotlin title="Add a symbol layers for points that are not clustered"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/CircleLayerActivity.kt:unclusteredLayer"
```

Next we define which point amounts correspond to which colors. More than 150 points will get a red circle, clusters with 21-150 points will be green and clusters with 20 or less points will be green.

```kotlin title="Define different colors for different point amounts"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/CircleLayerActivity.kt:clusteredCircleLayers"
```

Lastly we iterate over the array of `Pair`s to create a `CircleLayer` for each element.

```kotlin title="Add different circle layers for clusters of different point amounts"
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/CircleLayerActivity.kt:clusteredCircleLayersLoop"
```
