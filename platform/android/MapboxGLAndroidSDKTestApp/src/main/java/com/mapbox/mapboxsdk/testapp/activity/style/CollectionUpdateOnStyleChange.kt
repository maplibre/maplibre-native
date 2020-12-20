package com.mapbox.mapboxsdk.testapp.activity.style

import android.graphics.Color
import android.os.Bundle
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.google.android.material.floatingactionbutton.FloatingActionButton
import com.mapbox.geojson.Feature
import com.mapbox.geojson.FeatureCollection
import com.mapbox.geojson.LineString
import com.mapbox.geojson.Point
import com.mapbox.mapboxsdk.geometry.LatLng
import com.mapbox.mapboxsdk.geometry.LatLngBounds
import com.mapbox.mapboxsdk.maps.MapboxMap
import com.mapbox.mapboxsdk.maps.OnMapReadyCallback
import com.mapbox.mapboxsdk.maps.Style
import com.mapbox.mapboxsdk.style.layers.LineLayer
import com.mapbox.mapboxsdk.style.layers.PropertyFactory
import com.mapbox.mapboxsdk.style.sources.GeoJsonSource
import com.mapbox.mapboxsdk.testapp.R
import java.util.*
import kotlinx.android.synthetic.main.activity_collection_update_on_style_change.*

/**
 * Test activity that verifies whether the GeoJsonSource transition over style changes can be smooth.
 */
class CollectionUpdateOnStyleChange : AppCompatActivity(), OnMapReadyCallback, Style.OnStyleLoaded {

  private lateinit var mapboxMap: MapboxMap
  private var currentStyleIndex: Int = 0

  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
    setContentView(R.layout.activity_collection_update_on_style_change)
    mapView.onCreate(savedInstanceState)
    mapView.getMapAsync(this)
    setupStyleChangeView()
    Toast.makeText(
      this,
      "Make sure that the collection doesn't blink on style change",
      Toast.LENGTH_LONG)
      .show()
  }

  override fun onMapReady(map: MapboxMap) {
    mapboxMap = map
    mapboxMap.setStyle(Style.Builder().fromUri(STYLES[currentStyleIndex]), this)
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
      mapboxMap.setStyle(Style.Builder().fromUri(STYLES[currentStyleIndex]), this)
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

    private val STYLES = arrayOf(Style.MAPBOX_STREETS, Style.OUTDOORS, Style.LIGHT, Style.DARK, Style.SATELLITE, Style.SATELLITE_STREETS, Style.TRAFFIC_DAY, Style.TRAFFIC_NIGHT)

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
      val randomLat = bounds.latSouth + generator.nextDouble() * (bounds.latNorth - bounds.latSouth)
      val randomLon = bounds.lonWest + generator.nextDouble() * (bounds.lonEast - bounds.lonWest)
      return LatLng(randomLat, randomLon)
    }
  }
}