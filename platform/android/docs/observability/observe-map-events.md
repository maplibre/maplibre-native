# Observe Map Events

{{ activity_source_note("ObserverActivity.kt") }}

You can observe-low level map events that are happening by registering listeners to a `MapView`. Below you can see the the map events that are currently available.

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/events/ObserverActivity.kt:mapEvents"
```

You need to register them with these APIs:

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/events/ObserverActivity.kt:addListeners"
```

In this case we implement them by implementing the interfaces below in the activity class, but you could also use lambdas.

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/events/ObserverActivity.kt:ObserverActivity"
```

`ObserverActivity.onDidFinishRenderingFrame` uses `RenderStatsTracker` as an example for tracking rendering statistics over time. This offers periodic reports of minimum, maximum, average values and callbacks when predefined thresholds are exceeded.

```kotlin
--8<-- "MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/events/ObserverActivity.kt:renderStatsTracker"
```
