package org.maplibre.android.testapp.activity.textureview

import android.os.Bundle
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.*
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.utils.ResourceUtils
import timber.log.Timber
import java.io.IOException

/**
 * Example showcasing how to create a TextureView with a transparent background.
 */
class TextureViewTransparentBackgroundActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    private val maplibreMap: MapLibreMap? = null
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_textureview_transparent)
        setupBackground()
        setupMapView(savedInstanceState)
    }

    private fun setupBackground() {
        val imageView = findViewById<ImageView>(R.id.imageView)
        imageView.setImageResource(R.drawable.water)
        imageView.scaleType = ImageView.ScaleType.FIT_XY
    }

    private fun setupMapView(savedInstanceState: Bundle?) {
        val maplibreMapOptions = MapLibreMapOptions.createFromAttributes(this, null)
        maplibreMapOptions.translucentTextureSurface(true)
        maplibreMapOptions.textureMode(true)
        maplibreMapOptions.camera(
            CameraPosition.Builder()
                .zoom(2.0)
                .target(LatLng(48.507879, 8.363795))
                .build()
        )
        mapView = MapView(this, maplibreMapOptions)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync { maplibreMap: MapLibreMap -> initMap(maplibreMap) }
        (findViewById<View>(R.id.coordinator_layout) as ViewGroup).addView(mapView)
    }

    private fun initMap(maplibreMap: MapLibreMap) {
        try {
            maplibreMap.setStyle(
                Style.Builder().fromJson(ResourceUtils.readRawResource(this, R.raw.no_bg_style))
            )
        } catch (exception: IOException) {
            Timber.e(exception)
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
}
