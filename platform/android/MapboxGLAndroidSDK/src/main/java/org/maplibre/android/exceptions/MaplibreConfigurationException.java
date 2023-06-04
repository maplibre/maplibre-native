package org.maplibre.android.exceptions;

import org.maplibre.android.Maplibre;
import org.maplibre.android.WellKnownTileServer;

import android.content.Context;

import androidx.annotation.NonNull;

/**
 * A MapboxConfigurationException is thrown by MaplibreMap when the SDK hasn't been properly initialised.
 * <p>
 * This occurs either when {@link Maplibre} is not correctly initialised or the provided apiKey
 * through {@link Maplibre#getInstance(Context, String, WellKnownTileServer)} isn't valid.
 * </p>
 *
 * @see Maplibre#getInstance(Context, String,  WellKnownTileServer)
 */
public class MaplibreConfigurationException extends RuntimeException {

  /**
   * Creates a Maplibre configuration exception thrown by MapboxMap when the SDK hasn't been properly initialised.
   */
  public MaplibreConfigurationException() {
    super("\nUsing MapView requires calling Maplibre.getInstance(Context context, String apiKey, "
            + "WellKnownTileServer wellKnownTileServer) before inflating or creating the view.");
  }

  /**
   * Creates a Maplibre configuration exception thrown by MapboxMap when the SDK hasn't been properly initialised.
   */
  public MaplibreConfigurationException(@NonNull String message) {
    super(message);
  }
}
