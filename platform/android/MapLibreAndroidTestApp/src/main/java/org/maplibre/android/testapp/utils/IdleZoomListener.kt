package org.maplibre.android.testapp.utils

import android.widget.TextView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapLibreMap.OnCameraIdleListener
import org.maplibre.android.testapp.R

class IdleZoomListener(private val maplibreMap: MapLibreMap, private val textView: TextView) :
    OnCameraIdleListener {
    override fun onCameraIdle() {
        val context = textView.context
        val position = maplibreMap.cameraPosition
        textView.text = String.format(context.getString(R.string.debug_zoom), position.zoom)
    }
}
