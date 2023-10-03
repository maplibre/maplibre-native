package org.maplibre.android.testapp.activity.annotation

import android.graphics.Color
import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.view.View
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.annotations.Line
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.OnMapReadyCallback
import org.maplibre.android.maps.Style
import org.maplibre.android.testapp.R

/**
 * Test activity showcasing the Polyline annotations API.
 *
 *
 * Shows how to add and remove polylines.
 *
 */
class PolylineActivity : AppCompatActivity() {
    private var polylines: MutableList<Line> = mutableListOf()
    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap
    private var fullAlpha = true
    private var showPolylines = true
    private var width = true
    private var color = true
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_polyline)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(
            OnMapReadyCallback { maplibreMap: MapLibreMap ->
                this@PolylineActivity.maplibreMap = maplibreMap
                maplibreMap.setStyle(Style.getPredefinedStyle("Satellite Hybrid"))
                /*maplibreMap.setOnPolylineClickListener { polyline: Polyline ->
                    Toast.makeText(
                        this@PolylineActivity,
                        "You clicked on polyline with id = " + polyline.id,
                        Toast.LENGTH_SHORT
                    ).show()
                }*/
                maplibreMap.addAnnotations(*polylines.toTypedArray())
            }
        )
        val fab = findViewById<View>(R.id.fab)
        fab?.setOnClickListener { view: View? ->
            if (maplibreMap != null) {
                if (polylines != null && polylines.size > 0) {
                    if (polylines.size == 1) {
                        // test for removing annotation
                        maplibreMap.removeAnnotation(polylines[0])
                    } else {
                        // test for removing annotations
                        maplibreMap.removeAnnotations(*polylines.toTypedArray())
                    }
                }
                polylines.clear()
                randomLine.let {
                    polylines.add(it)
                    maplibreMap.addAnnotation(it)
                }
            }
        }
    }

    private val allPolylines: List<Line>
        private get() = listOf(
            generatePolyline(ANDORRA, LUXEMBOURG, "#F44336"),
            generatePolyline(ANDORRA, MONACO, "#FF5722"),
            generatePolyline(MONACO, VATICAN_CITY, "#673AB7"),
            generatePolyline(VATICAN_CITY, SAN_MARINO, "#009688"),
            generatePolyline(SAN_MARINO, LIECHTENSTEIN, "#795548"),
            generatePolyline(LIECHTENSTEIN, LUXEMBOURG, "#3F51B5")
        )

    private fun generatePolyline(start: LatLng, end: LatLng, color: String): Line =
        Line(listOf(start, end), color = Color.parseColor(color))

    val randomLine: Line
        get() = allPolylines.random()

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
        menuInflater.inflate(R.menu.menu_polyline, menu)
        return super.onCreateOptionsMenu(menu)
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        if (polylines.size <= 0) {
            Toast.makeText(this@PolylineActivity, "No polylines on map", Toast.LENGTH_LONG).show()
            return super.onOptionsItemSelected(item)
        }
        return when (item.itemId) {
            R.id.action_id_remove -> {
                // test to remove all annotations
                maplibreMap.clear()
                polylines.clear()
                true
            }
            R.id.action_id_alpha -> {
                fullAlpha = !fullAlpha
                for (p in polylines) {
                    p.opacity = if (fullAlpha) FULL_ALPHA else PARTIAL_ALPHA
                }
                true
            }
            R.id.action_id_color -> {
                color = !color
                for (p in polylines) {
                    p.color = if (color) Color.RED else Color.BLUE
                }
                true
            }
            R.id.action_id_width -> {
                width = !width
                for (p in polylines) {
                    p.width = if (width) 3.0f else 5.0f
                }
                true
            }
            R.id.action_id_visible -> {
                showPolylines = !showPolylines
                for (p in polylines) {
                    p.opacity =
                        if (showPolylines) (if (fullAlpha) FULL_ALPHA else PARTIAL_ALPHA) else NO_ALPHA
                }
                true
            }
            else -> super.onOptionsItemSelected(item)
        }
    }

    companion object {
        private val ANDORRA = LatLng(42.505777, 1.52529)
        private val LUXEMBOURG = LatLng(49.815273, 6.129583)
        private val MONACO = LatLng(43.738418, 7.424616)
        private val VATICAN_CITY = LatLng(41.902916, 12.453389)
        private val SAN_MARINO = LatLng(43.942360, 12.457777)
        private val LIECHTENSTEIN = LatLng(47.166000, 9.555373)
        private const val FULL_ALPHA = 1.0f
        private const val PARTIAL_ALPHA = 0.5f
        private const val NO_ALPHA = 0.0f
    }
}
