package com.mapbox.mapboxsdk.testapp.activity.style

import android.graphics.Color
import android.os.Bundle
import android.os.Handler
import android.view.Menu
import android.view.MenuItem
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.mapbox.geojson.Feature
import com.mapbox.geojson.FeatureCollection
import com.mapbox.geojson.Point
import com.mapbox.geojson.Polygon
import com.mapbox.mapboxsdk.camera.CameraUpdateFactory
import com.mapbox.mapboxsdk.geometry.LatLng
import com.mapbox.mapboxsdk.maps.MapView
import com.mapbox.mapboxsdk.maps.MapboxMap
import com.mapbox.mapboxsdk.maps.MapboxMap.CancelableCallback
import com.mapbox.mapboxsdk.maps.OnMapReadyCallback
import com.mapbox.mapboxsdk.maps.Style
import com.mapbox.mapboxsdk.style.expressions.Expression
import com.mapbox.mapboxsdk.style.layers.CircleLayer
import com.mapbox.mapboxsdk.style.layers.FillLayer
import com.mapbox.mapboxsdk.style.layers.Layer
import com.mapbox.mapboxsdk.style.layers.LineLayer
import com.mapbox.mapboxsdk.style.layers.Property
import com.mapbox.mapboxsdk.style.layers.PropertyFactory
import com.mapbox.mapboxsdk.style.layers.RasterLayer
import com.mapbox.mapboxsdk.style.layers.SymbolLayer
import com.mapbox.mapboxsdk.style.layers.TransitionOptions
import com.mapbox.mapboxsdk.style.sources.GeoJsonSource
import com.mapbox.mapboxsdk.style.sources.RasterSource
import com.mapbox.mapboxsdk.style.sources.Source
import com.mapbox.mapboxsdk.style.sources.TileSet
import com.mapbox.mapboxsdk.style.sources.VectorSource
import com.mapbox.mapboxsdk.testapp.R
import com.mapbox.mapboxsdk.testapp.utils.ResourceUtils
import timber.log.Timber
import java.io.IOException
import java.util.Arrays
import java.util.Collections

/**
 * Test activity showcasing the runtime style API.
 */
class RuntimeStyleActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    private var mapboxMap: MapboxMap? = null
    private var styleLoaded = false
    var lngLats = listOf(
        Arrays.asList(
            Point.fromLngLat(
                -15.468749999999998,
                41.77131167976407
            ),
            Point.fromLngLat(
                15.468749999999998,
                41.77131167976407
            ),
            Point.fromLngLat(
                15.468749999999998,
                58.26328705248601
            ),
            Point.fromLngLat(
                -15.468749999999998,
                58.26328705248601
            ),
            Point.fromLngLat(
                -15.468749999999998,
                41.77131167976407
            )
        )
    )
    var polygon = Polygon.fromLngLats(lngLats)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_runtime_style)

        // Initialize map as normal
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(
            OnMapReadyCallback { map: MapboxMap? ->
                // Store for later
                mapboxMap = map

                // Center and Zoom (Amsterdam, zoomed to streets)
                mapboxMap!!.animateCamera(
                    CameraUpdateFactory.newLatLngZoom(
                        LatLng(52.379189, 4.899431),
                        1.0
                    )
                )
                mapboxMap!!.setStyle(
                    Style.Builder()
                        .fromUri(Style.getPredefinedStyle("Streets")) // set custom transition
                        .withTransition(TransitionOptions(250, 50))
                ) { style: Style ->
                    styleLoaded = true
                    val laber = style.getLayer("country_1") as SymbolLayer?
                    laber!!.setProperties(
                        PropertyFactory.textOpacity(
                            Expression.switchCase(
                                Expression.within(polygon),
                                Expression.literal(1.0f),
                                Expression.literal(0.5f)
                            )
                        )
                    )
                }
            }
        )
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.menu_runtime_style, menu)
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
        mapView!!.onDestroy()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView!!.onLowMemory()
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return if (!styleLoaded) {
            false
        } else {
            when (item.itemId) {
                R.id.action_list_layers -> {
                    listLayers()
                    true
                }

                R.id.action_list_sources -> {
                    listSources()
                    true
                }

                R.id.action_water_color -> {
                    setWaterColor()
                    true
                }

                R.id.action_background_opacity -> {
                    setBackgroundOpacity()
                    true
                }

                R.id.action_road_avoid_edges -> {
                    setRoadSymbolPlacement()
                    true
                }

                R.id.action_layer_visibility -> {
                    setLayerInvisible()
                    true
                }

                R.id.action_remove_layer -> {
                    removeBuildings()
                    true
                }

                R.id.action_add_parks_layer -> {
                    addParksLayer()
                    true
                }

                R.id.action_add_dynamic_parks_layer -> {
                    addDynamicParksLayer()
                    true
                }

                R.id.action_add_terrain_layer -> {
                    addTerrainLayer()
                    true
                }

                R.id.action_add_satellite_layer -> {
                    addSatelliteLayer()
                    true
                }

                R.id.action_update_water_color_on_zoom -> {
                    updateWaterColorOnZoom()
                    true
                }

                R.id.action_add_custom_tiles -> {
                    addCustomTileSource()
                    true
                }

                R.id.action_fill_filter -> {
                    styleFillFilterLayer()
                    true
                }

                R.id.action_textsize_filter -> {
                    styleTextSizeFilterLayer()
                    true
                }

                R.id.action_line_filter -> {
                    styleLineFilterLayer()
                    true
                }

                R.id.action_numeric_filter -> {
                    styleNumericFillLayer()
                    true
                }

                R.id.action_bring_water_to_front -> {
                    bringWaterToFront()
                    true
                }

                R.id.action_fill_filter_color -> {
                    styleFillColorLayer()
                    true
                }

                else -> super.onOptionsItemSelected(item)
            }
        }
    }

    private fun listLayers() {
        val layers = mapboxMap!!.style!!
            .layers
        val builder = StringBuilder("Layers:")
        for (layer in layers) {
            builder.append("\n")
            builder.append(layer.id)
        }
        Toast.makeText(this, builder.toString(), Toast.LENGTH_LONG).show()
    }

    private fun listSources() {
        val sources = mapboxMap!!.style!!
            .sources
        val builder = StringBuilder("Sources:")
        for (source in sources) {
            builder.append("\n")
            builder.append(source.id)
        }
        Toast.makeText(this, builder.toString(), Toast.LENGTH_LONG).show()
    }

    private fun setLayerInvisible() {
        val roadLayers = arrayOf("water")
        for (roadLayer in roadLayers) {
            val layer = mapboxMap!!.style!!.getLayer(roadLayer)
            layer?.setProperties(PropertyFactory.visibility(Property.NONE))
        }
    }

    private fun setRoadSymbolPlacement() {
        // Zoom so that the labels are visible first
        mapboxMap!!.animateCamera(
            CameraUpdateFactory.zoomTo(14.0),
            object : DefaultCallback() {
                override fun onFinish() {
                    val roadLayers =
                        arrayOf("road-label-small", "road-label-medium", "road-label-large")
                    for (roadLayer in roadLayers) {
                        val layer = mapboxMap!!.style!!
                            .getLayer(roadLayer)
                        layer?.setProperties(PropertyFactory.symbolPlacement(Property.SYMBOL_PLACEMENT_POINT))
                    }
                }
            }
        )
    }

    private fun setBackgroundOpacity() {
        val background = mapboxMap!!.style!!.getLayer("background")
        background?.setProperties(PropertyFactory.backgroundOpacity(0.2f))
    }

    private fun setWaterColor() {
        val water = mapboxMap!!.style!!.getLayerAs<FillLayer>("water")
        if (water != null) {
            water.fillColorTransition = TransitionOptions(7500, 1000)
            water.setProperties(
                PropertyFactory.visibility(Property.VISIBLE),
                PropertyFactory.fillColor(Color.RED)
            )
        } else {
            Toast.makeText(
                this@RuntimeStyleActivity,
                "No water layer in this style",
                Toast.LENGTH_SHORT
            ).show()
        }
    }

    private fun removeBuildings() {
        // Zoom to see buildings first
        mapboxMap!!.style!!.removeLayer("building")
    }

    private fun addParksLayer() {
        // Add a source
        val source: Source
        source = try {
            GeoJsonSource("amsterdam-spots", ResourceUtils.readRawResource(this, R.raw.amsterdam))
        } catch (ioException: IOException) {
            Toast.makeText(
                this@RuntimeStyleActivity,
                "Couldn't add source: " + ioException.message,
                Toast.LENGTH_SHORT
            ).show()
            return
        }
        mapboxMap!!.style!!.addSource(source)
        var layer: FillLayer? = FillLayer("parksLayer", "amsterdam-spots")
        layer!!.setProperties(
            PropertyFactory.fillColor(Color.RED),
            PropertyFactory.fillOutlineColor(Color.BLUE),
            PropertyFactory.fillOpacity(0.3f),
            PropertyFactory.fillAntialias(true)
        )

        // Only show me parks (except westerpark with stroke-width == 3)
        layer.setFilter(
            Expression.all(
                Expression.eq(Expression.get("type"), Expression.literal("park")),
                Expression.eq(
                    Expression.get("stroke-width"),
                    Expression.literal(3)
                )
            )
        )
        mapboxMap!!.style!!.addLayerBelow(layer, "building")
        // layer.setPaintProperty(fillColor(Color.RED)); // XXX But not after the object is attached

        // Or get the object later and set it. It's all good.
        mapboxMap!!.style!!.getLayer("parksLayer")!!
            .setProperties(PropertyFactory.fillColor(Color.RED))

        // You can get a typed layer, if you're sure it's of that type. Use with care
        layer = mapboxMap!!.style!!.getLayerAs("parksLayer")
        // And get some properties
        val fillAntialias = layer!!.fillAntialias
        Timber.d("Fill anti alias: %s", fillAntialias.getValue())
        layer.setProperties(PropertyFactory.fillTranslateAnchor(Property.FILL_TRANSLATE_ANCHOR_MAP))
        val fillTranslateAnchor = layer.fillTranslateAnchor
        Timber.d("Fill translate anchor: %s", fillTranslateAnchor.getValue())
        val visibility = layer.visibility
        Timber.d("Visibility: %s", visibility.getValue())

        // Get a good look at it all
        mapboxMap!!.animateCamera(CameraUpdateFactory.zoomTo(12.0))
    }

    private fun addDynamicParksLayer() {
        // Load some data
        val parks: FeatureCollection
        parks = try {
            val json = ResourceUtils.readRawResource(this, R.raw.amsterdam)
            FeatureCollection.fromJson(json)
        } catch (ioException: IOException) {
            Toast.makeText(
                this@RuntimeStyleActivity,
                "Couldn't add source: " + ioException.message,
                Toast.LENGTH_SHORT
            ).show()
            return
        }

        // Add an empty source
        mapboxMap!!.style!!.addSource(GeoJsonSource("dynamic-park-source"))
        val layer = FillLayer("dynamic-parks-layer", "dynamic-park-source")
        layer.setProperties(
            PropertyFactory.fillColor(Color.GREEN),
            PropertyFactory.fillOutlineColor(Color.GREEN),
            PropertyFactory.fillOpacity(0.8f),
            PropertyFactory.fillAntialias(true)
        )

        // Only show me parks
        layer.setFilter(
            Expression.all(
                Expression.eq(
                    Expression.get("type"),
                    Expression.literal("park")
                )
            )
        )
        mapboxMap!!.style!!.addLayer(layer)

        // Get a good look at it all
        mapboxMap!!.animateCamera(CameraUpdateFactory.zoomTo(12.0))

        // Animate the parks source
        animateParksSource(parks, 0)
    }

    private fun animateParksSource(parks: FeatureCollection, counter: Int) {
        val handler = Handler(mainLooper)
        handler.postDelayed(
            {
                if (mapboxMap == null) {
                    return@postDelayed
                }
                Timber.d("Updating parks source")
                // change the source
                val park = if (counter < parks.features()!!.size - 1) counter else 0
                val source = mapboxMap!!.style!!.getSourceAs<GeoJsonSource>("dynamic-park-source")
                if (source == null) {
                    Timber.e("Source not found")
                    Toast.makeText(this@RuntimeStyleActivity, "Source not found", Toast.LENGTH_SHORT)
                        .show()
                    return@postDelayed
                }
                val features: MutableList<Feature> = ArrayList()
                features.add(parks.features()!![park])
                source.setGeoJson(FeatureCollection.fromFeatures(features))

                // Re-post
                animateParksSource(parks, park + 1)
            },
            if (counter == 0) 100 else 1000.toLong()
        )
    }

    private fun addTerrainLayer() {
        // Add a source
        val source: Source = VectorSource("my-terrain-source", "maptiler://sources/hillshades")
        mapboxMap!!.style!!.addSource(source)
        var layer: LineLayer? = LineLayer("terrainLayer", "my-terrain-source")
        layer!!.sourceLayer = "contour"
        layer.setProperties(
            PropertyFactory.lineJoin(Property.LINE_JOIN_ROUND),
            PropertyFactory.lineCap(Property.LINE_CAP_ROUND),
            PropertyFactory.lineColor(Color.RED),
            PropertyFactory.lineWidth(20f)
        )

        // adding layers below "road" layers
        val layers = mapboxMap!!.style!!
            .layers
        var latestLayer: Layer? = null
        Collections.reverse(layers)
        for (currentLayer in layers) {
            if (currentLayer is FillLayer && currentLayer.sourceLayer == "road") {
                latestLayer = currentLayer
            } else if (currentLayer is CircleLayer && currentLayer.sourceLayer == "road") {
                latestLayer = currentLayer
            } else if (currentLayer is SymbolLayer && currentLayer.sourceLayer == "road") {
                latestLayer = currentLayer
            } else if (currentLayer is LineLayer && currentLayer.sourceLayer == "road") {
                latestLayer = currentLayer
            }
        }
        if (latestLayer != null) {
            mapboxMap!!.style!!.addLayerBelow(layer, latestLayer.id)
        }

        // Need to get a fresh handle
        layer = mapboxMap!!.style!!.getLayerAs("terrainLayer")

        // Make sure it's also applied after the fact
        layer!!.minZoom = 10f
        layer.maxZoom = 15f
        layer = mapboxMap!!.style!!.getLayer("terrainLayer") as LineLayer?
        Toast.makeText(
            this,
            String.format(
                "Set min/max zoom to %s - %s",
                layer!!.minZoom,
                layer.maxZoom
            ),
            Toast.LENGTH_SHORT
        ).show()
    }

    private fun addSatelliteLayer() {
        // Add a source
        val source: Source = RasterSource("my-raster-source", "maptiler://sources/satellite", 512)
        mapboxMap!!.style!!.addSource(source)

        // Add a layer
        mapboxMap!!.style!!.addLayer(RasterLayer("satellite-layer", "my-raster-source"))
    }

    private fun updateWaterColorOnZoom() {
        val layer = mapboxMap!!.style!!.getLayerAs<FillLayer>("water") ?: return

        // Set a zoom function to update the color of the water
        layer.setProperties(
            PropertyFactory.fillColor(
                Expression.interpolate(
                    Expression.exponential(0.8f),
                    Expression.zoom(),
                    Expression.stop(1, Expression.color(Color.GREEN)),
                    Expression.stop(4, Expression.color(Color.BLUE)),
                    Expression.stop(12, Expression.color(Color.RED)),
                    Expression.stop(20, Expression.color(Color.BLACK))
                )
            )
        )

        // do some animations to show it off properly
        mapboxMap!!.animateCamera(CameraUpdateFactory.zoomTo(1.0), 1500)
    }

    private fun addCustomTileSource() {
        // TODO: migrate
        // Add a source
        val tileSet = TileSet("2.1.0", "https://d25uarhxywzl1j.cloudfront.net/v0.1/{z}/{x}/{y}.mvt")
        tileSet.minZoom = 0f
        tileSet.maxZoom = 14f
        val source: Source = VectorSource("custom-tile-source", tileSet)
        mapboxMap!!.style!!.addSource(source)

        // Add a layer
        val lineLayer = LineLayer("custom-tile-layers", "custom-tile-source")
        lineLayer.sourceLayer = "mapillary-sequences"
        lineLayer.setProperties(
            PropertyFactory.lineCap(Property.LINE_CAP_ROUND),
            PropertyFactory.lineJoin(Property.LINE_JOIN_ROUND),
            PropertyFactory.lineOpacity(0.6f),
            PropertyFactory.lineWidth(2.0f),
            PropertyFactory.lineColor(Color.GREEN)
        )
        mapboxMap!!.style!!.addLayer(lineLayer)
    }

    private fun styleFillColorLayer() {
        mapboxMap!!.setStyle(Style.Builder().fromUri("asset://fill_color_style.json"))
        mapboxMap!!.moveCamera(
            CameraUpdateFactory.newLatLngZoom(
                LatLng(31.0, (-100).toDouble()),
                3.0
            )
        )
    }

    private fun styleFillFilterLayer() {
        mapboxMap!!.setStyle(Style.Builder().fromUri("asset://fill_filter_style.json"))
        mapboxMap!!.moveCamera(
            CameraUpdateFactory.newLatLngZoom(
                LatLng(31.0, (-100).toDouble()),
                3.0
            )
        )
        val handler = Handler(mainLooper)
        handler.postDelayed(
            {
                if (mapboxMap == null) {
                    return@postDelayed
                }
                Timber.d("Styling filtered fill layer")
                val states = mapboxMap!!.style!!.getLayer("states") as FillLayer?
                if (states != null) {
                    states.setFilter(Expression.eq(Expression.get("name"), Expression.literal("Texas")))
                    states.fillOpacityTransition = TransitionOptions(2500, 0)
                    states.fillColorTransition = TransitionOptions(2500, 0)
                    states.setProperties(
                        PropertyFactory.fillColor(Color.RED),
                        PropertyFactory.fillOpacity(0.25f)
                    )
                } else {
                    Toast.makeText(
                        this@RuntimeStyleActivity,
                        "No states layer in this style",
                        Toast.LENGTH_SHORT
                    ).show()
                }
            },
            2000
        )
    }

    private fun styleTextSizeFilterLayer() {
        mapboxMap!!.setStyle(Style.Builder().fromUri("asset://fill_filter_style.json"))
        mapboxMap!!.moveCamera(
            CameraUpdateFactory.newLatLngZoom(
                LatLng(31.0, (-100).toDouble()),
                3.0
            )
        )
        val handler = Handler(mainLooper)
        handler.postDelayed(
            {
                if (mapboxMap == null) {
                    return@postDelayed
                }
                Timber.d("Styling text size fill layer")
                val states = mapboxMap!!.style!!.getLayer("state-label-lg") as SymbolLayer?
                if (states != null) {
                    states.setProperties(
                        PropertyFactory.textSize(
                            Expression.switchCase(
                                Expression.`in`(Expression.get("name"), Expression.literal("Texas")),
                                Expression.literal(25.0f),
                                Expression.`in`(
                                    Expression.get("name"),
                                    Expression.literal(arrayOf<Any>("California", "Illinois"))
                                ),
                                Expression.literal(25.0f),
                                Expression.literal(6.0f) // default value
                            )
                        )
                    )
                } else {
                    Toast.makeText(
                        this@RuntimeStyleActivity,
                        "No states layer in this style",
                        Toast.LENGTH_SHORT
                    ).show()
                }
            },
            2000
        )
    }

    private fun styleLineFilterLayer() {
        mapboxMap!!.setStyle(Style.Builder().fromUri("asset://line_filter_style.json"))
        mapboxMap!!.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(40.0, (-97).toDouble()), 5.0))
        val handler = Handler(mainLooper)
        handler.postDelayed(
            {
                if (mapboxMap == null) {
                    return@postDelayed
                }
                Timber.d("Styling filtered line layer")
                val counties = mapboxMap!!.style!!.getLayer("counties") as LineLayer?
                if (counties != null) {
                    counties.setFilter(Expression.eq(Expression.get("NAME10"), "Washington"))
                    counties.setProperties(
                        PropertyFactory.lineColor(Color.RED),
                        PropertyFactory.lineOpacity(0.75f),
                        PropertyFactory.lineWidth(5f)
                    )
                } else {
                    Toast.makeText(
                        this@RuntimeStyleActivity,
                        "No counties layer in this style",
                        Toast.LENGTH_SHORT
                    ).show()
                }
            },
            2000
        )
    }

    private fun styleNumericFillLayer() {
        mapboxMap!!.setStyle(Style.Builder().fromUri("asset://numeric_filter_style.json"))
        mapboxMap!!.moveCamera(CameraUpdateFactory.newLatLngZoom(LatLng(40.0, (-97).toDouble()), 5.0))
        val handler = Handler(mainLooper)
        handler.postDelayed(
            {
                if (mapboxMap == null) {
                    return@postDelayed
                }
                Timber.d("Styling numeric fill layer")
                val regions = mapboxMap!!.style!!.getLayer("regions") as FillLayer?
                if (regions != null) {
                    regions.setFilter(
                        Expression.all(
                            Expression.gte(
                                Expression.toNumber(Expression.get("HRRNUM")),
                                Expression.literal(200)
                            ),
                            Expression.lt(
                                Expression.toNumber(Expression.get("HRRNUM")),
                                Expression.literal(300)
                            )
                        )
                    )
                    regions.setProperties(
                        PropertyFactory.fillColor(Color.BLUE),
                        PropertyFactory.fillOpacity(0.5f)
                    )
                } else {
                    Toast.makeText(
                        this@RuntimeStyleActivity,
                        "No regions layer in this style",
                        Toast.LENGTH_SHORT
                    ).show()
                }
            },
            2000
        )
    }

    private fun bringWaterToFront() {
        val water = mapboxMap!!.style!!.getLayer("water")
        if (water != null) {
            mapboxMap!!.style!!.removeLayer(water)
            mapboxMap!!.style!!.addLayerAt(water, mapboxMap!!.style!!.layers.size - 1)
        } else {
            Toast.makeText(this, "No water layer in this style", Toast.LENGTH_SHORT).show()
        }
    }

    private open class DefaultCallback : CancelableCallback {
        override fun onCancel() {
            // noop
        }

        override fun onFinish() {
            // noop
        }
    }
}
