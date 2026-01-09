# Distance Expression

{{ activity_source_note("DistanceExpressionActivity.kt") }}

This example shows how you can modify a style to only show certain features within a certain distance to a point. For this the [distance expression](https://maplibre.org/maplibre-style-spec/expressions/#within) is used.

<figure markdown="span">
  ![Screenshot of map where only labels inside some circular area are shown]({{ s3_url("distance_expression_activity.png") }}){ width="400" }
  {{ openmaptiles_caption() }}
</figure>

First we add a [fill layer](https://maplibre.org/maplibre-style-spec/layers/#fill) and a GeoJSON source.

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/DistanceExpressionActivity.kt:FillLayer"
```

Next, we only show features from symbol layers that are less than a certain distance from the point. All symbol layers whose identifier does not start with `poi` are completely hidden.

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/style/DistanceExpressionActivity.kt:distanceExpression"
```
