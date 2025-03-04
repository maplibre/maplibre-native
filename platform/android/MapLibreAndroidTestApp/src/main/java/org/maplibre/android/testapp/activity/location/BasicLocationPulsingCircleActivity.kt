package org.maplibre.android.testapp.activity.location

import android.annotation.SuppressLint
import android.location.Location
import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.location.LocationComponent
import org.maplibre.android.location.LocationComponentActivationOptions
import org.maplibre.android.location.LocationComponentOptions
import org.maplibre.android.location.engine.LocationEngineRequest
import org.maplibre.android.location.modes.CameraMode
import org.maplibre.android.location.permissions.PermissionsListener
import org.maplibre.android.location.permissions.PermissionsManager
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.OnMapReadyCallback
import org.maplibre.android.maps.Style
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles

// # --8<-- [start:top]
/**
 * This activity shows a basic usage of the LocationComponent's pulsing circle. There's no
 * customization of the pulsing circle's color, radius, speed, etc.
 */
class BasicLocationPulsingCircleActivity : AppCompatActivity(), OnMapReadyCallback {
    private var lastLocation: Location? = null
    private lateinit var mapView: MapView
    private var permissionsManager: PermissionsManager? = null
    private var locationComponent: LocationComponent? = null
    private lateinit var maplibreMap: MapLibreMap

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_location_layer_basic_pulsing_circle)
        mapView = findViewById(R.id.mapView)
        if (savedInstanceState != null) {
            lastLocation = savedInstanceState.getParcelable(SAVED_STATE_LOCATION, Location::class.java)
        }
        mapView.onCreate(savedInstanceState)
        checkPermissions()
    }
    // # --8<-- [end:top]

    // # --8<-- [start:onMapReady]
    @SuppressLint("MissingPermission")
    override fun onMapReady(maplibreMap: MapLibreMap) {
        this.maplibreMap = maplibreMap
        maplibreMap.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets")) { style: Style ->
            locationComponent = maplibreMap.locationComponent
            val locationComponentOptions =
                LocationComponentOptions.builder(this@BasicLocationPulsingCircleActivity)
                    .pulseEnabled(true)
                    .build()
            val locationComponentActivationOptions =
                buildLocationComponentActivationOptions(style, locationComponentOptions)
            locationComponent!!.activateLocationComponent(locationComponentActivationOptions)
            locationComponent!!.isLocationComponentEnabled = true
            locationComponent!!.cameraMode = CameraMode.TRACKING
            locationComponent!!.forceLocationUpdate(lastLocation)
        }
    }
    // # --8<-- [end:onMapReady]

    // # --8<-- [start:LocationComponentActivationOptions]
    private fun buildLocationComponentActivationOptions(
        style: Style,
        locationComponentOptions: LocationComponentOptions
    ): LocationComponentActivationOptions {
        return LocationComponentActivationOptions
            .builder(this, style)
            .locationComponentOptions(locationComponentOptions)
            .useDefaultLocationEngine(true)
            .locationEngineRequest(
                LocationEngineRequest.Builder(750)
                    .setFastestInterval(750)
                    .setPriority(LocationEngineRequest.PRIORITY_HIGH_ACCURACY)
                    .build()
            )
            .build()
    }
    // # --8<-- [end:LocationComponentActivationOptions]

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.menu_pulsing_location_mode, menu)
        return true
    }

    @SuppressLint("MissingPermission")
    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        if (locationComponent == null) {
            return super.onOptionsItemSelected(item)
        }
        val id = item.itemId
        when (id) {
            R.id.action_map_style_change -> {
                loadNewStyle()
                return true
            }
            R.id.action_component_disable -> {
                locationComponent!!.isLocationComponentEnabled = false
                return true
            }
            R.id.action_component_enabled -> {
                locationComponent!!.isLocationComponentEnabled = true
                return true
            }
            R.id.action_stop_pulsing -> {
                locationComponent!!.applyStyle(
                    LocationComponentOptions.builder(
                        this@BasicLocationPulsingCircleActivity
                    )
                        .pulseEnabled(false)
                        .build()
                )
                return true
            }
            R.id.action_start_pulsing -> {
                locationComponent!!.applyStyle(
                    LocationComponentOptions.builder(
                        this@BasicLocationPulsingCircleActivity
                    )
                        .pulseEnabled(true)
                        .build()
                )
                return true
            }
            else -> return super.onOptionsItemSelected(item)
        }
    }

    private fun loadNewStyle() {
        maplibreMap.setStyle(Style.Builder().fromUri(Utils.nextStyle()))
    }

    // # --8<-- [start:permission]
    private fun checkPermissions() {
        if (PermissionsManager.areLocationPermissionsGranted(this)) {
            mapView.getMapAsync(this)
        } else {
            permissionsManager = PermissionsManager(object : PermissionsListener {
                override fun onExplanationNeeded(permissionsToExplain: List<String>) {
                    Toast.makeText(
                        this@BasicLocationPulsingCircleActivity,
                        "You need to accept location permissions.",
                        Toast.LENGTH_SHORT
                    ).show()
                }

                override fun onPermissionResult(granted: Boolean) {
                    if (granted) {
                        mapView.getMapAsync(this@BasicLocationPulsingCircleActivity)
                    } else {
                        finish()
                    }
                }
            })
            permissionsManager!!.requestLocationPermissions(this)
        }
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        permissionsManager!!.onRequestPermissionsResult(requestCode, permissions, grantResults)
    }
    // # --8<-- [end:permission]

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

    @SuppressLint("MissingPermission")
    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
        if (locationComponent != null) {
            outState.putParcelable(SAVED_STATE_LOCATION, locationComponent!!.lastKnownLocation)
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        mapView.onDestroy()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    companion object {
        private const val SAVED_STATE_LOCATION = "saved_state_location"
    }
}
