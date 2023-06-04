package org.maplibre.android.testapp.utils

import android.widget.TextView
import org.maplibre.android.maps.MapboxMap
import org.maplibre.android.maps.MapboxMap.OnCameraIdleListener
import org.maplibre.android.testapp.R

class IdleZoomListener(private val mapboxMap: MapboxMap, private val textView: TextView) :
    OnCameraIdleListener {
    override fun onCameraIdle() {
        val context = textView.context
        val position = mapboxMap.cameraPosition
        textView.text = String.format(context.getString(R.string.debug_zoom), position.zoom)
    }
}
