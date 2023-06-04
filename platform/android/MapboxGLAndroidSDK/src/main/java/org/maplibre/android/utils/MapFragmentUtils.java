package org.maplibre.android.utils;

import android.content.Context;
import android.os.Bundle;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.maplibre.android.constants.MaplibreConstants;
import org.maplibre.android.maps.MapFragment;
import org.maplibre.android.maps.SupportMapFragment;
import org.maplibre.android.maps.MaplibreMapOptions;

/**
 * MapFragment utility class.
 * <p>
 * Used to extract duplicate code between {@link MapFragment} and
 * {@link SupportMapFragment}.
 * </p>
 */
public class MapFragmentUtils {

  /**
   * Convert MapboxMapOptions to a bundle of fragment arguments.
   *
   * @param options The MapboxMapOptions to convert
   * @return a bundle of converted fragment arguments
   */
  @NonNull
  public static Bundle createFragmentArgs(MaplibreMapOptions options) {
    Bundle bundle = new Bundle();
    bundle.putParcelable(MaplibreConstants.FRAG_ARG_MAPLIBREMAPOPTIONS, options);
    return bundle;
  }

  /**
   * Convert a bundle of fragment arguments to MapboxMapOptions.
   *
   * @param context The context of the activity hosting the fragment
   * @param args    The fragment arguments
   * @return converted MapboxMapOptions
   */
  @Nullable
  public static MaplibreMapOptions resolveArgs(@NonNull Context context, @Nullable Bundle args) {
    MaplibreMapOptions options;
    if (args != null && args.containsKey(MaplibreConstants.FRAG_ARG_MAPLIBREMAPOPTIONS)) {
      options = args.getParcelable(MaplibreConstants.FRAG_ARG_MAPLIBREMAPOPTIONS);
    } else {
      // load default options
      options = MaplibreMapOptions.createFromAttributes(context);
    }
    return options;
  }
}
