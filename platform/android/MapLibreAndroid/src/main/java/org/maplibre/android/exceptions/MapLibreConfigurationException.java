package org.maplibre.android.exceptions;

import org.maplibre.android.MapLibre;
import org.maplibre.android.WellKnownTileServer;

import android.content.Context;

import androidx.annotation.NonNull;

/**
 * A MapboxConfigurationException is thrown by MapLibreMap when the SDK hasn't been properly initialised.
 * <p>
 * This occurs either when {@link MapLibre} is not correctly initialised or the provided apiKey
 * through {@link MapLibre#getInstance(Context, String, WellKnownTileServer)} isn't valid.
 * </p>
 *
 * @see MapLibre#getInstance(Context, String,  WellKnownTileServer)
 */
public class MapLibreConfigurationException extends RuntimeException {

  /**
   * Creates a MapLibre configuration exception thrown by MapLibreMap when the SDK hasn't been properly initialised.
   */
  public MapLibreConfigurationException() {
    super("\nUsing MapView requires calling MapLibre.getInstance(Context context, String apiKey, "
            + "WellKnownTileServer wellKnownTileServer) before inflating or creating the view.");
  }

  /**
   * Creates a MapLibre configuration exception thrown by MapLibreMap when the SDK hasn't been properly initialised.
   */
  public MapLibreConfigurationException(@NonNull String message) {
    super(message);
  }
}
