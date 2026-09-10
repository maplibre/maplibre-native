package org.maplibre.android.exceptions

import org.maplibre.android.MapLibre

/**
 * A MapboxConfigurationException is thrown by MapLibreMap when the SDK hasn't been properly initialised.
 *
 * This occurs either when [MapLibre] is not correctly initialised or the provided apiKey
 * through [MapLibre.getInstance] isn't valid.
 *
 * @see MapLibre.getInstance
 */
class MapLibreConfigurationException : RuntimeException {
    /**
     * Creates a MapLibre configuration exception thrown by MapLibreMap when the SDK hasn't been properly initialised.
     */
    constructor() : super(
        "\nUsing MapView requires calling MapLibre.getInstance(Context context, String apiKey, " +
            "WellKnownTileServer wellKnownTileServer) before inflating or creating the view.",
    )

    /**
     * Creates a MapLibre configuration exception thrown by MapLibreMap when the SDK hasn't been properly initialised.
     */
    constructor(message: String) : super(message)
}
