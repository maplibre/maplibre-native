package org.maplibre.android.testapp.activity.location

import android.annotation.SuppressLint
import android.content.res.Configuration
import android.graphics.RectF
import android.location.Location
import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.view.View
import android.widget.AdapterView
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.ListPopupWindow
import org.maplibre.android.location.LocationComponent
import org.maplibre.android.location.LocationComponentActivationOptions
import org.maplibre.android.location.LocationComponentOptions
import org.maplibre.android.location.OnCameraTrackingChangedListener
import org.maplibre.android.location.OnLocationCameraTransitionListener
import org.maplibre.android.location.OnLocationClickListener
import org.maplibre.android.location.engine.LocationEngineRequest
import org.maplibre.android.location.modes.CameraMode
import org.maplibre.android.location.modes.RenderMode
import org.maplibre.android.location.permissions.PermissionsListener
import org.maplibre.android.location.permissions.PermissionsManager
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapLibreMap.CancelableCallback
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.OnMapReadyCallback
import org.maplibre.android.maps.Style
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import java.util.Random

class LocationModesActivity :
    AppCompatActivity(),
    OnMapReadyCallback,
    OnLocationClickListener,
    OnCameraTrackingChangedListener {
    private lateinit var mapView: MapView
    private lateinit var locationModeBtn: Button
    private lateinit var locationTrackingBtn: Button
    private var protectedGestureArea: View? = null
    private var permissionsManager: PermissionsManager? = null
    private var locationComponent: LocationComponent? = null
    private lateinit var maplibreMap: MapLibreMap
    private var defaultStyle = false

    @CameraMode.Mode
    private var cameraMode = CameraMode.TRACKING

    @RenderMode.Mode
    private var renderMode = RenderMode.NORMAL
    private var lastLocation: Location? = null
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_location_layer_mode)
        mapView = findViewById(R.id.mapView)
        protectedGestureArea = findViewById(R.id.view_protected_gesture_area)
        locationModeBtn = findViewById(R.id.button_location_mode)
        locationModeBtn.setOnClickListener(
            View.OnClickListener setOnClickListener@{ v: View? ->
                if (locationComponent == null) {
                    return@setOnClickListener
                }
                showModeListDialog()
            }
        )
        locationTrackingBtn = findViewById(R.id.button_location_tracking)
        locationTrackingBtn.setOnClickListener(
            View.OnClickListener setOnClickListener@{ v: View? ->
                if (locationComponent == null) {
                    return@setOnClickListener
                }
                showTrackingListDialog()
            }
        )
        if (savedInstanceState != null) {
            cameraMode = savedInstanceState.getInt(SAVED_STATE_CAMERA)
            renderMode = savedInstanceState.getInt(SAVED_STATE_RENDER)
            lastLocation = savedInstanceState.getParcelable(SAVED_STATE_LOCATION)
        }
        mapView.onCreate(savedInstanceState)
        if (PermissionsManager.areLocationPermissionsGranted(this)) {
            mapView.getMapAsync(this)
        } else {
            permissionsManager = PermissionsManager(object : PermissionsListener {
                override fun onExplanationNeeded(permissionsToExplain: List<String>) {
                    Toast.makeText(
                        this@LocationModesActivity,
                        "You need to accept location permissions.",
                        Toast.LENGTH_SHORT
                    ).show()
                }

                override fun onPermissionResult(granted: Boolean) {
                    if (granted) {
                        mapView.getMapAsync(this@LocationModesActivity)
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

    @SuppressLint("MissingPermission")
    override fun onMapReady(maplibreMap: MapLibreMap) {
        this.maplibreMap = maplibreMap
        maplibreMap.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets")) { style: Style? ->
            locationComponent = maplibreMap.locationComponent
            locationComponent!!.activateLocationComponent(
                LocationComponentActivationOptions
                    .builder(this, style!!)
                    .useSpecializedLocationLayer(true)
                    .useDefaultLocationEngine(true)
                    .locationEngineRequest(
                        LocationEngineRequest.Builder(750)
                            .setFastestInterval(750)
                            .setPriority(LocationEngineRequest.PRIORITY_HIGH_ACCURACY)
                            .build()
                    )
                    .build()
            )
            toggleStyle()
            locationComponent!!.isLocationComponentEnabled = true
            locationComponent!!.addOnLocationClickListener(this)
            locationComponent!!.addOnCameraTrackingChangedListener(this)
            locationComponent!!.cameraMode = cameraMode
            setRendererMode(renderMode)
            locationComponent!!.forceLocationUpdate(lastLocation)
        }
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.menu_location_mode, menu)
        return true
    }

    @SuppressLint("MissingPermission")
    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        if (locationComponent == null) {
            return super.onOptionsItemSelected(item)
        }
        val id = item.itemId
        if (id == R.id.action_style_change) {
            toggleStyle()
            return true
        } else if (id == R.id.action_map_style_change) {
            toggleMapStyle()
            return true
        } else if (id == R.id.action_component_disable) {
            locationComponent!!.isLocationComponentEnabled = false
            return true
        } else if (id == R.id.action_component_enabled) {
            locationComponent!!.isLocationComponentEnabled = true
            return true
        } else if (id == R.id.action_gestures_management_disabled) {
            disableGesturesManagement()
            return true
        } else if (id == R.id.action_gestures_management_enabled) {
            enableGesturesManagement()
            return true
        } else if (id == R.id.action_component_throttling_enabled) {
            locationComponent!!.setMaxAnimationFps(5)
        } else if (id == R.id.action_component_throttling_disabled) {
            locationComponent!!.setMaxAnimationFps(Int.MAX_VALUE)
        } else if (id == R.id.action_component_animate_while_tracking) {
            locationComponent!!.zoomWhileTracking(
                17.0,
                750,
                object : CancelableCallback {
                    override fun onCancel() {
                        // No impl
                    }

                    override fun onFinish() {
                        locationComponent!!.tiltWhileTracking(60.0)
                    }
                }
            )
            if (locationComponent!!.cameraMode == CameraMode.NONE) {
                Toast.makeText(this, "Not possible to animate - not tracking", Toast.LENGTH_SHORT)
                    .show()
            }
        } else if (id == R.id.action_component_padding_animation_while_tracking) {
            val paddingRandom = Random()
            locationComponent!!.paddingWhileTracking(
                doubleArrayOf(
                    paddingRandom.nextDouble() * 500,
                    paddingRandom.nextDouble() * 500,
                    paddingRandom.nextDouble() * 500,
                    paddingRandom.nextDouble() * 500
                ), 1000L, object : CancelableCallback {
                    override fun onCancel() {
                        // No impl
                    }

                    override fun onFinish() {
                        locationComponent!!.zoomWhileTracking(16.0)
                    }
                })
            if (locationComponent!!.getCameraMode() == CameraMode.NONE) {
                Toast.makeText(this, "Not possible to animate - not tracking", Toast.LENGTH_SHORT)
                    .show()
            }
        }
        return super.onOptionsItemSelected(item)
    }

    private fun toggleStyle() {
        if (locationComponent == null) {
            return
        }
        defaultStyle = !defaultStyle
        var options = LocationComponentOptions.createFromAttributes(
            this,
            if (defaultStyle) R.style.maplibre_LocationComponent else R.style.CustomLocationComponent
        )
        if (defaultStyle) {
            val padding: IntArray
            padding =
                if (resources.configuration.orientation == Configuration.ORIENTATION_PORTRAIT) {
                    intArrayOf(0, 750, 0, 0)
                } else {
                    intArrayOf(0, 250, 0, 0)
                }
            options = options.toBuilder()
                .padding(padding)
                .build()
        }
        locationComponent!!.applyStyle(options)
    }

    private fun toggleMapStyle() {
        if (locationComponent == null) {
            return
        }
        maplibreMap.getStyle { style: Style ->
            val styleUrl =
                TestStyles.getPredefinedStyleWithFallback(
                    if (TestStyles.getPredefinedStyleWithFallback("Bright") == style.uri) {
                        "Bright"
                    } else {
                        "Pastel"
                    }
                )

            maplibreMap.setStyle(Style.Builder().fromUri(styleUrl))
        }
    }

    private fun disableGesturesManagement() {
        if (locationComponent == null) {
            return
        }
        protectedGestureArea!!.layoutParams.height = 0
        protectedGestureArea!!.layoutParams.width = 0
        val options = locationComponent!!
            .locationComponentOptions
            .toBuilder()
            .trackingGesturesManagement(false)
            .build()
        locationComponent!!.applyStyle(options)
    }

    private fun enableGesturesManagement() {
        if (locationComponent == null) {
            return
        }
        val rectF = RectF(0f, 0f, mapView.width / 2f, mapView.height / 2f)
        protectedGestureArea!!.layoutParams.height = rectF.bottom.toInt()
        protectedGestureArea!!.layoutParams.width = rectF.right.toInt()
        val options = locationComponent!!
            .locationComponentOptions
            .toBuilder()
            .trackingGesturesManagement(true)
            .trackingMultiFingerProtectedMoveArea(rectF)
            .trackingMultiFingerMoveThreshold(500f)
            .build()
        locationComponent!!.applyStyle(options)
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

    @SuppressLint("MissingPermission")
    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
        outState.putInt(SAVED_STATE_CAMERA, cameraMode)
        outState.putInt(SAVED_STATE_RENDER, renderMode)
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

    override fun onLocationComponentClick() {
        Toast.makeText(this, "OnLocationComponentClick", Toast.LENGTH_LONG).show()
    }

    private fun showModeListDialog() {
        val modes: MutableList<String> = ArrayList()
        modes.add("Normal")
        modes.add("Compass")
        modes.add("GPS")
        val profileAdapter = ArrayAdapter(
            this,
            android.R.layout.simple_list_item_1,
            modes
        )
        val listPopup = ListPopupWindow(this)
        listPopup.setAdapter(profileAdapter)
        listPopup.anchorView = locationModeBtn
        listPopup.setOnItemClickListener { parent: AdapterView<*>?, itemView: View?, position: Int, id: Long ->
            val selectedMode = modes[position]
            locationModeBtn.text = selectedMode
            if (selectedMode.contentEquals("Normal")) {
                setRendererMode(RenderMode.NORMAL)
            } else if (selectedMode.contentEquals("Compass")) {
                setRendererMode(RenderMode.COMPASS)
            } else if (selectedMode.contentEquals("GPS")) {
                setRendererMode(RenderMode.GPS)
            }
            listPopup.dismiss()
        }
        listPopup.show()
    }

    @SuppressLint("SetTextI18n")
    private fun setRendererMode(@RenderMode.Mode mode: Int) {
        renderMode = mode
        locationComponent!!.renderMode = mode
        if (mode == RenderMode.NORMAL) {
            locationModeBtn.text = "Normal"
        } else if (mode == RenderMode.COMPASS) {
            locationModeBtn.text = "Compass"
        } else if (mode == RenderMode.GPS) {
            locationModeBtn.text = "Gps"
        }
    }

    private fun showTrackingListDialog() {
        val trackingTypes: MutableList<String> = ArrayList()
        trackingTypes.add("None")
        trackingTypes.add("None Compass")
        trackingTypes.add("None GPS")
        trackingTypes.add("Tracking")
        trackingTypes.add("Tracking Compass")
        trackingTypes.add("Tracking GPS")
        trackingTypes.add("Tracking GPS North")
        val profileAdapter = ArrayAdapter(
            this,
            android.R.layout.simple_list_item_1,
            trackingTypes
        )
        val listPopup = ListPopupWindow(this)
        listPopup.setAdapter(profileAdapter)
        listPopup.anchorView = locationTrackingBtn
        listPopup.setOnItemClickListener { parent: AdapterView<*>?, itemView: View?, position: Int, id: Long ->
            val selectedTrackingType = trackingTypes[position]
            locationTrackingBtn.text = selectedTrackingType
            if (selectedTrackingType.contentEquals("None")) {
                setCameraTrackingMode(CameraMode.NONE)
            } else if (selectedTrackingType.contentEquals("None Compass")) {
                setCameraTrackingMode(CameraMode.NONE_COMPASS)
            } else if (selectedTrackingType.contentEquals("None GPS")) {
                setCameraTrackingMode(CameraMode.NONE_GPS)
            } else if (selectedTrackingType.contentEquals("Tracking")) {
                setCameraTrackingMode(CameraMode.TRACKING)
            } else if (selectedTrackingType.contentEquals("Tracking Compass")) {
                setCameraTrackingMode(CameraMode.TRACKING_COMPASS)
            } else if (selectedTrackingType.contentEquals("Tracking GPS")) {
                setCameraTrackingMode(CameraMode.TRACKING_GPS)
            } else if (selectedTrackingType.contentEquals("Tracking GPS North")) {
                setCameraTrackingMode(CameraMode.TRACKING_GPS_NORTH)
            }
            listPopup.dismiss()
        }
        listPopup.show()
    }

    private fun setCameraTrackingMode(@CameraMode.Mode mode: Int) {
        locationComponent!!.setCameraMode(
            mode,
            1200,
            16.0,
            null,
            45.0,
            object : OnLocationCameraTransitionListener {
                override fun onLocationCameraTransitionFinished(@CameraMode.Mode cameraMode: Int) {
                    Toast.makeText(
                        this@LocationModesActivity,
                        "Transition finished",
                        Toast.LENGTH_SHORT
                    ).show()
                }

                override fun onLocationCameraTransitionCanceled(@CameraMode.Mode cameraMode: Int) {
                    Toast.makeText(
                        this@LocationModesActivity,
                        "Transition canceled",
                        Toast.LENGTH_SHORT
                    ).show()
                }
            }
        )
    }

    @SuppressLint("SetTextI18n")
    override fun onCameraTrackingDismissed() {
        locationTrackingBtn.text = "None"
    }

    @SuppressLint("SetTextI18n")
    override fun onCameraTrackingChanged(currentMode: Int) {
        cameraMode = currentMode
        if (currentMode == CameraMode.NONE) {
            locationTrackingBtn.text = "None"
        } else if (currentMode == CameraMode.NONE_COMPASS) {
            locationTrackingBtn.text = "None Compass"
        } else if (currentMode == CameraMode.NONE_GPS) {
            locationTrackingBtn.text = "None GPS"
        } else if (currentMode == CameraMode.TRACKING) {
            locationTrackingBtn.text = "Tracking"
        } else if (currentMode == CameraMode.TRACKING_COMPASS) {
            locationTrackingBtn.text = "Tracking Compass"
        } else if (currentMode == CameraMode.TRACKING_GPS) {
            locationTrackingBtn.text = "Tracking GPS"
        } else if (currentMode == CameraMode.TRACKING_GPS_NORTH) {
            locationTrackingBtn.text = "Tracking GPS North"
        }
    }

    companion object {
        private const val SAVED_STATE_CAMERA = "saved_state_camera"
        private const val SAVED_STATE_RENDER = "saved_state_render"
        private const val SAVED_STATE_LOCATION = "saved_state_location"
    }
}
