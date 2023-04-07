package com.mapbox.mapboxsdk.testapp.activity.turf

import android.graphics.Color
import android.os.Bundle
import android.os.PersistableBundle
import android.widget.SeekBar
import androidx.appcompat.app.AppCompatActivity
import com.mapbox.geojson.Point
import com.mapbox.mapboxsdk.camera.CameraPosition
import com.mapbox.mapboxsdk.geometry.LatLng
import com.mapbox.mapboxsdk.maps.Style
import com.mapbox.mapboxsdk.style.expressions.Expression.*
import com.mapbox.mapboxsdk.style.layers.FillLayer
import com.mapbox.mapboxsdk.style.layers.PropertyFactory.fillColor
import com.mapbox.mapboxsdk.style.sources.GeoJsonSource
import com.mapbox.mapboxsdk.testapp.databinding.ActivityPhysicalCircleBinding
import com.mapbox.turf.TurfTransformation

/**
 * An Activity that showcases how to create a Circle with radius expressed in physical units using a FillLayer.
 */
class PhysicalUnitCircleActivity : AppCompatActivity(), SeekBar.OnSeekBarChangeListener {

    companion object {
        const val LAYER_ID = "circle-id"
        const val SOURCE_ID = "circle-id"
        const val LATITUDE = 22.928207
        const val LONGITUDE = 15.0155543
        const val ZOOM = 10.0
    }

    private lateinit var binding: ActivityPhysicalCircleBinding
    private lateinit var source: GeoJsonSource
    private var steps: Int = 10
    private var radius: Double = 9000.0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityPhysicalCircleBinding.inflate(layoutInflater)
        setContentView(binding.root)
        binding.mapView.onCreate(savedInstanceState)
        binding.mapView.getMapAsync { mapboxMap ->

            mapboxMap.cameraPosition = CameraPosition.Builder()
                .target(LatLng(LATITUDE, LONGITUDE))
                .zoom(ZOOM)
                .build()

            source = GeoJsonSource(
                SOURCE_ID,
                TurfTransformation.circle(
                    Point.fromLngLat(LONGITUDE, LATITUDE),
                    9000.0,
                    10,
                    "meters"
                )
            )

            binding.stepsBar.setOnSeekBarChangeListener(this)
            binding.radiusBar.setOnSeekBarChangeListener(this)

            mapboxMap.setStyle(
                Style.Builder()
                    .fromUri(Style.getPredefinedStyle("Satellite Hybrid"))
                    .withLayer(
                        FillLayer(LAYER_ID, SOURCE_ID).withProperties(
                            fillColor(
                                interpolate(
                                    exponential(0.5f),
                                    zoom(),
                                    stop(8, color(Color.RED)),
                                    stop(12, color(Color.BLUE)),
                                    stop(16, color(Color.GREEN))
                                )
                            )
                        )
                    )
                    .withSource(source)
            )
        }
    }

    override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
        seekBar?.let {
            if (it.id == binding.stepsBar.id) {
                steps = progress
            } else {
                radius = progress.toDouble()
            }

            source.setGeoJson(
                TurfTransformation.circle(
                    Point.fromLngLat(LONGITUDE, LATITUDE),
                    radius,
                    steps,
                    "meters"
                )
            )
        }
    }

    override fun onStartTrackingTouch(seekBar: SeekBar?) {
        // no-op
    }

    override fun onStopTrackingTouch(seekBar: SeekBar?) {
        // no-op
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
}
