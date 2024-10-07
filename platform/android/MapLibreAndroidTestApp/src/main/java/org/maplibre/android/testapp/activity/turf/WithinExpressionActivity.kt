package org.maplibre.android.testapp.activity.turf

import android.graphics.Color
import android.os.*
import androidx.appcompat.app.AppCompatActivity
import kotlinx.coroutines.*
import org.maplibre.geojson.*
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.Style
import org.maplibre.android.style.expressions.Expression.within
import org.maplibre.android.style.layers.CircleLayer
import org.maplibre.android.style.layers.FillExtrusionLayer
import org.maplibre.android.style.layers.LineLayer
import org.maplibre.android.style.layers.Property.NONE
import org.maplibre.android.style.layers.PropertyFactory.*
import org.maplibre.android.style.layers.SymbolLayer
import org.maplibre.android.style.sources.GeoJsonOptions
import org.maplibre.android.style.sources.GeoJsonSource
import org.maplibre.android.testapp.databinding.ActivityWithinExpressionBinding
import java.lang.Runnable
import org.maplibre.android.testapp.styles.TestStyles

/**
 * An Activity that showcases the within expression to filter features outside a geometry
 */
class WithinExpressionActivity : AppCompatActivity() {

    private lateinit var maplibreMap: MapLibreMap
    private lateinit var binding: ActivityWithinExpressionBinding
    private lateinit var mapView: MapView

    private val handler: Handler = Handler(Looper.getMainLooper())
    private val runnable: Runnable = Runnable {
        optimizeStyle()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityWithinExpressionBinding.inflate(layoutInflater)
        setContentView(binding.root)
        mapView = binding.mapView

        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync { map ->
            maplibreMap = map

            // Setup camera position above Georgetown
            maplibreMap.cameraPosition = CameraPosition.Builder()
                .target(LatLng(38.90628988399711, -77.06574689337494))
                .zoom(15.5)
                .build()

            // Wait for the map to become idle before manipulating the style and camera of the map
            mapView.addOnDidBecomeIdleListener(object : MapView.OnDidBecomeIdleListener {
                override fun onDidBecomeIdle() {
                    handler.postDelayed(runnable, 500)
                    mapView.removeOnDidBecomeIdleListener(this)
                }
            })

            // Load mapbox streets and add lines and circles
            setupStyle()
        }
    }
    
    private var bufferedLineSource: GeoJsonSource? = null
    private var timerJob: Job? = null
    private var symbolLayer: SymbolLayer? = null
    
    private fun setupStyle() {
        // Create buffer around linestring
        val bufferedRouteGeometry = bufferLineStringGeometry(outlineCoordinates)
        
        bufferedLineSource = GeoJsonSource(
            FILL_ID,
            FeatureCollection.fromFeature(Feature.fromGeometry(bufferedRouteGeometry)),
            GeoJsonOptions().withBuffer(0).withTolerance(0.0f)
        )

        // Setup style with additional layers,
        // using streets as a base style
        maplibreMap.setStyle(
            Style.Builder()
                .fromUri(TestStyles.getPredefinedStyleWithFallback("Streets"))
                .withSources(
                    GeoJsonSource(
                        POINT_ID,
                        LineString.fromLngLats(routeLineCoordinates)
                    ), bufferedLineSource
                )
                .withLayerBelow(
                    LineLayer(LINE_ID, POINT_ID)
                        .withProperties(lineWidth(7.5f), lineColor(Color.LTGRAY)),
                    "poi-label"
                )
                .withLayerBelow(
                    CircleLayer(POINT_ID, POINT_ID)
                        .withProperties(
                            circleRadius(7.5f),
                            circleColor(Color.DKGRAY),
                            circleOpacity(0.75f)
                        ),
                    "poi-label"
                )
        )

        // Un-comment to repeatedly shuffle the points of the bound as a test of fills and earcut
/*
        mapView.addOnDidBecomeIdleListener(object : MapView.OnDidBecomeIdleListener {
            override fun onDidBecomeIdle() {
                handler.postDelayed(Runnable {
                    var shuffleGeometry: (()->Unit)? = null
                    shuffleGeometry = {
                        mapView.addOnDidBecomeIdleListener(object : MapView.OnDidBecomeIdleListener {
                            override fun onDidBecomeIdle() {
                                handler.postDelayed(Runnable {
                                    // Duplicate each point and then shuffle to allow for degenerate cases
                                    val points = outlineCoordinates.flatMap { listOf(it,it,it) }.shuffled()
                                    val shuffledGeometry = bufferLineStringGeometry(points)
                                    val features = FeatureCollection.fromFeature(Feature.fromGeometry(shuffledGeometry))
                                    bufferedLineSource?.setGeoJson(features)
                                    symbolLayer?.setFilter(within(shuffledGeometry))
                                }, 100)
                                mapView.removeOnDidBecomeIdleListener(this)
                                shuffleGeometry?.let { it() }
                            }
                        })
                    }.also { it() }
                }, 1000)
                mapView.removeOnDidBecomeIdleListener(this)
            }
        })
*/
    }

    private fun optimizeStyle() {
        val style = maplibreMap.style!!

        // Add fill layer to represent buffered LineString
        maplibreMap.style!!.addLayerBelow(
            FillExtrusionLayer(FILL_ID, FILL_ID)
                .withProperties(
                    fillExtrusionOpacity(0.2f),
                    fillExtrusionColor(Color.YELLOW),
                    fillExtrusionHeight(100.0f),
                    fillExtrusionVerticalGradient(true),
                ),
            LINE_ID
        )

        // Move to a new camera position
        maplibreMap.easeCamera(
            CameraUpdateFactory.newCameraPosition(
                CameraPosition.Builder()
                    .zoom(16.0)
                    .target(LatLng(38.905156245642814, -77.06535338052844))
                    .bearing(80.68015859462369)
                    .tilt(55.0)
                    .build()
            ),
            1750
        )

        // Show only POI labels inside geometry using within expression
        symbolLayer = (style.getLayer("poi_z16") as SymbolLayer).apply {
            setFilter(
                within(
                    bufferLineStringGeometry(outlineCoordinates)
                )
            )
        }

        // Hide other types of labels to highlight POI labels
        (style.getLayer("road_label") as SymbolLayer?)?.setProperties(visibility(NONE))
        (style.getLayer("airport-label-major") as SymbolLayer?)?.setProperties(visibility(NONE))
        (style.getLayer("poi_transit") as SymbolLayer?)?.setProperties(visibility(NONE))
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

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    override fun onDestroy() {
        timerJob?.cancel()
        timerJob = null
        super.onDestroy()
        handler.removeCallbacks(runnable)
        mapView.onDestroy()
    }

    override fun onSaveInstanceState(outState: Bundle, outPersistentState: PersistableBundle) {
        super.onSaveInstanceState(outState, outPersistentState)
        outState.let {
            mapView.onSaveInstanceState(it)
        }
    }
    
    private val routeLineCoordinates = listOf<Point>(
        Point.fromLngLat(-77.06866264343262, 38.90506061276737),
        Point.fromLngLat(-77.06283688545227, 38.905194197410545),
        Point.fromLngLat(-77.06285834312439, 38.906429843444094),
        Point.fromLngLat(-77.0630407333374, 38.90680554236621)
    )
    
    private val outlineCoordinates = listOf<Point>(
        Point.fromLngLat(-77.06867337226866, 38.90467655551809),
        Point.fromLngLat(-77.06233263015747, 38.90479344272695),
        Point.fromLngLat(-77.06234335899353, 38.906463238984344),
        Point.fromLngLat(-77.06290125846863, 38.907206285691615),
        Point.fromLngLat(-77.06364154815674, 38.90684728656818),
        Point.fromLngLat(-77.06326603889465, 38.90637140121084),
        Point.fromLngLat(-77.06321239471436, 38.905561553883246),
        Point.fromLngLat(-77.0691454410553, 38.905436318935635),
        Point.fromLngLat(-77.06912398338318, 38.90466820642439),
        Point.fromLngLat(-77.06867337226866, 38.90467655551809),
    )
    
    private fun bufferLineStringGeometry(coordinates: List<Point>): Polygon {
        // TODO replace static data by Turf#Buffer: mapbox-java/issues/987
        return Polygon.fromLngLats(listOf(coordinates))
    }

    companion object {
        const val POINT_ID = "point"
        const val FILL_ID = "fill"
        const val LINE_ID = "line"
    }
}
