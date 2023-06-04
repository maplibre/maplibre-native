package org.maplibre.android.location.engine;

import android.content.Intent;
import android.location.Location;
import android.location.LocationManager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * A wrapper class representing location result from the location engine.
 * <p>
 * TODO: Override default equals(), hashCode() and toString()
 *
 * @since 1.0.0
 */
public final class LocationEngineResult {
  private final List<Location> locations;

  private LocationEngineResult(List<Location> locations) {
    this.locations = Collections.unmodifiableList(locations);
  }

  /**
   * Creates {@link LocationEngineResult} instance for location.
   *
   * @param location default location added to the result.
   * @return instance of the new location result.
   * @since 1.0.0
   */
  @NonNull
  public static LocationEngineResult create(@Nullable Location location) {
    List<Location> locations = new ArrayList<>();
    if (location != null) {
      locations.add(location);
    }
    return new LocationEngineResult(locations);
  }

  /**
   * Creates {@link LocationEngineResult} instance for given list of locations.
   *
   * @param locations list of locations.
   * @return instance of the new location result.
   * @since 1.0.0
   */
  @NonNull
  public static LocationEngineResult create(@Nullable List<Location> locations) {
    if (locations != null) {
      List<Location> locationsList = new ArrayList<>(locations);
      locationsList.removeAll(Collections.singleton(null));
      return new LocationEngineResult(locationsList);
    }

    return new LocationEngineResult(Collections.<Location>emptyList());
  }

  /**
   * Returns most recent location available in this result.
   *
   * @return the most recent location {@link Location} or null.
   * @since 1.0.0
   */
  @Nullable
  public Location getLastLocation() {
    return locations.isEmpty() ? null : locations.get(0);
  }

  /**
   * Returns locations computed, ordered from oldest to newest.
   *
   * @return ordered list of locations.
   * @since 1.0.0
   */
  public List<Location> getLocations() {
    return Collections.unmodifiableList(locations);
  }

  /**
   * Extracts location result from intent object
   *
   * @param intent valid intent object
   * @return location result.
   * @since 1.1.0
   */
  @Nullable
  public static LocationEngineResult extractResult(Intent intent) {
    return extractAndroidResult(intent);
  }

  private static LocationEngineResult extractAndroidResult(Intent intent) {
    return !hasResult(intent) ? null :
      LocationEngineResult.create((Location) intent.getExtras()
        .getParcelable(LocationManager.KEY_LOCATION_CHANGED));
  }

  private static boolean hasResult(Intent intent) {
    return intent != null && intent.hasExtra(LocationManager.KEY_LOCATION_CHANGED);
  }
}
