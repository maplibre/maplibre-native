package org.maplibre.android.utils;

import android.content.Context;
import android.os.Bundle;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.maplibre.android.constants.MapLibreConstants;
import org.maplibre.android.maps.MapFragment;
import org.maplibre.android.maps.SupportMapFragment;
import org.maplibre.android.maps.MapLibreMapOptions;

/**
 * MapFragment utility class.
 * <p>
 * Used to extract duplicate code between {@link MapFragment} and
 * {@link SupportMapFragment}.
 * </p>
 */
public class MapFragmentUtils {

  /**
   * Convert MapLibreMapOptions to a bundle of fragment arguments.
   *
   * @param options The MapLibreMapOptions to convert
   * @return a bundle of converted fragment arguments
   */
  @NonNull
  public static Bundle createFragmentArgs(MapLibreMapOptions options) {
    Bundle bundle = new Bundle();
    bundle.putParcelable(MapLibreConstants.FRAG_ARG_MAPLIBREMAPOPTIONS, options);
    return bundle;
  }

  /**
   * Convert a bundle of fragment arguments to MapLibreMapOptions.
   *
   * @param context The context of the activity hosting the fragment
   * @param args    The fragment arguments
   * @return converted MapLibreMapOptions
   */
  @Nullable
  public static MapLibreMapOptions resolveArgs(@NonNull Context context, @Nullable Bundle args) {
    MapLibreMapOptions options;
    if (args != null && args.containsKey(MapLibreConstants.FRAG_ARG_MAPLIBREMAPOPTIONS)) {
      options = args.getParcelable(MapLibreConstants.FRAG_ARG_MAPLIBREMAPOPTIONS);
    } else {
      // load default options
      options = MapLibreMapOptions.createFromAttributes(context);
    }
    return options;
  }
}
