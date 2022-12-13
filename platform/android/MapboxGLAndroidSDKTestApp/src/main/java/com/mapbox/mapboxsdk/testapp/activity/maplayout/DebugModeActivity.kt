package com.mapbox.mapboxsdk.testapp.activity.maplayout

import android.content.Context
import android.os.Bundle
import android.view.*
import android.widget.*
import androidx.appcompat.app.ActionBarDrawerToggle
import androidx.appcompat.app.AppCompatActivity
import androidx.drawerlayout.widget.DrawerLayout
import com.google.android.material.floatingactionbutton.FloatingActionButton
import com.mapbox.mapboxsdk.maps.*
import com.mapbox.mapboxsdk.maps.MapboxMap.OnCameraMoveListener
import com.mapbox.mapboxsdk.maps.MapboxMap.OnFpsChangedListener
import com.mapbox.mapboxsdk.style.layers.Layer
import com.mapbox.mapboxsdk.style.layers.Property
import com.mapbox.mapboxsdk.style.layers.PropertyFactory
import com.mapbox.mapboxsdk.testapp.R
import timber.log.Timber
import java.util.*

/**
 * Test activity showcasing the different debug modes and allows to cycle between the default map styles.
 */
open class DebugModeActivity : AppCompatActivity(), OnMapReadyCallback, OnFpsChangedListener {
    private var mapView: MapView? = null
    private var mapboxMap: MapboxMap? = null
    private var cameraMoveListener: OnCameraMoveListener? = null
    private var actionBarDrawerToggle: ActionBarDrawerToggle? = null
    private var currentStyleIndex = 0
    private var isReportFps = true
    private var fpsView: TextView? = null
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_debug_mode)
        setupToolbar()
        setupMapView(savedInstanceState)
        setupDebugChangeView()
        setupStyleChangeView()
    }

    private fun setupToolbar() {
        val actionBar = supportActionBar
        if (actionBar != null) {
            supportActionBar!!.setDisplayHomeAsUpEnabled(true)
            supportActionBar!!.setHomeButtonEnabled(true)
            val drawerLayout = findViewById<DrawerLayout>(R.id.drawer_layout)
            actionBarDrawerToggle = ActionBarDrawerToggle(
                this,
                drawerLayout,
                R.string.navigation_drawer_open,
                R.string.navigation_drawer_close
            )
            actionBarDrawerToggle!!.isDrawerIndicatorEnabled = true
            actionBarDrawerToggle!!.syncState()
        }
    }

    private fun setupMapView(savedInstanceState: Bundle?) {
        val mapboxMapOptions = setupMapboxMapOptions()
        mapView = MapView(this, mapboxMapOptions)
        (findViewById<View>(R.id.coordinator_layout) as ViewGroup).addView(mapView, 0)
        mapView!!.addOnDidFinishLoadingStyleListener {
            if (mapboxMap != null) {
                setupNavigationView(mapboxMap!!.style!!.layers)
            }
        }
        mapView!!.tag = true
        mapView!!.onCreate(savedInstanceState)
        mapView!!.getMapAsync(this)
        mapView!!.addOnDidFinishLoadingStyleListener { Timber.d("Style loaded") }
    }

    protected open fun setupMapboxMapOptions(): MapboxMapOptions {
        return MapboxMapOptions.createFromAttributes(this, null)
    }

    override fun onMapReady(map: MapboxMap) {
        mapboxMap = map
        mapboxMap!!.setStyle(
            Style.Builder().fromUri(STYLES[currentStyleIndex])
        ) { style: Style -> setupNavigationView(style.layers) }
        setupZoomView()
        setFpsView()
    }

    private fun setFpsView() {
        fpsView = findViewById(R.id.fpsView)
        mapboxMap!!.setOnFpsChangedListener(this)
    }

    override fun onFpsChanged(fps: Double) {
        fpsView!!.text = String.format(Locale.US, "FPS: %4.2f", fps)
    }

    private fun setupNavigationView(layerList: List<Layer>) {
        Timber.v("New style loaded with JSON: %s", mapboxMap!!.style!!.json)
        val adapter = LayerListAdapter(this, layerList)
        val listView = findViewById<ListView>(R.id.listView)
        listView.adapter = adapter
        listView.onItemClickListener =
            AdapterView.OnItemClickListener { parent: AdapterView<*>?, view: View?, position: Int, id: Long ->
                val clickedLayer = adapter.getItem(position)
                toggleLayerVisibility(clickedLayer)
                closeNavigationView()
            }
    }

    private fun toggleLayerVisibility(layer: Layer) {
        val isVisible = layer.visibility.getValue() == Property.VISIBLE
        layer.setProperties(
            PropertyFactory.visibility(
                if (isVisible) Property.NONE else Property.VISIBLE
            )
        )
    }

    private fun closeNavigationView() {
        val drawerLayout = findViewById<DrawerLayout>(R.id.drawer_layout)
        drawerLayout.closeDrawers()
    }

    private fun setupZoomView() {
        val textView = findViewById<TextView>(R.id.textZoom)
        mapboxMap!!.addOnCameraMoveListener(
            OnCameraMoveListener {
                textView.text = String.format(
                    this@DebugModeActivity.getString(
                        R.string.debug_zoom
                    ),
                    mapboxMap!!.cameraPosition.zoom
                )
            }.also { cameraMoveListener = it }
        )
    }

    private fun setupDebugChangeView() {
        val fabDebug = findViewById<FloatingActionButton>(R.id.fabDebug)
        fabDebug.setOnClickListener { view: View? ->
            if (mapboxMap != null) {
                mapboxMap!!.isDebugActive = !mapboxMap!!.isDebugActive
                Timber.d("Debug FAB: isDebug Active? %s", mapboxMap!!.isDebugActive)
            }
        }
    }

    private fun setupStyleChangeView() {
        val fabStyles = findViewById<FloatingActionButton>(R.id.fabStyles)
        fabStyles.setOnClickListener { view: View? ->
            if (mapboxMap != null) {
                currentStyleIndex++
                if (currentStyleIndex == STYLES.size) {
                    currentStyleIndex = 0
                }
                mapboxMap!!.setStyle(Style.Builder().fromUri(STYLES[currentStyleIndex]))
            }
        }
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        val itemId = item.itemId
        if (itemId == R.id.menu_action_toggle_report_fps) {
            isReportFps = !isReportFps
            fpsView!!.visibility = if (isReportFps) View.VISIBLE else View.GONE
            mapboxMap!!.setOnFpsChangedListener(if (isReportFps) this else null)
        } else if (itemId == R.id.menu_action_limit_to_30_fps) {
            mapView!!.setMaximumFps(30)
        } else if (itemId == R.id.menu_action_limit_to_60_fps) {
            mapView!!.setMaximumFps(60)
        }
        return actionBarDrawerToggle!!.onOptionsItemSelected(item) || super.onOptionsItemSelected(
            item
        )
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.menu_debug, menu)
        return true
    }

    override fun onStart() {
        super.onStart()
        mapView!!.onStart()
    }

    override fun onResume() {
        super.onResume()
        mapView!!.onResume()
    }

    override fun onPause() {
        super.onPause()
        mapView!!.onPause()
    }

    override fun onStop() {
        super.onStop()
        mapView!!.onStop()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView!!.onSaveInstanceState(outState)
    }

    override fun onDestroy() {
        super.onDestroy()
        if (mapboxMap != null) {
            mapboxMap!!.removeOnCameraMoveListener(cameraMoveListener!!)
        }
        mapView!!.onDestroy()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView!!.onLowMemory()
    }

    private class LayerListAdapter(context: Context?, layers: List<Layer>) :
        BaseAdapter() {
        private val layoutInflater: LayoutInflater
        private val layers: List<Layer>
        override fun getCount(): Int {
            return layers.size
        }

        override fun getItem(position: Int): Layer {
            return layers[position]
        }

        override fun getItemId(position: Int): Long {
            return position.toLong()
        }

        override fun getView(position: Int, convertView: View?, parent: ViewGroup): View {
            val layer = layers[position]
            var view = convertView
            if (view == null) {
                view = layoutInflater.inflate(android.R.layout.simple_list_item_2, parent, false)
                val holder = ViewHolder(
                    view.findViewById(android.R.id.text1),
                    view.findViewById(android.R.id.text2)
                )
                view.tag = holder
            }
            val holder = view!!.tag as ViewHolder
            holder.text.text = layer.javaClass.simpleName
            holder.subText.text = layer.id
            return view
        }

        private class ViewHolder(val text: TextView, val subText: TextView)

        init {
            layoutInflater = LayoutInflater.from(context)
            this.layers = layers
        }
    }

    companion object {
        private val STYLES = arrayOf(
            Style.getPredefinedStyle("Streets"),
            Style.getPredefinedStyle("Outdoor"),
            Style.getPredefinedStyle("Bright"),
            Style.getPredefinedStyle("Pastel"),
            Style.getPredefinedStyle("Satellite Hybrid"),
            Style.getPredefinedStyle("Satellite Hybrid")
        )
    }
}
