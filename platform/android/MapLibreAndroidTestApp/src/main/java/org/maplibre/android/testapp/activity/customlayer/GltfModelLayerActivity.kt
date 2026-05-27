package org.maplibre.android.testapp.activity.customlayer
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.widget.LinearLayout
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Style
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.FillExtrusionLayer
import org.maplibre.android.style.layers.GltfModelLayer
import org.maplibre.android.style.layers.Property
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import org.maplibre.android.testapp.utils.animateCameraSuspend
import org.maplibre.android.testapp.utils.easeCameraSuspend
import org.maplibre.geojson.Point
import org.json.JSONArray

/**
 * Example activity demonstrating the GltfModelLayer.
 *
 * Renders GLB models from the app's assets directory at geographic locations
 * defined in **assets/gltf_model_placements.json**. Each entry can specify
 * position, scale, heading (`rotationY`, etc.), and axis mirrors without
 * Kotlin changes.
 */
class GltfModelLayerActivity : AppCompatActivity() {

    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap
    private var currentStyle: Style? = null
    private var exploreMenuReady = false
    private var lastSelectedPlacement: ModelPlacement? = null
    private var zoomTransitionJob: Job? = null

    /** Loaded from [PLACEMENTS_JSON_ASSET]; empty if the file is missing or invalid. */
    private var modelPlacements: List<ModelPlacement> = emptyList()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_custom_layer)

        modelPlacements = loadModelPlacementsFromJson()
        if (modelPlacements.isEmpty()) {
            Toast.makeText(
                this,
                "No models: fix or add assets/$PLACEMENTS_JSON_ASSET",
                Toast.LENGTH_LONG
            ).show()
        }

        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        setupExploreMenu()
        mapView.getMapAsync { map ->
            maplibreMap = map

            val focus = modelPlacements.firstOrNull()
            if (focus != null) {
                maplibreMap.moveCamera(
                    CameraUpdateFactory.newLatLngZoom(
                        LatLng(focus.lat, focus.lng),
                        TOUR_ZOOM_OUT
                    )
                )
            }

            maplibreMap.setStyle(TestStyles.OPENFREEMAP_LIBERTY) { style ->
                currentStyle = style
                addModelLayers(style)
                lastSelectedPlacement = modelPlacements.firstOrNull()
                exploreMenuReady = true
            }
        }
    }

    private fun setupExploreMenu() {
        val selector = findViewById<TextView>(R.id.exploreModelSelector)
        val modelList = findViewById<LinearLayout>(R.id.exploreModelList)
        val displayNames = modelPlacements.map { it.displayName }

        val initialPlacement = lastSelectedPlacement ?: modelPlacements.firstOrNull()
        val currentIndex = if (initialPlacement != null) {
            modelPlacements.indexOf(initialPlacement).coerceIn(0, (displayNames.size - 1).coerceAtLeast(0))
        } else 0
        selector.text = if (displayNames.isNotEmpty()) "${displayNames[currentIndex]} \u25BC" else "—"

        selector.setOnClickListener {
            if (displayNames.isEmpty()) return@setOnClickListener
            val isExpanded = modelList.visibility == View.VISIBLE
            modelList.visibility = if (isExpanded) View.GONE else View.VISIBLE
            val ref = lastSelectedPlacement ?: modelPlacements.firstOrNull()
            val idx = if (ref != null) modelPlacements.indexOf(ref).coerceIn(0, displayNames.lastIndex) else 0
            selector.text = "${displayNames[idx]} ${if (isExpanded) "\u25BC" else "\u25B2"}"
        }

        displayNames.forEachIndexed { index, name ->
            val item = LayoutInflater.from(this).inflate(R.layout.explore_model_list_item, modelList, false) as TextView
            item.text = name
            item.setOnClickListener {
                if (exploreMenuReady && index in modelPlacements.indices) {
                    zoomToModel(modelPlacements[index])
                    modelList.visibility = View.GONE
                    selector.text = "$name \u25BC"
                }
            }
            modelList.addView(item)
        }
    }

    private fun zoomToModel(placement: ModelPlacement) {
        if (!::maplibreMap.isInitialized) return

        zoomTransitionJob?.cancel()
        val menuContainer = findViewById<View>(R.id.exploreMenuContainer)
        menuContainer.isEnabled = false

        zoomTransitionJob = lifecycleScope.launch {
            try {
                withContext(Dispatchers.Main) {
                    val targetLatLng = LatLng(placement.lat, placement.lng)
                    val isSwitchingModel = lastSelectedPlacement != null && lastSelectedPlacement != placement

                    if (isSwitchingModel) {
                    // Transition: zoom out from current model -> fly to new model (user sees whole movement) -> zoom in
                    currentStyle?.let { setLayersVisibilityDuringTransition(it, false) }
                    val fromLatLng = LatLng(lastSelectedPlacement!!.lat, lastSelectedPlacement!!.lng)
                    maplibreMap.easeCameraSuspend(
                        CameraUpdateFactory.newCameraPosition(
                            CameraPosition.Builder()
                                .target(fromLatLng)
                                .zoom(TOUR_ZOOM_OUT)
                                .tilt(20.0)
                                .bearing(0.0)
                                .build()
                        ),
                        TOUR_ZOOM_OUT_DURATION_MS
                    )
                    maplibreMap.animateCameraSuspend(
                        CameraUpdateFactory.newLatLngZoom(targetLatLng, TOUR_ZOOM_OUT),
                        TOUR_FLY_TO_NEXT_DURATION_MS
                    )
                    currentStyle?.let { setLayersVisibilityDuringTransition(it, true) }
                    maplibreMap.easeCameraSuspend(
                        CameraUpdateFactory.newCameraPosition(
                            CameraPosition.Builder()
                                .target(targetLatLng)
                                .zoom(zoomForModel(placement.heightMeters))
                                .tilt(TOUR_TILT)
                                .bearing(0.0)
                                .build()
                        ),
                        TOUR_ZOOM_IN_DURATION_MS
                    )
                } else {
                    currentStyle?.let { setLayersVisibilityDuringTransition(it, true) }
                    maplibreMap.easeCameraSuspend(
                        CameraUpdateFactory.newCameraPosition(
                            CameraPosition.Builder()
                                .target(targetLatLng)
                                .zoom(zoomForModel(placement.heightMeters))
                                .tilt(TOUR_TILT)
                                .bearing(0.0)
                                .build()
                        ),
                        TOUR_ZOOM_IN_DURATION_MS
                    )
                }
                lastSelectedPlacement = placement
            }
            } finally {
                withContext(Dispatchers.Main) {
                    findViewById<View>(R.id.exploreMenuContainer).isEnabled = true
                }
            }
        }
    }

    private fun loadAssetBytes(assetName: String): ByteArray {
        return assets.open(assetName).use { it.readBytes() }
    }

    private fun addModelLayers(style: Style) {
        var successCount = 0
        var lastError: Exception? = null

        for (placement in modelPlacements) {
            try {
                val modelData = loadAssetBytes(placement.asset)
                val layer = GltfModelLayer(
                    "gltf-model-${placement.id}",
                    modelData,
                    LatLng(placement.lat, placement.lng),
                    placement.scale,
                    placement.rotationX,
                    placement.rotationY,
                    placement.rotationZ,
                    placement.mirrorX,
                    placement.mirrorY,
                    placement.mirrorZ
                )
                style.addLayer(layer)
                successCount++
            } catch (e: Exception) {
                lastError = e
            }
        }

        // Hide 2.5D fill-extrusion buildings at POI locations where we have GLB models
        hide2_5DBuildingsAtGlbLocations(style)

        when {
            successCount == modelPlacements.size ->
                Toast.makeText(this, "$successCount models added", Toast.LENGTH_SHORT).show()
            successCount > 0 ->
                Toast.makeText(this, "$successCount of ${modelPlacements.size} models added", Toast.LENGTH_SHORT).show()
            else ->
                Toast.makeText(this, "Failed to add models: ${lastError?.message}", Toast.LENGTH_LONG).show()
        }
    }

    /**
     * Hides 3D fill-extrusion box buildings at POI locations where we render GLB models instead.
     * Uses a distance filter to exclude buildings within EXCLUSION_RADIUS_M of each GLB placement.
     * When a location has a GLB model, hide the box extrusion so only the GLB is visible.
     */
    private fun hide2_5DBuildingsAtGlbLocations(style: Style) {
        val buildingLayerIds = listOf(
            "building-3d", "3d-buildings", "Building 3D", "building-3d-roof"
        )
        val layers = buildingLayerIds.mapNotNull { id ->
            style.getLayer(id)?.takeIf { it is FillExtrusionLayer } as? FillExtrusionLayer
        }
        if (layers.isEmpty()) return

        val distanceFilters = modelPlacements.map { placement ->
            Expression.gte(
                Expression.distance(Point.fromLngLat(placement.lng, placement.lat)),
                Expression.literal(EXCLUSION_RADIUS_M)
            )
        }

        val filter = Expression.all(
            Expression.not(Expression.has("hide_3d")),
            *distanceFilters.toTypedArray()
        )
        for (layer in layers) {
            layer.setFilter(filter)
        }
    }

    /**
     * Sets visibility of layers that show POI names and 2.5D buildings during camera movement.
     * Hiding them during transitions prevents loading/rendering and keeps the view smooth.
     */
    private fun setLayersVisibilityDuringTransition(style: Style, visible: Boolean) {
        val visibility = if (visible) Property.VISIBLE else Property.NONE
        val layersToToggle = listOf(
            "building", "building-3d", "3d-buildings", "Building 3D", "building-3d-roof",
            "poi_z16", "poi_z15", "poi_z14", "poi_transit", "poi_z16_subclass",
            "place_city", "place_town", "place_village", "place_other", "place_state",
            "road_label", "water_name_point", "water_ocean_name_point",
            "airport-label-major"
        )
        for (layerId in layersToToggle) {
            try {
                style.getLayer(layerId)?.setProperties(PropertyFactory.visibility(visibility))
            } catch (_: Exception) { /* Layer may not exist in this style */ }
        }
    }

    /**
     * Starts an automatic camera tour: zooms into each GLB model one by one, zooms out, then
     * smoothly flies to the next. Uses ease for zoom (smooth) and fly for long moves. Hides
     * POI labels and 2.5D buildings during flight to prevent loading and keep transitions smooth.
     */
    private fun startCameraTour(style: Style) {
        lifecycleScope.launch {
            withContext(Dispatchers.Main) {
                delay(TOUR_INITIAL_DELAY_MS)
                for ((index, placement) in modelPlacements.withIndex()) {
                    val latLng = LatLng(placement.lat, placement.lng)

                    // Show POI labels and 2.5D when viewing a model
                    setLayersVisibilityDuringTransition(style, true)

                    // Zoom in to the model (ease = smooth acceleration)
                    maplibreMap.easeCameraSuspend(
                        CameraUpdateFactory.newCameraPosition(
                            CameraPosition.Builder()
                                .target(latLng)
                                .zoom(zoomForModel(placement.heightMeters))
                                .tilt(TOUR_TILT)
                                .bearing(0.0)
                                .build()
                        ),
                        TOUR_ZOOM_IN_DURATION_MS
                    )
                    delay(TOUR_HOLD_AT_MODEL_MS)

                    // Zoom out from the model
                    maplibreMap.easeCameraSuspend(
                        CameraUpdateFactory.newLatLngZoom(latLng, TOUR_ZOOM_OUT),
                        TOUR_ZOOM_OUT_DURATION_MS
                    )
                    delay(TOUR_PAUSE_BETWEEN_MS)

                    // Fly to next model – hide labels and 2.5D during flight
                    if (index < modelPlacements.size - 1) {
                        setLayersVisibilityDuringTransition(style, false)
                        val next = modelPlacements[index + 1]
                        maplibreMap.animateCameraSuspend(
                            CameraUpdateFactory.newLatLngZoom(
                                LatLng(next.lat, next.lng),
                                TOUR_ZOOM_OUT
                            ),
                            TOUR_FLY_TO_NEXT_DURATION_MS
                        )
                        delay(TOUR_PAUSE_BETWEEN_MS)
                    }
                }
            }
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
        mapView.onDestroy()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    companion object {
        /** Radius in meters around each GLB POI within which extrusion (2.5D/3D) buildings are hidden. */
        private const val EXCLUSION_RADIUS_M = 50

        /** Model list, coordinates, scale, and per-model rotation — edit this file, not Kotlin. */
        private const val PLACEMENTS_JSON_ASSET = "gltf_model_placements.json"

        private const val TOUR_ZOOM_IN = 17.5
        private const val TOUR_ZOOM_OUT = 15.0
        private const val REF_HEIGHT_M = 25f
        private const val ZOOM_PER_METER = 0.04
        private const val TOUR_TILT = 45.0
        private const val TOUR_INITIAL_DELAY_MS = 1500L
        private const val TOUR_ZOOM_IN_DURATION_MS = 2500
        private const val TOUR_ZOOM_OUT_DURATION_MS = 1500
        private const val TOUR_FLY_TO_NEXT_DURATION_MS = 3000
        private const val TOUR_HOLD_AT_MODEL_MS = 2000L
        private const val TOUR_PAUSE_BETWEEN_MS = 800L
    }

    private data class ModelPlacement(
        val id: String,
        /** Label in the explore dropdown; defaults to [id] if omitted in JSON. */
        val displayName: String,
        val asset: String,
        val lat: Double,
        val lng: Double,
        val heightMeters: Float = 25f,
        val scale: Float = 1.0f,
        val rotationX: Float = 90f,
        val rotationY: Float = 0f,
        val rotationZ: Float = 0f,
        /** Negate scale on map-plane X before rotation (geometric mirror). */
        val mirrorX: Boolean = false,
        /** Negate scale on map-plane Y before rotation. */
        val mirrorY: Boolean = false,
        /** Negate scale on vertical axis before rotation. */
        val mirrorZ: Boolean = false,
    )

    private fun loadModelPlacementsFromJson(): List<ModelPlacement> {
        return try {
            assets.open(PLACEMENTS_JSON_ASSET).bufferedReader().use { reader ->
                val arr = JSONArray(reader.readText())
                buildList {
                    for (i in 0 until arr.length()) {
                        val o = arr.getJSONObject(i)
                        val id = o.getString("id")
                        add(
                            ModelPlacement(
                                id = id,
                                displayName = o.optString("displayName", id),
                                asset = o.getString("asset"),
                                lat = o.getDouble("lat"),
                                lng = o.getDouble("lng"),
                                heightMeters = o.optDouble("heightMeters", 25.0).toFloat(),
                                scale = o.optDouble("scale", 1.0).toFloat(),
                                rotationX = o.optDouble("rotationX", 90.0).toFloat(),
                                rotationY = o.optDouble("rotationY", 0.0).toFloat(),
                                rotationZ = o.optDouble("rotationZ", 0.0).toFloat(),
                                mirrorX = o.optBoolean("mirrorX", false),
                                mirrorY = o.optBoolean("mirrorY", false),
                                mirrorZ = o.optBoolean("mirrorZ", false),
                            )
                        )
                    }
                }
            }
        } catch (e: Exception) {
            Log.e("GltfModelLayer", "Failed to load $PLACEMENTS_JSON_ASSET", e)
            emptyList()
        }
    }

    /** Zoom level for viewing a model: smaller height = zoom in more. Clamped to 16..19. */
    private fun zoomForModel(heightMeters: Float): Double {
        val zoom = TOUR_ZOOM_IN + ZOOM_PER_METER * (REF_HEIGHT_M - heightMeters)
        return zoom.coerceIn(16.0, 19.0)
    }
}
