package com.mapbox.mapboxsdk.testapp.activity.infowindow

import android.graphics.Color
import android.os.Bundle
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.res.ResourcesCompat
import com.mapbox.mapboxsdk.annotations.Marker
import com.mapbox.mapboxsdk.annotations.MarkerOptions
import com.mapbox.mapboxsdk.camera.CameraUpdateFactory
import com.mapbox.mapboxsdk.geometry.LatLng
import com.mapbox.mapboxsdk.maps.MapView
import com.mapbox.mapboxsdk.maps.MapboxMap
import com.mapbox.mapboxsdk.maps.MapboxMap.InfoWindowAdapter
import com.mapbox.mapboxsdk.maps.MapboxMap.OnMapClickListener
import com.mapbox.mapboxsdk.maps.OnMapReadyCallback
import com.mapbox.mapboxsdk.maps.Style
import com.mapbox.mapboxsdk.testapp.R
import com.mapbox.mapboxsdk.testapp.utils.IconUtils
import java.util.*

/**
 * Test activity showcasing how to dynamically update InfoWindow when Using an MapboxMap.InfoWindowAdapter.
 */
class DynamicInfoWindowAdapterActivity : AppCompatActivity(), OnMapReadyCallback {
    private lateinit var mapboxMap: MapboxMap
    private lateinit var mapView: MapView
    private var marker: Marker? = null
    private val mapClickListener = OnMapClickListener { point ->
        if (marker == null) {
            return@OnMapClickListener false
        }

        // Distance from click to marker
        val distanceKm = marker!!.position.distanceTo(point) / 1000

        // Get the info window
        val infoWindow = marker!!.infoWindow

        // Get the view from the info window
        if (infoWindow != null && infoWindow.view != null) {
            // Set the new text on the text view in the info window
            val textView = infoWindow.view as TextView?
            textView!!.text = String.format(Locale.getDefault(), "%.2fkm", distanceKm)
            // Update the info window position (as the text length changes)
            textView.post { infoWindow.update() }
        }
        true
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_infowindow_adapter)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(this)
    }

    override fun onMapReady(map: MapboxMap) {
        mapboxMap = map
        map.setStyle(Style.getPredefinedStyle("Streets"))

        // Add info window adapter
        addCustomInfoWindowAdapter(mapboxMap!!)

        // Keep info windows open on click
        mapboxMap.uiSettings.isDeselectMarkersOnTap = false

        // Add a marker
        marker = addMarker(mapboxMap!!)
        mapboxMap.selectMarker(marker!!)

        // On map click, change the info window contents
        mapboxMap.addOnMapClickListener(mapClickListener)

        // Focus on Paris
        mapboxMap.animateCamera(CameraUpdateFactory.newLatLng(PARIS))
    }

    private fun addMarker(mapboxMap: MapboxMap): Marker {
        return mapboxMap.addMarker(
            MarkerOptions()
                .position(PARIS)
                .icon(
                    IconUtils.drawableToIcon(
                        this,
                        R.drawable.ic_location_city,
                        ResourcesCompat.getColor(resources, R.color.maplibre_blue, theme)
                    )
                )
        )
    }

    private fun addCustomInfoWindowAdapter(mapboxMap: MapboxMap) {
        val padding = resources.getDimension(R.dimen.attr_margin).toInt()
        mapboxMap.infoWindowAdapter = InfoWindowAdapter { marker: Marker ->
            val textView = TextView(this@DynamicInfoWindowAdapterActivity)
            textView.text = marker.title
            textView.setBackgroundColor(Color.WHITE)
            textView.setText(R.string.action_calculate_distance)
            textView.setPadding(padding, padding, padding, padding)
            textView
        }
    }

    override fun onStart() {
        super.onStart()
        mapView.onStart()
    }

    override fun onResume() {
        super.onResume()
        mapView.onResume()
    }

    override fun onPause() {
        super.onPause()
        mapView.onPause()
    }

    override fun onStop() {
        super.onStop()
        mapView.onStop()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }

    override fun onDestroy() {
        super.onDestroy()
        if (mapboxMap != null) {
            mapboxMap.removeOnMapClickListener(mapClickListener)
        }
        mapView.onDestroy()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    companion object {
        private val PARIS = LatLng(48.864716, 2.349014)
    }
}
