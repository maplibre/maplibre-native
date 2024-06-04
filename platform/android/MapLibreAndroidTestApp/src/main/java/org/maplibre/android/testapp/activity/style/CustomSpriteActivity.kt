package org.maplibre.android.testapp.activity.style

import android.graphics.BitmapFactory
import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.google.android.material.floatingactionbutton.FloatingActionButton
import org.maplibre.geojson.Feature
import org.maplibre.geojson.FeatureCollection
import org.maplibre.geojson.Point
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.OnMapReadyCallback
import org.maplibre.android.maps.Style
import org.maplibre.android.style.layers.Layer
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.style.layers.SymbolLayer
import org.maplibre.android.style.sources.GeoJsonSource
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import timber.log.Timber

/**
 * Test activity showcasing adding a sprite image and use it in a Symbol Layer
 */
class CustomSpriteActivity : AppCompatActivity() {
    private var source: GeoJsonSource? = null
    private lateinit var maplibreMap: MapLibreMap
    private lateinit var mapView: MapView
    private lateinit var layer: Layer
    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_add_sprite)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(
            OnMapReadyCallback { map: MapLibreMap ->
                maplibreMap = map
                map.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets")) { style: Style ->
                    val fab = findViewById<FloatingActionButton>(R.id.fab)
                    fab.setColorFilter(
                        ContextCompat.getColor(
                            this@CustomSpriteActivity,
                            R.color.primary
                        )
                    )
                    fab.setOnClickListener(object : View.OnClickListener {
                        private lateinit var point: Point
                        override fun onClick(view: View) {
                            if (point == null) {
                                Timber.i("First click -> Car")
                                // Add an icon to reference later
                                style.addImage(
                                    CUSTOM_ICON,
                                    BitmapFactory.decodeResource(
                                        resources,
                                        R.drawable.ic_car_top
                                    )
                                )

                                // Add a source with a geojson point
                                point = Point.fromLngLat(13.400972, 52.519003)
                                source = GeoJsonSource(
                                    "point",
                                    FeatureCollection.fromFeatures(arrayOf(Feature.fromGeometry(point)))
                                )
                                maplibreMap.style!!.addSource(source!!)

                                // Add a symbol layer that references that point source
                                layer = SymbolLayer("layer", "point")
                                layer.setProperties( // Set the id of the sprite to use
                                    PropertyFactory.iconImage(CUSTOM_ICON),
                                    PropertyFactory.iconAllowOverlap(true),
                                    PropertyFactory.iconIgnorePlacement(true)
                                )

                                // lets add a circle below labels!
                                maplibreMap.style!!.addLayerBelow(layer, "water_intermittent")
                                fab.setImageResource(R.drawable.ic_directions_car_black)
                            } else {
                                // Update point
                                point = Point.fromLngLat(
                                    point!!.longitude() + 0.001,
                                    point!!.latitude() + 0.001
                                )
                                source!!.setGeoJson(
                                    FeatureCollection.fromFeatures(
                                        arrayOf(
                                            Feature.fromGeometry(
                                                point
                                            )
                                        )
                                    )
                                )

                                // Move the camera as well
                                maplibreMap.moveCamera(
                                    CameraUpdateFactory.newLatLng(
                                        LatLng(
                                            point.latitude(),
                                            point.longitude()
                                        )
                                    )
                                )
                            }
                        }
                    })
                }
            }
        )
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

    public override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    public override fun onDestroy() {
        super.onDestroy()
        mapView.onDestroy()
    }

    companion object {
        private const val CUSTOM_ICON = "custom-icon"
    }
}
