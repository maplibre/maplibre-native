package org.maplibre.android.utils

import android.content.Context
import android.os.Bundle
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.maps.MapFragment
import org.maplibre.android.maps.MapLibreMapOptions
import org.maplibre.android.maps.SupportMapFragment

/**
 * MapFragment utility class.
 *
 * Used to extract duplicate code between [MapFragment] and [SupportMapFragment].
 */
object MapFragmentUtils {
    /**
     * Convert MapLibreMapOptions to a bundle of fragment arguments.
     *
     * @param options The MapLibreMapOptions to convert
     * @return a bundle of converted fragment arguments
     */
    @JvmStatic
    fun createFragmentArgs(options: MapLibreMapOptions?): Bundle {
        val bundle = Bundle()
        bundle.putParcelable(MapLibreConstants.FRAG_ARG_MAPLIBREMAPOPTIONS, options)
        return bundle
    }

    /**
     * Convert a bundle of fragment arguments to MapLibreMapOptions.
     *
     * @param context The context of the activity hosting the fragment
     * @param args    The fragment arguments
     * @return converted MapLibreMapOptions
     */
    @JvmStatic
    @Suppress("DEPRECATION")
    fun resolveArgs(
        context: Context,
        args: Bundle?,
    ): MapLibreMapOptions? =
        if (args != null && args.containsKey(MapLibreConstants.FRAG_ARG_MAPLIBREMAPOPTIONS)) {
            args.getParcelable(MapLibreConstants.FRAG_ARG_MAPLIBREMAPOPTIONS)
        } else {
            // load default options
            MapLibreMapOptions.createFromAttributes(context)
        }
}
