# Feature State

{{ activity_source_note("FeatureStateActivity.kt") }}

This example shows how to use [`feature-state`](https://maplibre.org/maplibre-style-spec/expressions/#feature-state) for interactive styling on Android, matching the iOS feature-state example.

Feature state lets you attach runtime JSON values to individual features, then read those values back inside style expressions. This is useful for interaction-driven UI such as selection, highlighting, or temporary analysis state.

## Load US states from a remote GeoJSON source

The example loads the same US states GeoJSON used by the GLFW and iOS demos:

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/feature/FeatureStateActivity.kt:addStatesLayer"
```

## Style layers with feature-state expressions

The fill color/opacity and border color/width respond to a `selected` boolean stored in feature state:

- **Selected** — red fill, higher opacity, thicker red border
- **Default** — blue fill, semi-transparent, thin blue border

## Toggle selection on tap

Tapping a state toggles its `selected` feature-state key and shows a toast:

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/feature/FeatureStateActivity.kt:handleMapClick"
```

## Notes

- Use `MapLibreMap.setFeatureState()` to merge new keys into a feature's state.
- Use `MapLibreMap.getFeatureState()` to inspect the current state for one feature.
- Use `MapLibreMap.removeFeatureState()` or `MapLibreMap.resetFeatureStates()` to clear state.
- For vector tile sources, you must pass `sourceLayerId`.
