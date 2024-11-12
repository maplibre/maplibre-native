package org.maplibre.android.testapp.activity.style

import android.graphics.Color
import android.os.Bundle
import android.os.PersistableBundle
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.geojson.Point
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Style
import org.maplibre.android.style.expressions.Expression.distance
import org.maplibre.android.style.expressions.Expression.lt
import org.maplibre.android.style.layers.FillLayer
import org.maplibre.android.style.layers.Property.NONE
import org.maplibre.android.style.layers.PropertyFactory.*
import org.maplibre.android.style.layers.SymbolLayer
import org.maplibre.android.style.sources.GeoJsonSource
import org.maplibre.android.testapp.databinding.ActivityWithinExpressionBinding
import org.maplibre.android.testapp.styles.TestStyles
import org.maplibre.turf.TurfConstants
import org.maplibre.turf.TurfTransformation

/**
 * An Activity that showcases the within expression to filter features outside a geometry
 */
class DistanceExpressionActivity : AppCompatActivity() {

    private lateinit var binding: ActivityWithinExpressionBinding
    private lateinit var maplibreMap: MapLibreMap

    private val lat = 37.78794572301525
    private val lon = -122.40752220153807

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityWithinExpressionBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.mapView.onCreate(savedInstanceState)
        binding.mapView.getMapAsync { map ->
            maplibreMap = map

            // Setup camera position above Georgetown
            maplibreMap.cameraPosition = CameraPosition.Builder()
                .target(LatLng(lat, lon))
                .zoom(16.0)
                .build()
            setupStyle()
        }
    }

    private fun setupStyle() {
        val center = Point.fromLngLat(lon, lat)
        val circle = TurfTransformation.circle(center, 150.0, TurfConstants.UNIT_METRES)
        // Setup style with additional layers,
        // using Streets as a base style
        maplibreMap.setStyle(
            Style.Builder()
                .fromUri(TestStyles.getPredefinedStyleWithFallback("Streets"))
                .withSources(
                    GeoJsonSource(
                        POINT_ID,
                        Point.fromLngLat(lon, lat)
                    ),
                    GeoJsonSource(CIRCLE_ID, circle)
                )
                .withLayerBelow(
                    FillLayer(CIRCLE_ID, CIRCLE_ID)
                        .withProperties(
                            fillOpacity(0.5f),
                            fillColor(Color.parseColor("#3bb2d0"))
                        ),
                    "poi-label"
                )
        ) { style ->
            // Show only POI labels inside circle radius using distance expression
            val symbolLayer = style.getLayer("poi_z16") as SymbolLayer
            symbolLayer.setFilter(
                lt(
                    distance(
                        Point.fromLngLat(lon, lat)
                    ),
                    150
                )
            )

            // Hide other types of labels to highlight POI labels
            (style.getLayer("road_label") as SymbolLayer?)?.setProperties(visibility(NONE))
            (style.getLayer("airport-label-major") as SymbolLayer?)?.setProperties(visibility(NONE))
            (style.getLayer("poi_transit") as SymbolLayer?)?.setProperties(visibility(NONE))
        }
    }

    override fun onStart() {
        super.onStart()
        binding.mapView.onStart()
    }

    override fun onResume() {
        super.onResume()
        binding.mapView.onResume()
    }

    override fun onPause() {
        super.onPause()
        binding.mapView.onPause()
    }

    override fun onStop() {
        super.onStop()
        binding.mapView.onStop()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        binding.mapView.onLowMemory()
    }

    override fun onDestroy() {
        super.onDestroy()
        binding.mapView.onDestroy()
    }

    override fun onSaveInstanceState(outState: Bundle, outPersistentState: PersistableBundle) {
        super.onSaveInstanceState(outState, outPersistentState)
        outState?.let {
            binding.mapView.onSaveInstanceState(it)
        }
    }

    companion object {
        const val POINT_ID = "point"
        const val CIRCLE_ID = "circle"
    }
}
