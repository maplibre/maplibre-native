package org.maplibre.android.testapp.activity.style

import android.graphics.Color
import android.os.Bundle
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.google.android.material.floatingactionbutton.FloatingActionButton
import org.maplibre.geojson.Feature
import org.maplibre.geojson.FeatureCollection
import org.maplibre.geojson.LineString
import org.maplibre.geojson.Point
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.OnMapReadyCallback
import org.maplibre.android.maps.Style
import org.maplibre.android.style.layers.LineLayer
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.style.sources.GeoJsonSource
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.databinding.ActivityCollectionUpdateOnStyleChangeBinding
import org.maplibre.android.testapp.styles.TestStyles
import java.util.*

/**
 * Test activity that verifies whether the GeoJsonSource transition over style changes can be smooth.
 */
class CollectionUpdateOnStyleChange : AppCompatActivity(), OnMapReadyCallback, Style.OnStyleLoaded {

    private lateinit var binding: ActivityCollectionUpdateOnStyleChangeBinding
    private lateinit var maplibreMap: MapLibreMap
    private var currentStyleIndex: Int = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityCollectionUpdateOnStyleChangeBinding.inflate(layoutInflater)
        setContentView(binding.root)
        binding.mapView.onCreate(savedInstanceState)
        binding.mapView.getMapAsync(this)
        setupStyleChangeView()
        Toast.makeText(
            this,
            "Make sure that the collection doesn't blink on style change",
            Toast.LENGTH_LONG
        )
            .show()
    }

    override fun onMapReady(map: MapLibreMap) {
        maplibreMap = map
        maplibreMap.setStyle(Style.Builder().fromUri(STYLES[currentStyleIndex]), this)
    }

    override fun onStyleLoaded(style: Style) {
        setupLayer(style)
    }

    private fun setupLayer(style: Style) {
        val source = GeoJsonSource("source", featureCollection)
        val lineLayer = LineLayer("layer", "source")
            .withProperties(
                PropertyFactory.lineColor(Color.RED),
                PropertyFactory.lineWidth(10f)
            )

        style.addSource(source)
        style.addLayer(lineLayer)
    }

    private fun setupStyleChangeView() {
        val fabStyles = findViewById<FloatingActionButton>(R.id.fabStyles)
        fabStyles.setOnClickListener {
            currentStyleIndex++
            if (currentStyleIndex == STYLES.size) {
                currentStyleIndex = 0
            }
            maplibreMap.setStyle(Style.Builder().fromUri(STYLES[currentStyleIndex]), this)
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

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        binding.mapView.onSaveInstanceState(outState)
    }

    override fun onDestroy() {
        super.onDestroy()
        binding.mapView.onDestroy()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        binding.mapView.onLowMemory()
    }

    companion object {

        private val STYLES = arrayOf(
            TestStyles.getPredefinedStyleWithFallback("Streets"),
            TestStyles.getPredefinedStyleWithFallback("Outdoor"),
            TestStyles.getPredefinedStyleWithFallback("Bright"),
            TestStyles.getPredefinedStyleWithFallback("Pastel"),
            TestStyles.getPredefinedStyleWithFallback("Satellite Hybrid"),
            TestStyles.getPredefinedStyleWithFallback("Satellite Hybrid"))

        private val featureCollection: FeatureCollection

        init {
            val bounds = LatLngBounds.from(60.0, 100.0, -60.0, -100.0)
            val points = ArrayList<Point>()
            for (i in 0 until 1000) {
                val latLng = getLatLngInBounds(bounds)
                points.add(Point.fromLngLat(latLng.longitude, latLng.latitude))
            }
            featureCollection = FeatureCollection.fromFeature(Feature.fromGeometry(LineString.fromLngLats(points)))
        }

        private fun getLatLngInBounds(bounds: LatLngBounds): LatLng {
            val generator = Random()
            val randomLat = bounds.latitudeSouth + generator.nextDouble() * (bounds.latitudeNorth - bounds.latitudeSouth)
            val randomLon = bounds.longitudeWest + generator.nextDouble() * (bounds.longitudeEast - bounds.longitudeWest)
            return LatLng(randomLat, randomLon)
        }
    }
}
