# Using AttributionDialogManager (Android)

This guide shows how to use `AttributionDialogManager` in MapLibre Android to control what happens when the user taps the attribution icon.

## How attribution works

- MapLibre Android uses an `AttributionDialogManager` as the click handler for the attribution view.
- `AttributionDialogManager` implements `View.OnClickListener`, so tapping the attribution icon triggers `AttributionDialogManager#onClick(View)`.
- By default, `MapView` creates a `defaultDialogManager`.
- You can provide your own custom manager through `UiSettings#setAttributionDialogManager(...)`.
  When a custom manager is set, `MapView` will use it instead of the default one (see `MapView#getDialogManager()`).

## Option 1: Use the default attribution dialog

If you don’t set a custom manager, MapLibre will use its default dialog manager automatically.

## Option 2: Provide a custom AttributionDialogManager

You can replace the default attribution behavior by setting your own manager:

```java
import android.content.Intent;
import android.view.View;

import org.maplibre.android.maps.AttributionDialogManager;
import org.maplibre.android.maps.MapView;
import org.maplibre.android.maps.UiSettings;

MapView mapView = findViewById(R.id.mapView);

mapView.getMapAsync(mapLibreMap -> {
    UiSettings uiSettings = mapLibreMap.getUiSettings();

    // Replace the default attribution dialog behavior
    uiSettings.setAttributionDialogManager(
        new AttributionDialogManager(mapView.getContext(), mapLibreMap) {
            @Override
            public void onClick(View view) {
                // Example: open your own "Full attributions" screen
                Intent intent = new Intent(view.getContext(), FullAttributionsActivity.class);
                view.getContext().startActivity(intent);

                // If you want to keep the default dialog instead, call:
                // super.onClick(view);
            }
        }
    );
});
```

## Notes

- Use `super.onClick(view)` if you want to keep the default attribution dialog and only add extra actions.
- `FullAttributionsActivity` is just an example placeholder — implement it to show your attribution/license content.

## References in this repository

- `platform/android/MapLibreAndroid/src/main/java/org/maplibre/android/maps/UiSettings.java`
- `platform/android/MapLibreAndroid/src/main/java/org/maplibre/android/maps/MapView.java`
- `platform/android/MapLibreAndroid/src/main/java/org/maplibre/android/maps/AttributionDialogManager.java`
- `platform/android/MapLibreAndroid/src/test/java/org/maplibre/android/maps/AttributionDialogManagerTest.kt`
