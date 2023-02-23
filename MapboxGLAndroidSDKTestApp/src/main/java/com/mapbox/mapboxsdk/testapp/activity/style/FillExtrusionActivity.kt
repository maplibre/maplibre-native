package com.mapbox.mapboxsdk.testapp.activity.style

import android.graphics.Color
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.mapbox.geojson.Point
import com.mapbox.geojson.Polygon
import com.mapbox.mapboxsdk.camera.CameraPosition
import com.mapbox.mapboxsdk.camera.CameraUpdateFactory
import com.mapbox.mapboxsdk.geometry.LatLng
import com.mapbox.mapboxsdk.maps.MapView
import com.mapbox.mapboxsdk.maps.MapboxMap
import com.mapbox.mapboxsdk.maps.OnMapReadyCallback
import com.mapbox.mapboxsdk.maps.Style
import com.mapbox.mapboxsdk.style.layers.*
import com.mapbox.mapboxsdk.style.sources.GeoJsonSource
import com.mapbox.mapboxsdk.testapp.R
import java.util.*

/**
 * Test activity showcasing fill extrusions
 */
class FillExtrusionActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_fill_extrusion_layer)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(
            OnMapReadyCallback { mapboxMap: MapboxMap ->
                mapboxMap.setStyle(Style.getPredefinedStyle("Streets")) { style: Style ->
                    val lngLats = listOf(
                        Arrays.asList(
                            Point.fromLngLat(5.12112557888031, 52.09071040847704),
                            Point.fromLngLat(5.121227502822875, 52.09053901776669),
                            Point.fromLngLat(5.121484994888306, 52.090601641371805),
                            Point.fromLngLat(5.1213884353637695, 52.090766439912635),
                            Point.fromLngLat(5.12112557888031, 52.09071040847704)
                        )
                    )
                    val domTower = Polygon.fromLngLats(lngLats)
                    val source = GeoJsonSource("extrusion-source", domTower)
                    style.addSource(source)
                    style.addLayer(
                        FillExtrusionLayer("extrusion-layer", source.id)
                            .withProperties(
                                PropertyFactory.fillExtrusionHeight(40f),
                                PropertyFactory.fillExtrusionOpacity(0.5f),
                                PropertyFactory.fillExtrusionColor(Color.RED)
                            )
                    )
                    mapboxMap.animateCamera(
                        CameraUpdateFactory.newCameraPosition(
                            CameraPosition.Builder()
                                .target(LatLng(52.09071040847704, 5.12112557888031))
                                .tilt(45.0)
                                .zoom(18.0)
                                .build()
                        ),
                        10000
                    )
                }
            }
        )
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

    public override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView!!.onSaveInstanceState(outState)
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView!!.onLowMemory()
    }

    public override fun onDestroy() {
        super.onDestroy()
        mapView!!.onDestroy()
    }
}
