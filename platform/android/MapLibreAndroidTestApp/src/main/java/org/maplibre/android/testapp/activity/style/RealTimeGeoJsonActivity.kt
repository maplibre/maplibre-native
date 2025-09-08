package org.maplibre.android.testapp.activity.style

import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.View
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.res.ResourcesCompat
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.*
import org.maplibre.android.style.layers.*
import org.maplibre.android.style.sources.GeoJsonSource
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import org.maplibre.geojson.Feature
import org.maplibre.geojson.Point
import timber.log.Timber
import java.net.URI
import java.net.URISyntaxException
import kotlin.math.atan2

fun calculateRotationAngle(from: Point, to: Point): Float {
    val longitudeDiff = to.longitude() - from.longitude()
    val latitudeDiff = to.latitude() - from.latitude()

    val angleRadians = atan2(longitudeDiff, latitudeDiff)
    val angleDegrees = Math.toDegrees(angleRadians)

    return ((angleDegrees + 360) % 360).toFloat() // Normalize to 0-360 degrees
}

/**
 * Test activity showcasing using realtime GeoJSON to move a symbol on your map
 *
 *
 * MapLibre Native equivalent of https://maplibre.org/maplibre-gl-js-docs/example/live-geojson/
 *
 */
class RealTimeGeoJsonActivity : AppCompatActivity(), OnMapReadyCallback {
    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap
    private var handler: Handler? = null
    private var runnable: Runnable? = null

    private var lastLocation: Point? = null

    private val netherlands = LatLng(52.646396, 5.723877)

    override fun onCreate(savedInstanceState: Bundle?) {
        enableEdgeToEdge()
        super.onCreate(savedInstanceState)

        supportActionBar?.hide() // Hides the action bar if present
        setContentView(R.layout.activity_default)
        mapView = findViewById<View>(R.id.mapView) as MapView
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(this)
    }

    override fun onMapReady(map: MapLibreMap) {
        maplibreMap = map
        maplibreMap.cameraPosition = CameraPosition.Builder().target(netherlands).zoom(6.0).build()
        maplibreMap.setStyle(TestStyles.PROTOMAPS_WHITE) { style -> // add source
            ResourcesCompat.getDrawable(resources, R.drawable.ic_airplanemode_active_black, theme)
                ?.let { style.addImage("plane", it) }
            // --8<-- [start:addSource]
            try {
                style.addSource(GeoJsonSource(ID_GEOJSON_SOURCE, URI(URL_GEOJSON_SOURCE)))
            } catch (malformedUriException: URISyntaxException) {
                Timber.e(malformedUriException, "Invalid URL")
            }
            // --8<-- [end:addSource]

            // --8<-- [start:addLayer]
            val layer = SymbolLayer(ID_GEOJSON_LAYER, ID_GEOJSON_SOURCE)
            layer.setProperties(
                PropertyFactory.iconImage("plane"),
                PropertyFactory.iconAllowOverlap(true)
            )
            style.addLayer(layer)
            // --8<-- [end:addLayer]


            // loop refresh geojson
            handler = Handler(Looper.getMainLooper())
            runnable = RefreshGeoJsonRunnable(maplibreMap, handler!!)
            runnable?.let {
                handler!!.postDelayed(it, 2000)
            }
        }
    }

    override fun onStart() {
        super.onStart()
        mapView.onStart()
    }

    public override fun onResume() {
        super.onResume()
        mapView.onResume()
    }

    public override fun onPause() {
        super.onPause()
        mapView.onPause()
    }

    override fun onStop() {
        super.onStop()
        mapView.onStop()
        runnable?.let {
            handler!!.removeCallbacks(it)
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        mapView.onDestroy()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }

    private fun setIconRotation(features: List<Feature>) {
        // --8<-- [start:setIconRotation]
        if (features.size != 1) {
            Timber.e("Expected only one feature")
            return
        }

        val feature = features[0]
        val geometry = feature.geometry()
        if (geometry !is Point) {
            Timber.e("Expected geometry to be a point")
            return
        }

        if (lastLocation == null) {
            lastLocation = geometry
            return
        }

        maplibreMap.style!!.getLayer(ID_GEOJSON_LAYER)!!.setProperties(
            PropertyFactory.iconRotate(calculateRotationAngle(lastLocation!!, geometry)),
        )
        // --8<-- [end:setIconRotation]
        lastLocation = geometry
    }

    // --8<-- [start:Runnable]
    private inner class RefreshGeoJsonRunnable(
        private val maplibreMap: MapLibreMap,
        private val handler: Handler
    ) : Runnable {
        override fun run() {
            val geoJsonSource = maplibreMap.style!!.getSource(ID_GEOJSON_SOURCE) as GeoJsonSource
            geoJsonSource.setUri(URL_GEOJSON_SOURCE)
            val features = geoJsonSource.querySourceFeatures(null)
            setIconRotation(features)
            handler.postDelayed(this, 2000)
        }
    }
    // --8<-- [end:Runnable]

    companion object {
        private const val ID_GEOJSON_LAYER = "border"
        private const val ID_GEOJSON_SOURCE = ID_GEOJSON_LAYER
        private const val URL_GEOJSON_SOURCE = "https://m6rgfvqjp34nnwqcdm4cmmy3cm0dtupu.lambda-url.us-east-1.on.aws/"
    }
}
