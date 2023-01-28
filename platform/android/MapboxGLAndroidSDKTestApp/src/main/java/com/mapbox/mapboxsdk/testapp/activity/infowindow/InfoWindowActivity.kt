package com.mapbox.mapboxsdk.testapp.activity.infowindow

import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.view.View
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.mapbox.mapboxsdk.annotations.Marker
import com.mapbox.mapboxsdk.annotations.MarkerOptions
import com.mapbox.mapboxsdk.geometry.LatLng
import com.mapbox.mapboxsdk.maps.MapView
import com.mapbox.mapboxsdk.maps.MapboxMap
import com.mapbox.mapboxsdk.maps.MapboxMap.OnInfoWindowClickListener
import com.mapbox.mapboxsdk.maps.MapboxMap.OnInfoWindowCloseListener
import com.mapbox.mapboxsdk.maps.MapboxMap.OnInfoWindowLongClickListener
import com.mapbox.mapboxsdk.maps.MapboxMap.OnMapLongClickListener
import com.mapbox.mapboxsdk.maps.OnMapReadyCallback
import com.mapbox.mapboxsdk.maps.Style
import com.mapbox.mapboxsdk.testapp.R
import java.text.DecimalFormat

/**
 * Test activity showcasing using the InfoWindow API above Washington D.C.
 *
 *
 * Allows to test mulitple concurrently open InfoWindows.
 *
 */
class InfoWindowActivity :
    AppCompatActivity(),
    OnMapReadyCallback,
    OnInfoWindowCloseListener,
    OnInfoWindowClickListener,
    OnInfoWindowLongClickListener {
    private lateinit var mapboxMap: MapboxMap
    private lateinit var mapView: MapView
    private var customMarker: Marker? = null
    private val mapLongClickListener = OnMapLongClickListener { point ->
        if (customMarker != null) {
            // Remove previous added marker
            mapboxMap.removeAnnotation(customMarker!!)
            customMarker = null
        }

        // Add marker on long click location with default marker image
        customMarker = mapboxMap.addMarker(
            MarkerOptions()
                .title("Custom Marker")
                .snippet(
                    DecimalFormat("#.#####").format(point.latitude) + ", " +
                        DecimalFormat("#.#####").format(point.longitude)
                )
                .position(point)
        )
        true
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_infowindow)
        mapView = findViewById<View>(R.id.mapView) as MapView
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(this)
    }

    override fun onMapReady(mapboxMap: MapboxMap) {
        this.mapboxMap = mapboxMap
        mapboxMap.setStyle(Style.getPredefinedStyle("Streets")) { style: Style? ->
            addMarkers()
            addInfoWindowListeners()
        }
    }

    private fun addMarkers() {
        mapboxMap.addMarker(
            MarkerOptions()
                .title("Intersection")
                .snippet("H St NW with 15th St NW")
                .position(LatLng(38.9002073, -77.03364419))
        )
        mapboxMap.addMarker(
            MarkerOptions().title("Intersection")
                .snippet("E St NW with 17th St NW")
                .position(LatLng(38.8954236, -77.0394623))
        )
        mapboxMap.addMarker(
            MarkerOptions().title("The Ellipse").position(LatLng(38.89393, -77.03654))
        )
        mapboxMap.addMarker(MarkerOptions().position(LatLng(38.89596, -77.03434)))
        mapboxMap.addMarker(
            MarkerOptions().snippet("Lafayette Square").position(LatLng(38.89949, -77.03656))
        )
        val marker = mapboxMap.addMarker(
            MarkerOptions()
                .title("White House")
                .snippet(
                    "The official residence and principal workplace of the President of the United States, " +
                        "located at 1600 Pennsylvania Avenue NW in Washington, D.C. It has been the residence of every" +
                        "U.S. president since John Adams in 1800."
                )
                .position(LatLng(38.897705003219784, -77.03655168667463))
        )

        // open InfoWindow at startup
        mapboxMap.selectMarker(marker)
    }

    private fun addInfoWindowListeners() {
        mapboxMap.onInfoWindowCloseListener = this
        mapboxMap.addOnMapLongClickListener(mapLongClickListener)
        mapboxMap.onInfoWindowClickListener = this
        mapboxMap.onInfoWindowLongClickListener = this
    }

    private fun toggleConcurrentInfoWindow(allowConcurrentInfoWindow: Boolean) {
        mapboxMap.deselectMarkers()
        mapboxMap.isAllowConcurrentMultipleOpenInfoWindows = allowConcurrentInfoWindow
    }

    private fun toggleDeselectMarkersOnTap(deselectMarkersOnTap: Boolean) {
        mapboxMap.uiSettings.isDeselectMarkersOnTap = deselectMarkersOnTap
    }

    override fun onInfoWindowClick(marker: Marker): Boolean {
        Toast.makeText(applicationContext, "OnClick: " + marker.title, Toast.LENGTH_LONG).show()
        // returning true will leave the info window open
        return false
    }

    override fun onInfoWindowClose(marker: Marker) {
        Toast.makeText(applicationContext, "OnClose: " + marker.title, Toast.LENGTH_LONG).show()
    }

    override fun onInfoWindowLongClick(marker: Marker) {
        Toast.makeText(applicationContext, "OnLongClick: " + marker.title, Toast.LENGTH_LONG).show()
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
            mapboxMap.removeOnMapLongClickListener(mapLongClickListener)
        }
        mapView.onDestroy()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.menu_infowindow, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.action_toggle_concurrent_infowindow -> {
                toggleConcurrentInfoWindow(!item.isChecked)
                item.isChecked = !item.isChecked
                true
            }
            R.id.action_toggle_deselect_markers_on_tap -> {
                toggleDeselectMarkersOnTap(!item.isChecked)
                item.isChecked = !item.isChecked
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }
}
