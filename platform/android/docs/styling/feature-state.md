# Feature State

{{ activity_source_note("FeatureStateActivity.kt") }}

This example shows how to use [`feature-state`](https://maplibre.org/maplibre-style-spec/expressions/#feature-state) for interactive styling on Android.

Feature state lets you attach runtime JSON values to individual features, then read those values back inside style expressions. This is useful for interaction-driven UI such as selection, hover, highlighting, or temporary analysis state.

## Prepare features with stable ids

Feature state targets features by id, so the source data must already contain feature ids.

This example builds a small inline GeoJSON source with explicit ids:

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/feature/FeatureStateActivity.kt:createRegions"
```

## Style a layer with feature-state expressions

The fill and line layers read the `selected` state key and change color, opacity, and line width accordingly:

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/feature/FeatureStateActivity.kt:addFeatureStateLayers"
```

## Toggle state on tap

On tap, the example queries the rendered feature under the touch point, reads its current state, then writes back the toggled value:

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/feature/FeatureStateActivity.kt:toggleFeatureSelection"
```

## Notes

- Use `MapLibreMap.setFeatureState()` to merge new keys into a feature's state.
- Use `MapLibreMap.getFeatureState()` to inspect the current state for one feature.
- Use `MapLibreMap.removeFeatureState()` or `MapLibreMap.resetFeatureStates()` to clear state.
- For vector tile sources, you must pass `sourceLayerId`.
- This Android example uses embedded feature ids directly in GeoJSON so it works without extra source configuration.
