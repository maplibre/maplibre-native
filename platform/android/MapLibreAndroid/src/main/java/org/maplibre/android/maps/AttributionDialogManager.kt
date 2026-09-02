package org.maplibre.android.maps

import android.app.Activity
import android.app.AlertDialog
import android.content.ActivityNotFoundException
import android.content.Context
import android.content.DialogInterface
import android.content.Intent
import android.net.Uri
import android.view.View
import android.widget.ArrayAdapter
import android.widget.Toast
import org.maplibre.android.MapLibre
import org.maplibre.android.MapStrictMode
import org.maplibre.android.R
import org.maplibre.android.attribution.Attribution
import org.maplibre.android.attribution.AttributionParser
import java.lang.ref.WeakReference
import java.util.Locale
import java.util.regex.Pattern

/**
 * Responsible for managing attribution interactions on the map.
 *
 * When the user clicks the attribution icon, [AttributionDialogManager.onClick] will be invoked.
 * An attribution dialog will be shown to the user with contents based on the attributions found in the map style.
 */
open class AttributionDialogManager(
    private val context: Context,
    private val maplibreMap: MapLibreMap,
) : View.OnClickListener,
    DialogInterface.OnClickListener {
    private var attributionSet: Set<Attribution> = emptySet()
    private var dialog: AlertDialog? = null

    // Called when someone presses the attribution icon on the map
    override fun onClick(view: View) {
        attributionSet = AttributionBuilder(maplibreMap, view.context).build()

        val isActivityFinishing = (context as? Activity)?.isFinishing ?: false

        // check is hosting activity isn't finishing
        // https://github.com/mapbox/mapbox-gl-native/issues/11238
        if (!isActivityFinishing) {
            showAttributionDialog(getAttributionTitles())
        }
    }

    protected open fun showAttributionDialog(attributionTitles: Array<String>) {
        val builder = AlertDialog.Builder(context)
        builder.setTitle(R.string.maplibre_attributionsDialogTitle)
        builder.setAdapter(ArrayAdapter(context, R.layout.maplibre_attribution_list_item, attributionTitles), this)
        dialog = builder.show()
    }

    private fun getAttributionTitles(): Array<String> = attributionSet.map { it.title }.toTypedArray()

    // Called when someone selects an attribution from the dialog
    override fun onClick(
        dialog: DialogInterface,
        which: Int,
    ) {
        showMapAttributionWebPage(which)
    }

    fun onStop() {
        dialog?.takeIf { it.isShowing }?.dismiss()
    }

    private fun showMapAttributionWebPage(which: Int) {
        val attributions = attributionSet.toTypedArray()
        var url = attributions[which].url
        if (url.contains(MAP_FEEDBACK_URL_OLD) || url.contains(MAP_FEEDBACK_URL)) {
            url = buildMapFeedbackMapUrl(MapLibre.getApiKey())
        }
        showWebPage(url)
    }

    internal fun buildMapFeedbackMapUrl(apiKey: String?): String {
        // TODO Add Android Maps SDK version to the query parameter, currently the version API is not available.
        // TODO Keep track of this issue at [#15632](https://github.com/mapbox/mapbox-gl-native/issues/15632)

        val builder = Uri.parse(MAP_FEEDBACK_URL).buildUpon()

        val cameraPosition = maplibreMap.cameraPosition
        builder.encodedFragment(
            String.format(
                Locale.getDefault(),
                MAP_FEEDBACK_URL_LOCATION_FRAGMENT_FORMAT,
                cameraPosition.target!!.longitude,
                cameraPosition.target!!.latitude,
                cameraPosition.zoom,
                cameraPosition.bearing,
                cameraPosition.tilt.toInt(),
            ),
        )

        val packageName: String? = context.applicationContext.packageName
        if (packageName != null) {
            builder.appendQueryParameter("referrer", packageName)
        }

        if (apiKey != null) {
            // TODO:PP
            builder.appendQueryParameter("access_token", apiKey)
        }

        val style = maplibreMap.style
        if (style != null) {
            val matcher = Pattern.compile(MAP_FEEDBACK_STYLE_URI_REGEX).matcher(style.uri)
            if (matcher.find()) {
                builder
                    .appendQueryParameter("owner", matcher.group(2))
                    .appendQueryParameter("id", matcher.group(3))
            }
        }

        return builder.build().toString()
    }

    private fun showWebPage(url: String) {
        try {
            val intent = Intent(Intent.ACTION_VIEW)
            intent.data = Uri.parse(url)
            context.startActivity(intent)
        } catch (exception: ActivityNotFoundException) {
            // explicitly handling if the device hasn't have a web browser installed. #8899
            Toast.makeText(context, R.string.maplibre_attributionErrorNoBrowser, Toast.LENGTH_LONG).show()
            MapStrictMode.strictModeViolation(exception)
        }
    }

    private class AttributionBuilder(
        private val maplibreMap: MapLibreMap,
        context: Context,
    ) {
        private val context: WeakReference<Context> = WeakReference(context)

        fun build(): Set<Attribution> {
            val context = this.context.get() ?: return emptySet()

            val attributions = mutableListOf<String>()
            maplibreMap.style?.let { style ->
                for (source in style.sources) {
                    val attribution = source.attribution
                    if (attribution.isNotEmpty()) {
                        attributions.add(attribution)
                    }
                }
            }

            return AttributionParser
                .Options(context)
                .withCopyrightSign(true)
                .withImproveMap(true)
                .withAttributionData(*attributions.toTypedArray())
                .build()
                .attributions
        }
    }

    private companion object {
        const val MAP_FEEDBACK_URL = "https://apps.mapbox.com/feedback"
        const val MAP_FEEDBACK_URL_OLD = "https://www.mapbox.com/map-feedback"
        const val MAP_FEEDBACK_URL_LOCATION_FRAGMENT_FORMAT = "/%f/%f/%f/%f/%d"
        const val MAP_FEEDBACK_STYLE_URI_REGEX = "^(.*://[^:^/]*)/(.*)/(.*)"
    }
}
