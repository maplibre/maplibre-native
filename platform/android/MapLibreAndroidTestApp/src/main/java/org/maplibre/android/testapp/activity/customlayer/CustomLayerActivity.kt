package org.maplibre.android.testapp.activity.customlayer

import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.google.android.material.floatingactionbutton.FloatingActionButton
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.OnMapReadyCallback
import org.maplibre.android.maps.Style
import org.maplibre.android.style.layers.CustomLayer
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.model.customlayer.ExampleCustomLayer
import org.maplibre.android.testapp.styles.TestStyles

/**
 * Test activity showcasing the Custom Layer API
 *
 *
 * Note: experimental API, do not use.
 *
 */
class CustomLayerActivity : AppCompatActivity() {
    private lateinit var maplibreMap: MapLibreMap
    private lateinit var mapView: MapView
    private var customLayer: CustomLayer? = null
    private lateinit var fab: FloatingActionButton
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_custom_layer)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(
            OnMapReadyCallback { map: MapLibreMap? ->
                maplibreMap = map!!
                maplibreMap.moveCamera(
                    CameraUpdateFactory.newLatLngZoom(
                        LatLng(39.91448, -243.60947),
                        10.0
                    )
                )
                maplibreMap.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets")) { style: Style? -> initFab() }
            }
        )
    }

    private fun initFab() {
        fab = findViewById(R.id.fab)
        fab.setColorFilter(ContextCompat.getColor(this, R.color.primary))
        fab.setOnClickListener(
            View.OnClickListener { view: View? ->
                if (maplibreMap != null) {
                    swapCustomLayer()
                }
            }
        )
    }

    private fun swapCustomLayer() {
        val style = maplibreMap.style
        if (customLayer != null) {
            style!!.removeLayer(customLayer!!)
            customLayer = null
            fab!!.setImageResource(R.drawable.ic_layers)
        } else {
            customLayer = CustomLayer(
                "custom",
                ExampleCustomLayer.createContext()
            )
            style!!.addLayerBelow(customLayer!!, "building")
            fab!!.setImageResource(R.drawable.ic_layers_clear)
        }
    }

    private fun updateLayer() {
        if (maplibreMap != null) {
            maplibreMap.triggerRepaint()
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

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.menu_custom_layer, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.action_update_layer -> {
                updateLayer()
                true
            }
            R.id.action_set_color_red -> {
                ExampleCustomLayer.setColor(1f, 0f, 0f, 1f)
                true
            }
            R.id.action_set_color_green -> {
                ExampleCustomLayer.setColor(0f, 1f, 0f, 1f)
                true
            }
            R.id.action_set_color_blue -> {
                ExampleCustomLayer.setColor(0f, 0f, 1f, 1f)
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }
}
